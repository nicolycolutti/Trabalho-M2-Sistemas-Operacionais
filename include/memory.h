#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

/*
 * Gerenciador de memória usando lista de blocos livres.
 *
 * O heap é dividido em blocos encadeados.
 * Cada bloco guarda:
 *   - seu tamanho (área útil, sem o header)
 *   - se está livre ou em uso
 *   - qual é o próximo bloco da lista
 *
 * A alocação utiliza a estratégia First Fit,
 * ou seja, usa o primeiro bloco livre que
 * tenha espaço suficiente.
 *
 * Relação entre as funções de estatística:
 *   memory_used() + memory_free() + memory_overhead() == memory_total()
 */

/* Cabeçalho de cada bloco do heap */
typedef struct block {
    uint64_t size;      /* Tamanho da área útil em bytes (sem o header) */
    int free;           /* 1 = livre | 0 = ocupado                      */
    struct block* next; /* Próximo bloco da lista                        */
} block_t;

/* Cria o heap inicial com um único bloco livre */
void memory_init(void);

/* Reserva uma área de memória do tamanho solicitado (First Fit) */
void* kmalloc(uint64_t size);

/* Libera uma área de memória e executa coalescência */
void kfree(void* ptr);

/* Retorna quantos bytes úteis estão atualmente em uso */
uint64_t memory_used(void);

/* Retorna quantos bytes úteis ainda estão livres */
uint64_t memory_free(void);

/* Retorna o total consumido pelos headers de todos os blocos */
uint64_t memory_overhead(void);

/* Retorna o tamanho total do heap */
uint64_t memory_total(void);

#endif /* MEMORY_H */