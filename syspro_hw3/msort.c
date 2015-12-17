/** @file msort.c */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define MAXN 130000000

typedef struct {
    int *dat;
    size_t cnt;
} sort_struct;

typedef struct {
    int *d1;
    int *d2;
    int l1;
    int l2;
} merge_struct;

int cmp(const void* a, const void* b) {
    return *(int*)a - *(int*)b;
}

void *myqsort(void* _args) {
    sort_struct *args = (sort_struct*)_args;
    fprintf(stderr, "Sorted %zu elements.\n", args->cnt);
    qsort(args->dat, args->cnt, sizeof(int), cmp);
    return 0;
}

void *merge(void* _args) {
    merge_struct *args = (merge_struct*)_args;
    //printf("MERGE %d %d\n", args->l1, args->l2);
    int duplicate_cnt = 0, ptr1 = 0, ptr2 = 0;
    int *temp = (int*)malloc(sizeof(int) * (args->l1 + args->l2));
    int *ptr = temp;
    while (ptr1 < args->l1 || ptr2 < args->l2) {
        //printf("%d %d\n", ptr1, ptr2);
        if (ptr2 == args->l2 || (ptr1 != args->l1 && args->d1[ptr1] < args->d2[ptr2])) {
            *(ptr++) = args->d1[ptr1++];
        } else if (ptr1 == args->l1 || (ptr2 != args->l2 && args->d2[ptr2] < args->d1[ptr1])) {
            *(ptr++) = args->d2[ptr2++];
        } else {
            duplicate_cnt++;
            *(ptr++) = args->d1[ptr1++];
        }
    }
    memcpy(args->d1, temp, sizeof(int) * (args->l1 + args->l2));
    free(temp);
    fprintf(stderr, "Merged %d and %d elements with %d duplicates.\n", args->l1, args->l2, duplicate_cnt);
    return 0;
}

void msort(int *dat, int input_ct, int segment_count) {
    int values_per_segment, i, j;
    if (input_ct % segment_count == 0) {
        values_per_segment = input_ct / segment_count;
    } else {
        values_per_segment = (input_ct / segment_count) + 1;
    }
    //fprintf(stderr, "input_ct=%d, segment_count=%d, vps=%d\n", input_ct, segment_count, values_per_segment);
    pthread_t p[segment_count];
    sort_struct args[segment_count];
    for (i = 0; i < segment_count; ++i) {
        args[i].dat = dat + i * values_per_segment;
        if (i != segment_count - 1 || input_ct % segment_count == 0) {
            args[i].cnt = values_per_segment;
        } else {
            args[i].cnt = input_ct % values_per_segment;
        }
        pthread_create(&p[i], NULL, (void*)myqsort, &args[i]);
    }
    for (i = 0; i < segment_count; ++i) {
        pthread_join(p[i], NULL);
    }
    for (i = values_per_segment; i < input_ct; i <<= 1) {
        int cnt = input_ct / (i * 2) + (input_ct % (i * 2) > 0);
        //printf("i=%d cnt=%d\n", i, cnt);
        pthread_t p[cnt];
        merge_struct args[cnt];
        for (j = 0; j < cnt; ++j) {
            if (j == cnt - 1 && input_ct % (i * 2) <= i) {
                continue;
            }
            args[j].d1 = dat + j * 2 * i;
            args[j].d2 = dat + (j * 2 + 1) * i;
            args[j].l1 = i;
            if (j != cnt - 1 || input_ct % i == 0) {
                args[j].l2 = i;
            } else {
                args[j].l2 = input_ct % i;
            }
            pthread_create(&p[j], NULL, (void*)merge, &args[j]);
        }
        for (j = 0; j < cnt; ++j) {
            if (j == cnt - 1 && input_ct % (i * 2) <= i) {
                continue;
            }
            //printf("j=%d\n", j);
            pthread_join(p[j], NULL);
        }
    }
}

void output(int *dat, int input_ct) {
    int i;
    for (i = 0; i < input_ct; ++i) {
        printf("%d\n", dat[i]);
    }
}

int dat[MAXN];

int main(int argc, char **argv) {
    // init
    int input_ct = 0;
    int segment_count = atoi(argv[1]);
    while (scanf("%d", &dat[input_ct]) == 1) {
        input_ct++;
    }
    // merge sort
    msort(dat, input_ct, segment_count);
    // output result
    output(dat, input_ct);
    return 0;
}
