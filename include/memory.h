#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

/*
 * Gerenciador de memória usando lista de blocos livres.
 *
 * O heap é dividido em blocos encadeados.
 * Cada bloco guarda:
 *   - seu tamanho
 *   - se está livre ou em uso
 *   - qual é o próximo bloco da lista
 *
 * A alocação utiliza a estratégia First Fit,
 * ou seja, usa o primeiro bloco livre que
 * tenha espaço suficiente.
 */

/* Informações de cada bloco do heap */
typedef struct block
{
    uint64_t     size;   /* Tamanho do bloco em bytes */
    int          free;   /* 1 = livre | 0 = ocupado */
    struct block *next;  /* Próximo bloco da lista */
} block_t;

/* Cria o heap inicial com um único bloco livre */
void     memory_init(void);

/* Reserva uma área de memória do tamanho solicitado */
void    *kmalloc(uint64_t size);

/* Libera uma área de memória anteriormente alocada */
void     kfree(void *ptr);

/* Retorna quantos bytes estão atualmente em uso */
uint64_t memory_used(void);

/* Retorna quantos bytes ainda estão livres */
uint64_t memory_free(void);

/* Retorna o tamanho total do heap */
uint64_t memory_total(void);

#endif /* MEMORY_H */