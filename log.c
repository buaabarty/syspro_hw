/** @file log.c */
#include <stdlib.h>
#include <string.h>
#include "log.h"

/**
 * Initializes the log.
 *
 * You may assuem that:
 * - This function will only be called once per instance of log_t.
 * - This function will be the first function called per instance of log_t.
 * - All pointers will be valid, non-NULL pointer.
 *
 * @returns
 *   The initialized log structure.
 */
void log_init(log_t *l) {
    l->head = NULL;
}

/**
 * Frees all internal memory associated with the log.
 *
 * You may assume that:
 * - This function will be called once per instance of log_t.
 * - This funciton will be the last function called per instance of log_t.
 * - All pointers will be valid, non-NULL pointer.
 *
 * @param l
 *    Pointer to the log data structure to be destoryed.
 */
void log_destroy(log_t* l) {
    if (!l->head) {
        return;
    }
    node* h = l->head;
    while (h->next) {
        node* t = h->next;
        free(h->cmd);
        free(h);
        h = t;
    }
    free(h->cmd);
    free(h);
    l->head = NULL;
}

/**
 * Push an item to the log stack.
 *
 *
 * You may assume that:
* - All pointers will be valid, non-NULL pointer.
*
 * @param l
 *    Pointer to the log data structure.
 * @param item
 *    Pointer to a string to be added to the log.
 */
void log_push(log_t* l, const char *item) {
    if (l->head == NULL) {
        l->head = (node*)malloc(sizeof(node));
        l->head->next = NULL;
        l->head->cmd = (char*)malloc(strlen(item)+1);
        strcpy(l->head->cmd, item);
        return;
    }
    node* t = l->head;
    while (t->next) t = t->next;
    t->next = (node*)malloc(sizeof(node));
    t = t->next;
    t->next = NULL;
    t->cmd = (char*)malloc(strlen(item)+1);
    strcpy(t->cmd, item);
}


/**
 * Preforms a newest-to-oldest search of log entries for an entry matching a
 * given prefix.
 *
 * This search starts with the most recent entry in the log and
 * compares each entry to determine if the query is a prefix of the log entry.
 * Upon reaching a match, a pointer to that element is returned.  If no match
 * is found, a NULL pointer is returned.
 *
 *
 * You may assume that:
 * - All pointers will be valid, non-NULL pointer.
 *
 * @param l
 *    Pointer to the log data structure.
 * @param prefix
 *    The prefix to test each entry in the log for a match.
 *
 * @returns
 *    The newest entry in the log whose string matches the specified prefix.
 *    If no strings has the specified prefix, NULL is returned.
 */
char *log_search(log_t* l, const char *prefix) {
    if (l->head == NULL) {
         return NULL;
    }
    char* ret = NULL;
    node* t = l->head;
    while (t) {
        if (!strncmp(t->cmd, prefix, strlen(prefix))) {
            ret = t->cmd;
        }
        t = t->next;
    }
    return ret;
}
