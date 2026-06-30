#ifndef PARTITION_H
#define PARTITION_H

typedef enum {
    POLICY_WORST_FIT,
    POLICY_CIRCULAR_FIT
} Policy;

void partition_init(int total_size, Policy policy);
int  partition_alloc(const char *id, int size);   /* 1 = ok, 0 = falhou */
int  partition_free(const char *id);              /* 1 = ok, 0 = id nao encontrado */
void partition_print_free_blocks(void);
void partition_destroy(void);

#endif
