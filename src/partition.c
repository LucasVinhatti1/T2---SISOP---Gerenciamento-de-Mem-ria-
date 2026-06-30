#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "partition.h"

typedef struct Block {
    int start;
    int size;
    int free;
    char id[64];
    struct Block *prev;
    struct Block *next;
} Block;

static Block *head = NULL;
static Policy current_policy;
static Block *circular_cursor = NULL;

void partition_init(int total_size, Policy policy) {
    head = (Block *) malloc(sizeof(Block));
    head->start = 0;
    head->size = total_size;
    head->free = 1;
    head->id[0] = '\0';
    head->prev = NULL;
    head->next = NULL;
    current_policy = policy;
    circular_cursor = head;
}

static Block *find_by_id(const char *id) {
    Block *b = head;
    while (b != NULL) {
        if (!b->free && strcmp(b->id, id) == 0) return b;
        b = b->next;
    }
    return NULL;
}

static Block *new_block_after(Block *ref, int start, int size) {
    Block *nb = (Block *) malloc(sizeof(Block));
    nb->start = start;
    nb->size = size;
    nb->free = 1;
    nb->id[0] = '\0';
    nb->prev = ref;
    nb->next = ref->next;
    if (ref->next != NULL) ref->next->prev = nb;
    ref->next = nb;
    return nb;
}

/* Divide o bloco 'b' (que deve estar livre) de modo que a parte inicial
   tenha exatamente 'size' unidades; o restante vira um novo bloco livre
   logo em seguida. Se nao sobrar resto, nao cria bloco extra. */
static void split_block(Block *b, int size) {
    if (b->size > size) {
        new_block_after(b, b->start + size, b->size - size);
        b->size = size;
    }
}

static Block *worst_fit_search(int size) {
    Block *best = NULL;
    Block *b = head;
    while (b != NULL) {
        if (b->free && b->size >= size) {
            if (best == NULL || b->size > best->size) {
                best = b;
            }
        }
        b = b->next;
    }
    return best;
}

static Block *circular_fit_search(int size) {
    if (head == NULL) return NULL;
    if (circular_cursor == NULL) circular_cursor = head;

    Block *start_node = circular_cursor;
    Block *b = circular_cursor;
    int wrapped_once = 0;

    do {
        if (b->free && b->size >= size) {
            return b;
        }
        b = b->next;
        if (b == NULL) {
            b = head;
            wrapped_once = 1;
        }
        if (wrapped_once && b == start_node) break;
    } while (b != start_node);

    /* checa o proprio start_node por completude do ciclo */
    if (start_node->free && start_node->size >= size) {
        return start_node;
    }

    return NULL;
}

int partition_alloc(const char *id, int size) {
    Block *chosen = NULL;

    if (current_policy == POLICY_WORST_FIT) {
        chosen = worst_fit_search(size);
    } else {
        chosen = circular_fit_search(size);
    }

    if (chosen == NULL) {
        return 0;
    }

    split_block(chosen, size);
    chosen->free = 0;
    strncpy(chosen->id, id, sizeof(chosen->id) - 1);
    chosen->id[sizeof(chosen->id) - 1] = '\0';

    if (current_policy == POLICY_CIRCULAR_FIT) {
        circular_cursor = (chosen->next != NULL) ? chosen->next : head;
    }

    return 1;
}

static int merge_with_next_if_free(Block *b) {
    if (b == NULL || b->next == NULL) return 0;
    if (b->free && b->next->free) {
        Block *nxt = b->next;
        b->size += nxt->size;
        b->next = nxt->next;
        if (nxt->next != NULL) nxt->next->prev = b;
        if (circular_cursor == nxt) circular_cursor = b;
        free(nxt);
        return 1;
    }
    return 0;
}

int partition_free(const char *id) {
    Block *b = find_by_id(id);
    if (b == NULL) return 0;

    b->free = 1;
    b->id[0] = '\0';

    /* Uma passada pela lista, fundindo em cadeia todos os blocos livres
       adjacentes (inclusive o bloco recem liberado com seus vizinhos). */
    Block *cur = head;
    while (cur != NULL) {
        while (merge_with_next_if_free(cur)) {
            /* continua fundindo enquanto houver vizinhos livres */
        }
        cur = cur->next;
    }

    return 1;
}

void partition_print_free_blocks(void) {
    Block *b = head;
    printf("|");
    int any = 0;
    while (b != NULL) {
        if (b->free) {
            printf(" %d |", b->size);
            any = 1;
        }
        b = b->next;
    }
    if (!any) {
        printf(" (nenhum espaco livre) |");
    }
    printf("\n");
}

void partition_destroy(void) {
    Block *b = head;
    while (b != NULL) {
        Block *nxt = b->next;
        free(b);
        b = nxt;
    }
    head = NULL;
    circular_cursor = NULL;
}
