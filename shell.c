/** @file shell.c */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "log.h"

#define MAX_SIZE 1000

log_t Log;

void list_all_log(log_t *l) {
    if (l->head) {
        node* t = l->head;
        while (t) {
            printf("%s\n", t->cmd);
            t = t->next;
        }
    }
}

int split(char *args[], char* str, const char* spl) {
    int n = 0;
    char *result = NULL;
    result = strtok(str, spl);
    while(result != NULL) {
        args[n] = (char*)malloc(sizeof(char) * (strlen(result) + 1));
        strcpy(args[n++], result);
        result = strtok(NULL, spl);
    }
    return n;
}

int execute(char* buffer) {
    int i = 0;
    if (buffer[0] != '!') {
        log_push(&Log, buffer);
    }
    if (!strncmp(buffer, "cd", 2)) {
        while (buffer[i] && buffer[i] != ' ') ++i;
        if (chdir(buffer+i+1) < 0) {
            printf("%s: No such file or directory\n", buffer+i+1);
        }
    } else if (!strcmp(buffer, "exit")) {
        return 0;
    } else if (!strcmp(buffer, "!#")) {
        list_all_log(&Log);
    } else if (!strncmp(buffer, "!", 1)) {
        char *ret = log_search(&Log, buffer+1);
        if (!ret) {
            printf("No Match\n");
        } else {
            execute(ret);
        }
    } else if (strlen(buffer)) {
        char* args[10];
        for (i = 0; i < 10; ++i) {
            args[i] = NULL;
        }
        split(args, buffer, " ");
        pid_t pid = fork();
        if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
        } else {
            if (execv(buffer, args) < 0) {
                printf("%s: not found\n", buffer);
            }
            log_destroy(&Log);
            for (i = 0; i < 10; ++i) {
                free(args[i]);
            }
            exit(0);
        }
        for (i = 0; i < 10; ++i) {
            free(args[i]);
        }
    }
    return 1;
}

void prefix() {
    char* cwdstr = (char*)malloc(sizeof(char) * MAX_SIZE);
    printf("(pid=%d)%s$ ", getpid(), getcwd(cwdstr, MAX_SIZE));
    free(cwdstr);
}

/**
 * Starting point for shell.
 */
int main() {
    log_init(&Log);
    size_t len;
    prefix();
    char* buffer = NULL;
    while (~getline(&buffer, &len, stdin)) {
        buffer[strlen(buffer)-1] = '\0';
        if (!execute(buffer)) {
            break;
        }
        prefix();
        free(buffer);
        buffer = NULL;
    }
    if (buffer != NULL) {
         free(buffer);
         buffer = NULL;
    }
    log_destroy(&Log);
    return 0;
}
