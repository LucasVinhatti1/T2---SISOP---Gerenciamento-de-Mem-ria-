#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "partition.h"
#include "buddy.h"

#define MAX_LINE 256
#define MAX_ID 64

typedef enum {
    CMD_IN,
    CMD_OUT,
    CMD_INVALID
} CmdType;

typedef struct {
    CmdType type;
    char id[MAX_ID];
    int size;
} Command;

static void trim(char *s) {
    /* remove espacos, \r e \n no inicio e no fim da string */
    int len = (int) strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r' ||
                        s[len - 1] == ' ' || s[len - 1] == '\t')) {
        s[--len] = '\0';
    }
    int start = 0;
    while (s[start] == ' ' || s[start] == '\t') start++;
    if (start > 0) memmove(s, s + start, strlen(s + start) + 1);
}

static Command parse_line(const char *raw_line) {
    Command cmd;
    cmd.type = CMD_INVALID;
    cmd.id[0] = '\0';
    cmd.size = 0;

    char line[MAX_LINE];
    strncpy(line, raw_line, MAX_LINE - 1);
    line[MAX_LINE - 1] = '\0';
    trim(line);

    if (line[0] == '\0') {
        return cmd; /* linha em branco */
    }

    if (strncmp(line, "IN(", 3) == 0) {
        char id[MAX_ID];
        int size;
        if (sscanf(line + 3, " %63[^, ] , %d", id, &size) == 2) {
            cmd.type = CMD_IN;
            strncpy(cmd.id, id, MAX_ID - 1);
            cmd.size = size;
        }
    } else if (strncmp(line, "OUT(", 4) == 0) {
        char id[MAX_ID];
        if (sscanf(line + 4, " %63[^) ]", id) == 1) {
            cmd.type = CMD_OUT;
            strncpy(cmd.id, id, MAX_ID - 1);
        }
    }

    return cmd;
}

static int is_power_of_two(int n) {
    return n > 0 && (n & (n - 1)) == 0;
}

static int read_int(const char *prompt) {
    int value;
    char buf[64];
    while (1) {
        printf("%s", prompt);
        if (fgets(buf, sizeof(buf), stdin) == NULL) continue;
        if (sscanf(buf, "%d", &value) == 1) return value;
        printf("Entrada inválida, tente novamente.\n");
    }
}

int main(void) {
    printf("==============================================\n");
    printf(" Trabalho Prático 2 - Gerenciamento de Memória\n");
    printf("==============================================\n\n");

    int mem_size;
    do {
        mem_size = read_int("Informe o tamanho da memória principal (potência de 2): ");
        if (!is_power_of_two(mem_size)) {
            printf("O tamanho deve ser uma potência de 2. Tente novamente.\n");
        }
    } while (!is_power_of_two(mem_size));

    printf("\nEscolha o tipo de gerenciamento de memória:\n");
    printf("  1 - Partições Variáveis\n");
    printf("  2 - Sistema Buddy\n");
    int mode = read_int("Opcao: ");

    int use_buddy = (mode == 2);
    Policy policy = POLICY_WORST_FIT;

    if (!use_buddy) {
        printf("\nEscolha a política de alocação:\n");
        printf("  1 - Worst-Fit\n");
        printf("  2 - Circular-Fit\n");
        int pol = read_int("Opcao: ");
        policy = (pol == 2) ? POLICY_CIRCULAR_FIT : POLICY_WORST_FIT;
        partition_init(mem_size, policy);
    } else {
        buddy_init(mem_size);
    }

    char filename[256];
    printf("\nInforme o caminho do arquivo de requisições: ");
    if (fgets(filename, sizeof(filename), stdin) == NULL) {
        fprintf(stderr, "Erro ao ler nome do arquivo.\n");
        return 1;
    }
    filename[strcspn(filename, "\r\n")] = '\0';

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Não foi possível abrir o arquivo '%s'.\n", filename);
        return 1;
    }

    printf("\n--- Estado inicial ---\n");
    if (use_buddy) {
        buddy_print_free_blocks();
    } else {
        partition_print_free_blocks();
    }

    char raw_line[MAX_LINE];
    int line_number = 0;

    while (fgets(raw_line, sizeof(raw_line), fp) != NULL) {
        line_number++;
        Command cmd = parse_line(raw_line);

        if (cmd.type == CMD_INVALID) {
            continue; /* ignora linhas em branco ou mal formatadas */
        }

        printf("\n--- Linha %d: ", line_number);
        if (cmd.type == CMD_IN) {
            printf("IN(%s, %d) ---\n", cmd.id, cmd.size);
            int ok = use_buddy ? buddy_alloc(cmd.id, cmd.size)
                                : partition_alloc(cmd.id, cmd.size);
            if (!ok) {
                printf("ESPAÇO INSUFICIENTE DE MEMÓRIA\n");
            }
        } else {
            printf("OUT(%s) ---\n", cmd.id);
            int ok = use_buddy ? buddy_free(cmd.id) : partition_free(cmd.id);
            if (!ok) {
                printf("Processo '%s' não encontrado na memória.\n", cmd.id);
            }
        }

        if (use_buddy) {
            buddy_print_free_blocks();
            printf("Fragmentação interna total no momento: %d\n",
                   buddy_total_internal_fragmentation());
        } else {
            partition_print_free_blocks();
        }
    }

    fclose(fp);

    if (use_buddy) {
        printf("\n--- Fragmentação interna total ao final da execução: %d ---\n",
               buddy_total_internal_fragmentation());
        buddy_destroy();
    } else {
        partition_destroy();
    }

    printf("\nProcessamento concluído.\n");
    return 0;
}
