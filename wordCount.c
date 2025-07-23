#define _POSIX_C_SOURCE 200809L /* strdup 선언 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uthash.h" /* 같은 디렉터리에 uthash.h 포함 */

#define WORD_COUNT 10
#define FILE_PATH  "../words/tom-word-list.txt"

typedef struct {
    char *word; /* 키: 단어 */
    int  count; /* 값: 출현 빈도 */
    UT_hash_handle hh; /* uthash 핸들 */
} WordEntry;

/* ---------- 정렬 비교 함수 ---------- */
static int cmp_word_asc(const void *a, const void *b) {
    const WordEntry *ea = *(const WordEntry **)a;
    const WordEntry *eb = *(const WordEntry **)b;
    return strcmp(ea->word, eb->word);
}

static int cmp_freq_desc(const void *a, const void *b) {
    const WordEntry *ea = *(const WordEntry **)a;
    const WordEntry *eb = *(const WordEntry **)b;
    if (ea->count != eb->count) return eb->count - ea->count;
    return strcmp(ea->word, eb->word);
}

/* ---------- 메인 ---------- */
int main(int argc, char *argv[])
{
    /* [FR1] 인자 검증 */
    if (argc != 3) {
        fprintf(stderr, "사용법: %s 탐색지정단어 탐색지정빈도\n", argv[0]);
        return 1;
    }
    const char *search_word = argv[1];
    int search_freq = atoi(argv[2]);
    if (search_freq <= 0) {
        fprintf(stderr, "탐색지정빈도(%d) 입력 오류\n", search_freq);
        return 2;
    }

    /* [FR2] 파일 열기 */
    FILE *fp = fopen(FILE_PATH, "r");
    if (!fp) { perror(FILE_PATH); return 1; }

    /* [FR3] 단어 카운트 */
    WordEntry *table = NULL; /* 해시 테이블 헤드 */
    char token[1024];
    long long total_words = 0;

    while (fscanf(fp, "%1023s", token) == 1) {
        total_words++;

        WordEntry *e;
        HASH_FIND_STR(table, token, e);
        if (e) {
            e->count++; /* 기존 단어 */
        } else {
            e = malloc(sizeof(*e));
            if (!e) { perror("malloc"); fclose(fp); return 1; }
            e->word = strdup(token);
            if (!e->word) { perror("strdup"); fclose(fp); return 1; }
            e->count = 1;
            HASH_ADD_KEYPTR(hh, table, e->word, strlen(e->word), e);
        }
    }
    fclose(fp);

    size_t unique_words = HASH_COUNT(table);

    /* [P1] 요약 출력 */
    printf("[P1] 입력 단어 내역:\n");
    printf("입력 단어 총 갯수= %lld, 고유 단어 총 갯수= %zu\n",
           total_words, unique_words);

    /* 테이블 → 배열 복사 (정렬용) */
    WordEntry **arr = malloc(unique_words * sizeof(*arr));
    if (!arr) { perror("malloc"); return 1; }
    size_t idx = 0;
    WordEntry *cur, *tmp;
    HASH_ITER(hh, table, cur, tmp) arr[idx++] = cur;

    /* ---------- [P2] 단어 오름차순 ---------- */
    qsort(arr, unique_words, sizeof(*arr), cmp_word_asc);

    printf("\n[P2] 탐색지정 단어와 출현빈도:\n");
    printf("오름차순 정렬된 단어 기준 상하위 10개 단어와 출현빈도:\n");

    size_t limit = unique_words < WORD_COUNT ? unique_words : WORD_COUNT;
    for (idx = 0; idx < limit; ++idx)
        printf("('%s', %d)\n", arr[idx]->word, arr[idx]->count);

    if (unique_words > WORD_COUNT) {
        printf(".............\n");
        for (idx = unique_words - limit; idx < unique_words; ++idx)
            printf("('%s', %d)\n", arr[idx]->word, arr[idx]->count);
    }

    WordEntry *found_word;
    HASH_FIND_STR(table, search_word, found_word);
    if (found_word)
        printf("\n탐색지정단어('%s', %d)\n",
               search_word, found_word->count);
    else
        printf("\n탐색지정단어(%s) 없음\n", search_word);

    /* ---------- [P3] 빈도 내림차순 ---------- */
    qsort(arr, unique_words, sizeof(*arr), cmp_freq_desc);

    printf("\n[P3] 탐색지정빈도와 단어:\n");
    printf("출현빈도 기준 내림차순 정렬된 상하위 10개 출현빈도와 단어:\n");

    for (idx = 0; idx < limit; ++idx)
        printf("(%d, '%s')\n", arr[idx]->count, arr[idx]->word);

    if (unique_words > WORD_COUNT) {
        printf(".............\n");
        for (idx = unique_words - limit; idx < unique_words; ++idx)
            printf("(%d, '%s')\n", arr[idx]->count, arr[idx]->word);
    }

    printf("\n탐색지정빈도(%d)\n", search_freq);
    int freq_found = 0;
    for (idx = 0; idx < unique_words; ++idx) {
        if (arr[idx]->count == search_freq) {
            printf("(%d, '%s')\n", arr[idx]->count, arr[idx]->word);
            freq_found = 1;
        }
    }
    if (!freq_found) printf(" 해당 단어 없음\n");

    /* ---------- 메모리 해제 ---------- */
    for (idx = 0; idx < unique_words; ++idx) {
        free(arr[idx]->word);
        free(arr[idx]);
    }
    free(arr);

    return 0;
}