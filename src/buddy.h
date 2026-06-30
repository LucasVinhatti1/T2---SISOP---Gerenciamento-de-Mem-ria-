#ifndef BUDDY_H
#define BUDDY_H

void buddy_init(int total_size);
int  buddy_alloc(const char *id, int size);  /* 1 = ok, 0 = falhou */
int  buddy_free(const char *id);             /* 1 = ok, 0 = id nao encontrado */
void buddy_print_free_blocks(void);
int  buddy_total_internal_fragmentation(void);
void buddy_destroy(void);

#endif
