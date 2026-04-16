#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* ─────────────────────────────────────────────
   Configuration
   ───────────────────────────────────────────── */
#define HASH_SIZE        (1 << 16)   /* 65536 buckets — power of 2 for fast modulo */
#define HASH_MASK        (HASH_SIZE - 1)
#define MAX_WORD_LEN     256
#define POSITIONS_INIT   8           /* initial capacity of position array per word */
#define READ_BUFFER      (1 << 20)   /* 1 MB read buffer */

/* ─────────────────────────────────────────────
   Data Structures
   ───────────────────────────────────────────── */

/* One occurrence of a word: line number + column offset */
typedef struct {
    int line;
    int col;
} Position;

/* Hash table entry — a word and all its positions */
typedef struct Entry {
    char        *word;          /* heap-allocated, lowercased */
    Position    *positions;     /* dynamic array */
    int          count;         /* entries used */
    int          capacity;      /* entries allocated */
    struct Entry *next;         /* chaining for collision resolution */
} Entry;

/* The index itself */
typedef struct {
    Entry  **buckets;
    int      total_words;       /* distinct words */
    int      total_occurrences;
} Index;

/* ─────────────────────────────────────────────
   FNV-1a hash — fast, low-collision, branchless
   ───────────────────────────────────────────── */
static inline unsigned int fnv1a(const char *s) {
    unsigned int h = 2166136261u;
    while (*s) {
        h ^= (unsigned char)*s++;
        h *= 16777619u;
    }
    return h & HASH_MASK;
}

/* ─────────────────────────────────────────────
   Index lifecycle
   ───────────────────────────────────────────── */
static Index *index_create(void) {
    Index *idx = malloc(sizeof(Index));
    if (!idx) { perror("malloc"); exit(1); }
    idx->buckets = calloc(HASH_SIZE, sizeof(Entry *));
    if (!idx->buckets) { perror("calloc"); exit(1); }
    idx->total_words       = 0;
    idx->total_occurrences = 0;
    return idx;
}

static void index_free(Index *idx) {
    for (int i = 0; i < HASH_SIZE; i++) {
        Entry *e = idx->buckets[i];
        while (e) {
            Entry *next = e->next;
            free(e->word);
            free(e->positions);
            free(e);
            e = next;
        }
    }
    free(idx->buckets);
    free(idx);
}

/* Insert or update one occurrence */
static void index_insert(Index *idx, const char *word, int line, int col) {
    unsigned int h = fnv1a(word);
    Entry *e = idx->buckets[h];

    /* Walk chain — look for existing entry */
    while (e) {
        if (strcmp(e->word, word) == 0) goto found;
        e = e->next;
    }

    /* New entry */
    e = malloc(sizeof(Entry));
    if (!e) { perror("malloc"); exit(1); }
    e->word     = strdup(word);
    e->count    = 0;
    e->capacity = POSITIONS_INIT;
    e->positions = malloc(e->capacity * sizeof(Position));
    if (!e->positions) { perror("malloc"); exit(1); }
    e->next            = idx->buckets[h];
    idx->buckets[h]    = e;
    idx->total_words++;

found:
    /* Grow position array if needed — doubling strategy: O(1) amortised */
    if (e->count == e->capacity) {
        e->capacity *= 2;
        e->positions = realloc(e->positions, e->capacity * sizeof(Position));
        if (!e->positions) { perror("realloc"); exit(1); }
    }
    e->positions[e->count].line = line;
    e->positions[e->count].col  = col;
    e->count++;
    idx->total_occurrences++;
}

/* ─────────────────────────────────────────────
   Build index from file
   ───────────────────────────────────────────── */
static Index *build_index(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) { perror("fopen"); return NULL; }

    /* Large read buffer — fewer syscalls */
    char *buf = malloc(READ_BUFFER);
    if (!buf) { perror("malloc"); exit(1); }
    setvbuf(fp, buf, _IOFBF, READ_BUFFER);

    Index *idx  = index_create();
    char   word[MAX_WORD_LEN];
    int    wlen = 0;
    int    line = 1, col = 0;
    int    word_start_col = 0;
    int    c;

    while ((c = getc_unlocked(fp)) != EOF) {   /* getc_unlocked: no per-call mutex */
        col++;
        if (isalpha(c) || c == '\'') {          /* apostrophes kept — "don't" stays whole */
            if (wlen == 0) word_start_col = col;
            if (wlen < MAX_WORD_LEN - 1)
                word[wlen++] = (char)tolower(c);
        } else {
            if (wlen > 0) {
                word[wlen] = '\0';
                index_insert(idx, word, line, word_start_col);
                wlen = 0;
            }
            if (c == '\n') { line++; col = 0; }
        }
    }
    /* flush last word if file doesn't end with whitespace */
    if (wlen > 0) {
        word[wlen] = '\0';
        index_insert(idx, word, line, word_start_col);
    }

    fclose(fp);
    free(buf);
    return idx;
}

/* ─────────────────────────────────────────────
   Search
   ───────────────────────────────────────────── */
static const Entry *index_search(const Index *idx, const char *raw_word) {
    /* Lowercase the query before hashing */
    char word[MAX_WORD_LEN];
    int i = 0;
    while (raw_word[i] && i < MAX_WORD_LEN - 1) {
        word[i] = (char)tolower((unsigned char)raw_word[i]);
        i++;
    }
    word[i] = '\0';

    unsigned int h = fnv1a(word);
    const Entry *e = idx->buckets[h];
    while (e) {
        if (strcmp(e->word, word) == 0) return e;
        e = e->next;
    }
    return NULL;
}

/* ─────────────────────────────────────────────
   Display helpers
   ───────────────────────────────────────────── */
static void print_separator(int n) {
    for (int i = 0; i < n; i++) fputs("-", stdout);
    putchar('\n');
}

static void print_results(const Entry *e, const char *query) {
    if (!e) {
        printf("\n  [NOT FOUND]  \"%s\" — no occurrences in document.\n\n", query);
        return;
    }

    print_separator(56);
    printf("  Word     : \"%s\"\n", e->word);
    printf("  Found    : %d occurrence%s\n", e->count, e->count == 1 ? "" : "s");
    print_separator(56);

    /* Group by line for compact output */
    int shown = 0;
    int prev_line = -1;
    for (int i = 0; i < e->count; i++) {
        if (e->positions[i].line != prev_line) {
            if (shown > 0) putchar('\n');
            printf("  Line %4d  |  col ", e->positions[i].line);
            prev_line = e->positions[i].line;
            shown = 0;
        } else {
            printf(", ");
        }
        printf("%d", e->positions[i].col);
        shown++;
    }
    printf("\n");
    print_separator(56);
}

/* ─────────────────────────────────────────────
   Main
   ───────────────────────────────────────────── */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <document.txt> [word1 word2 ...]\n", argv[0]);
        return 1;
    }

    /* ── Build index ── */
    printf("\nIndexing \"%s\" ...\n", argv[1]);
    clock_t t0 = clock();
    Index *idx = build_index(argv[1]);
    clock_t t1 = clock();

    if (!idx) return 1;

    double elapsed = (double)(t1 - t0) / CLOCKS_PER_SEC * 1000.0;
    printf("Done. %d distinct words, %d total occurrences. [%.2f ms]\n\n",
           idx->total_words, idx->total_occurrences, elapsed);

    /* ── Query mode ── */
    if (argc > 2) {
        /* Words passed as CLI arguments */
        for (int i = 2; i < argc; i++) {
            clock_t s = clock();
            const Entry *e = index_search(idx, argv[i]);
            clock_t end = clock();
            double us = (double)(end - s) / CLOCKS_PER_SEC * 1e6;
            printf("Search: \"%s\"  [%.2f µs]\n", argv[i], us);
            print_results(e, argv[i]);
            putchar('\n');
        }
    } else {
        /* Interactive REPL */
        char query[MAX_WORD_LEN];
        printf("Enter a word to search (or \"exit\" to quit):\n");
        while (1) {
            printf("> ");
            fflush(stdout);
            if (!fgets(query, sizeof(query), stdin)) break;

            /* strip newline */
            query[strcspn(query, "\n")] = '\0';
            if (query[0] == '\0') continue;
            if (strcmp(query, "exit") == 0) break;

            clock_t s   = clock();
            const Entry *e = index_search(idx, query);
            clock_t end = clock();
            double us = (double)(end - s) / CLOCKS_PER_SEC * 1e6;

            printf("Search time: %.2f µs\n", us);
            print_results(e, query);
            putchar('\n');
        }
    }

    index_free(idx);
    return 0;
}
