#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

int cmp(const struct list_head *a, const struct list_head *b)
{
    element_t *a_ele = container_of(a, element_t, list);
    element_t *b_ele = container_of(b, element_t, list);
    return (strcmp(a_ele->value, b_ele->value));
}

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *new = malloc(sizeof(struct list_head));
    if (!new)
        return NULL;
    INIT_LIST_HEAD(new);

    return new;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;

    element_t *tmp, *safe;
    list_for_each_entry_safe (tmp, safe, l, list) {
        q_release_element(tmp);
    }

    free(l);
    return;
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *tmp = malloc(sizeof(element_t));
    if (!tmp)
        return false;

    INIT_LIST_HEAD(&tmp->list);
    tmp->value = strdup(s);
    if (!tmp->value) {
        free(tmp);
        return false;
    }

    list_add(&tmp->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *tmp = malloc(sizeof(element_t));
    if (!tmp)
        return false;

    INIT_LIST_HEAD(&tmp->list);
    tmp->value = strdup(s);
    if (!tmp->value) {
        free(tmp);
        return false;
    }

    list_add_tail(&tmp->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *tmp = list_first_entry(head, element_t, list);
    list_del(&tmp->list);

    if (sp && bufsize) {
        strncpy(sp, tmp->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return tmp;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *tmp = list_last_entry(head, element_t, list);
    list_del(&tmp->list);

    if (sp && bufsize) {
        strncpy(sp, tmp->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return tmp;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;
    struct list_head *tmp, *safe;
    int count = 0;
    list_for_each_safe (tmp, safe, head) {
        count++;
    }
    return count;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;

    struct list_head **ind = &head->next;
    for (struct list_head *fast = head->next;
         fast != head && fast->next != head; fast = fast->next->next)
        ind = &(*ind)->next;

    element_t *tmp = list_entry(*ind, element_t, list);
    list_del(*ind);
    q_release_element(tmp);

    return true;
}

bool q_delete_dup(struct list_head *head)
{
    if (!head)
        return false;
    struct list_head *tmp, *safe;
    bool dup = false;
    list_for_each_safe (tmp, safe, head) {
        if (safe != head && !cmp(tmp, safe)) {
            element_t *ptr = list_entry(tmp, element_t, list);
            list_del(tmp);
            q_release_element(ptr);
            dup = true;
        } else if (dup) {
            element_t *ptr = list_entry(tmp, element_t, list);
            list_del(tmp);
            q_release_element(ptr);
            dup = false;
        }
    }
    return true;
}
/* Delete all nodes that have duplicate string */
// bool q_delete_dup(struct list_head *head)
// {
//     // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
//     if (!head)
//         return false;
//     element_t *entry, *safe;
//     bool dup = false;
//     list_for_each_entry_safe (entry, safe, head, list) {
//         if (&safe->list != head && !strcmp(entry->value, safe->value)) {
//             list_del(&entry->list);
//             q_release_element(entry);
//             dup = true;
//         } else if (dup) {
//             list_del(&entry->list);
//             q_release_element(entry);
//             dup = false;
//         }
//     }
//     return true;
// }

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head))
        return;

    struct list_head *one, *two;
    for (one = head->next, two = one->next; one != head && two != head;
         one = two, two = two->next) {
        one->prev->next = two;
        two->prev = one->prev;
        one->next = two->next;
        one->prev = two;
        two->next->prev = one;
        two->next = one;

        two = one->next;
    }
    return;
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *tmp, *safe;
    list_for_each_safe (tmp, safe, head) {
        tmp->next = tmp->prev;
        tmp->prev = safe;
    }
    tmp->next = tmp->prev;
    tmp->prev = safe;
    return;
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    int count = k;
    struct list_head *tmp, *cut, *safe;
    cut = head;
    list_for_each_safe (tmp, safe, head) {
        if (--count)
            continue;
        LIST_HEAD(current);
        count = k;
        list_cut_position(&current, cut, tmp);
        q_reverse(&current);
        list_splice(&current, cut);
        cut = safe->prev;
    }
    return;
}

typedef int
    __attribute__((nonnull(1, 2))) (*list_cmp_func_t)(const struct list_head *,
                                                      const struct list_head *);



static struct list_head *merge(list_cmp_func_t cmp,
                               struct list_head *a,
                               struct list_head *b)
{
    struct list_head *head = a, **tail = &head;

    for (;;) {
        /* if equal, take 'a' -- important for sort stability */
        if (cmp(a, b) <= 0) {
            *tail = a;
            tail = &a->next;
            a = a->next;
            if (!a) {
                *tail = b;
                break;
            }
        } else {
            *tail = b;
            tail = &b->next;
            b = b->next;
            if (!b) {
                *tail = a;
                break;
            }
        }
    }
    return head;
}

__attribute__((nonnull(1, 2, 3, 4))) static void merge_final(
    list_cmp_func_t cmp,
    struct list_head *head,
    struct list_head *a,
    struct list_head *b)
{
    struct list_head *tail = head;
    int count = 0;

    for (;;) {
        /* if equal, take 'a' -- important for sort stability */
        if (cmp(a, b) <= 0) {
            tail->next = a;
            a->prev = tail;
            tail = a;
            a = a->next;
            if (!a)
                break;
        } else {
            tail->next = b;
            b->prev = tail;
            tail = b;
            b = b->next;
            if (!b) {
                b = a;
                break;
            }
        }
    }

    /* Finish linking remainder of list b on to tail */
    tail->next = b;
    do {
        /*
         * If the merge is highly unbalanced (e.g. the input is
         * already sorted), this loop may run many iterations.
         * Continue callbacks to the client even though no
         * element comparison is needed, so the client's cmp()
         * routine can invoke cond_resched() periodically.
         */
        if (unlikely(!++count))
            cmp(b, b);
        b->prev = tail;
        tail = b;
        b = b->next;
    } while (b);

    /* And the final links to make a circular doubly-linked list */
    tail->next = head;
    head->prev = tail;
}

__attribute__((nonnull(1, 2))) void list_sort(struct list_head *head,
                                              list_cmp_func_t cmp)
{
    struct list_head *list = head->next, *pending = NULL;
    size_t count = 0; /* Count of pending */

    if (list == head->prev) /* Zero or one elements */
        return;

    /* Convert to a null-terminated singly-linked list. */
    head->prev->next = NULL;

    do {
        size_t bits;
        struct list_head **tail = &pending;

        /* Find the least-significant clear bit in count */
        for (bits = count; bits & 1; bits >>= 1)
            tail = &(*tail)->prev;
        /* Do the indicated merge */
        if (likely(bits)) {
            struct list_head *a = *tail, *b = a->prev;

            a = merge(cmp, b, a);
            /* Install the merged result in place of the inputs */
            a->prev = b->prev;
            *tail = a;
        }

        /* Move one element from input list to pending */
        list->prev = pending;
        pending = list;
        list = list->next;
        pending->next = NULL;
        count++;
    } while (list);

    /* End of input; merge together all the pending lists. */
    list = pending;
    pending = pending->prev;
    for (;;) {
        struct list_head *next = pending->prev;

        if (!next)
            break;
        list = merge(cmp, pending, list);
        pending = next;
    }
    /* The final merge, rebuilding prev links */
    merge_final(cmp, head, pending, list);
}

void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    list_sort(head, cmp);
    return;
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return 1;

    q_reverse(head);
    struct list_head *tmp = head->next;
    while (tmp->next != head) {
        if (cmp(tmp, tmp->next) < 0) {
            element_t *ptr = list_entry(tmp->next, element_t, list);
            list_del(tmp->next);
            q_release_element(ptr);
        } else {
            tmp = tmp->next;
        }
    }
    q_reverse(head);

    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return 1;

    q_reverse(head);
    struct list_head *tmp = head->next;
    while (tmp->next != head) {
        if (cmp(tmp, tmp->next) > 0) {
            element_t *ptr = list_entry(tmp->next, element_t, list);
            list_del(tmp->next);
            q_release_element(ptr);
        } else {
            tmp = tmp->next;
        }
    }
    q_reverse(head);

    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return q_size(list_entry(head->next, queue_contex_t, chain)->q);

    queue_contex_t *tmp = list_entry(head->next, queue_contex_t, chain);
    struct list_head *ptr = head->next->next;
    while (ptr != head) {
        queue_contex_t *tmp2 = list_entry(ptr, queue_contex_t, chain);
        list_splice_init(tmp2->q, tmp->q);
        ptr = ptr->next;
    }
    q_sort(tmp->q, descend);

    return q_size(tmp->q);
}

void swap(struct list_head *node_1, struct list_head *node_2)
{
    if (node_1 == node_2)
        return;
    struct list_head *tmp1 = node_1->prev;
    struct list_head *tmp2 = node_2->prev;
    if (node_1->prev != node_2)
        list_move(node_2, tmp1);
    list_move(node_1, tmp2);
}

void q_shuffle(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    int len = q_size(head);
    struct list_head *back = head->prev;
    struct list_head *cur = head->next;
    for (; len > 0; len--) {
        int random = rand() % len;
        if (!random)
            continue;
        while (--random) {
            cur = cur->next;
        }
        swap(cur, back);

        back = cur->prev;
        cur = head->next;
    }
}