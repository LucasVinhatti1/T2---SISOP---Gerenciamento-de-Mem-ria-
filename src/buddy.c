#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buddy.h"

typedef struct Block {
    int start;
    int size;       /* tamanho real do bloco (potencia de 2) */
    int free;
    char id[64];
    int requested;  /* tamanho efetivamente pedido pelo processo (para fragmentacao) */
    struct Block *prev;
    struct Block *next;
} Block;

static Block *head = NULL;
static int memory_total = 0;

static int next_power_of_two(int n) {
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

void buddy_init(int total_size) {
    memory_total = total_size;
    head = (Block *) malloc(sizeof(Block));
    head->start = 0;
    head->size = total_size;
    head->free = 1;
    head->id[0] = '\0';
    head->requested = 0;
    head->prev = NULL;
    head->next = NULL;
}

static Block *find_by_id(const char *id) {
    Block *b = head;
    while (b != NULL) {
        if (!b->free && strcmp(b->id, id) == 0) return b;
        b = b->next;
    }
    return NULL;
}

/* Procura o menor bloco livre cujo tamanho seja >= target (target ja eh
   potencia de 2). Esse eh o bloco que sera repetidamente dividido ao meio
   ate atingir exatamente o tamanho 'target'. */
static Block *find_smallest_fit(int target) {
    Block *best = NULL;
    Block *b = head;
    while (b != NULL) {
        if (b->free && b->size >= target) {
            if (best == NULL || b->size < best->size) {
                best = b;
            }
        }
        b = b->next;
    }
    return best;
}

static Block *insert_after(Block *ref, int start, int size) {
    Block *nb = (Block *) malloc(sizeof(Block));
    nb->start = start;
    nb->size = size;
    nb->free = 1;
    nb->id[0] = '\0';
    nb->requested = 0;
    nb->prev = ref;
    nb->next = ref->next;
    if (ref->next != NULL) ref->next->prev = nb;
    ref->next = nb;
    return nb;
}

/* Divide 'b' ao meio sucessivamente ate seu tamanho ser igual a 'target'.
   A metade direita de cada divisao vira um novo bloco livre na lista;
   a metade esquerda continua sendo o bloco 'b' (e e devolvida). */
static void split_down(Block *b, int target) {
    while (b->size > target) {
        int half = b->size / 2;
        insert_after(b, b->start + half, half);
        b->size = half;
    }
}

int buddy_alloc(const char *id, int size) {
    int target = next_power_of_two(size);
    Block *chosen = find_smallest_fit(target);
    if (chosen == NULL) {
        return 0;
    }
    split_down(chosen, target);
    chosen->free = 0;
    chosen->requested = size;
    strncpy(chosen->id, id, sizeof(chosen->id) - 1);
    chosen->id[sizeof(chosen->id) - 1] = '\0';
    return 1;
}

static void remove_block(Block *b) {
    if (b->prev != NULL) b->prev->next = b->next;
    if (b->next != NULL) b->next->prev = b->prev;
    if (head == b) head = b->next;
    free(b);
}

static Block *find_buddy(Block *b) {
    /* Em um sistema buddy, o endereco do "irmao" de um bloco que comeca
       em 'start' e tem tamanho 'size' e obtido invertendo o bit
       correspondente ao tamanho do bloco no endereco (start XOR size). */
    int buddy_start = b->start ^ b->size;
    Block *cur = head;
    while (cur != NULL) {
        if (cur != b && cur->start == buddy_start && cur->size == b->size) {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}

int buddy_free(const char *id) {
    Block *b = find_by_id(id);
    if (b == NULL) return 0;

    b->free = 1;
    b->id[0] = '\0';
    b->requested = 0;

    /* Coalescencia: tenta repetidamente unir o bloco com seu buddy,
       enquanto o buddy existir, estiver livre e tiver o mesmo tamanho. */
    while (b->size < memory_total) {
        Block *buddy = find_buddy(b);
        if (buddy == NULL || !buddy->free) break;

        int new_start = (b->start < buddy->start) ? b->start : buddy->start;
        int new_size = b->size * 2;

        Block *to_remove = (buddy == b->next) ? buddy : b;
        Block *survivor = (to_remove == buddy) ? b : buddy;

        survivor->start = new_start;
        survivor->size = new_size;
        survivor->free = 1;
        remove_block(to_remove);

        b = survivor;
    }

    return 1;
}

void buddy_print_free_blocks(void) {
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

int buddy_total_internal_fragmentation(void) {
    int total = 0;
    Block *b = head;
    while (b != NULL) {
        if (!b->free) {
            total += (b->size - b->requested);
        }
        b = b->next;
    }
    return total;
}

void buddy_destroy(void) {
    Block *b = head;
    while (b != NULL) {
        Block *nxt = b->next;
        free(b);
        b = nxt;
    }
    head = NULL;
}
