#include "memory.h"

/*
 * Configuração do heap
 *
 * O heap é uma área fixa de memória com 64 KB.
 * Quando o sistema inicia, todo esse espaço é tratado
 * como um único bloco livre.
 *
 * Cada bloco possui um cabeçalho com informações sobre:
 * - tamanho do bloco
 * - se está livre ou ocupado
 * - próximo bloco da lista
 */

#define HEAP_SIZE 0x10000   /* 64 KB */

/* Área de memória usada pelo alocador */
static uint8_t heap[HEAP_SIZE];

/* Primeiro bloco da lista */
static block_t *free_list;

/*
 * Ajusta o tamanho para múltiplos de 8 bytes.
 * Isso melhora o alinhamento e o acesso à memória.
 *
 * Exemplos:
 * 1  -> 8
 * 8  -> 8
 * 9  -> 16
 */
static inline uint64_t align8(uint64_t n)
{
    return (n + 7ULL) & ~7ULL;
}

/*
 * Inicializa o heap.
 *
 * Todo o espaço disponível vira um único bloco livre.
 */
void memory_init(void)
{
    free_list        = (block_t *)heap;
    free_list->size  = HEAP_SIZE - sizeof(block_t);
    free_list->free  = 1;
    free_list->next  = 0;
}

/*
 * Reserva um bloco de memória.
 *
 * Usa a estratégia First Fit:
 * procura o primeiro bloco livre que tenha
 * espaço suficiente para a solicitação.
 */
void *kmalloc(uint64_t size)
{
    if (size == 0)
        return 0;

    /* Ajusta para múltiplos de 8 */
    size = align8(size);

    block_t *cur = free_list;

    while (cur)
    {
        /* Verifica se o bloco pode ser usado */
        if (cur->free && cur->size >= size)
        {
            /*
             * Se sobrar espaço suficiente, divide o bloco
             * em dois: um ocupado e outro livre.
             */
            uint64_t espaco_restante = cur->size - size;

            if (espaco_restante > sizeof(block_t) + 8)
            {
                block_t *novo = (block_t *)((uint8_t *)(cur + 1) + size);

                novo->size  = espaco_restante - sizeof(block_t);
                novo->free  = 1;
                novo->next  = cur->next;

                cur->size   = size;
                cur->next   = novo;
            }

            /* Marca como ocupado */
            cur->free = 0;

            /* Retorna apenas a área útil do bloco */
            return (void *)(cur + 1);
        }

        cur = cur->next;
    }

    /* Não encontrou espaço disponível */
    return 0;
}

/*
 * Libera um bloco anteriormente alocado.
 *
 * Depois de liberar, tenta juntar blocos livres
 * vizinhos para evitar fragmentação.
 */
void kfree(void *ptr)
{
    if (!ptr)
        return;

    /* Recupera o cabeçalho do bloco */
    block_t *bloco = ((block_t *)ptr) - 1;

    /* Marca como livre */
    bloco->free = 1;

    /*
     * Junta blocos livres consecutivos.
     * Isso cria blocos maiores e reduz
     * a fragmentação da memória.
     */
    block_t *cur = free_list;

    while (cur && cur->next)
    {
        if (cur->free && cur->next->free)
        {
            cur->size += sizeof(block_t) + cur->next->size;
            cur->next  = cur->next->next;
        }
        else
        {
            cur = cur->next;
        }
    }
}

/*
 * Retorna a quantidade de memória atualmente ocupada.
 */
uint64_t memory_used(void)
{
    block_t  *cur  = free_list;
    uint64_t  used = 0;

    while (cur)
    {
        if (!cur->free)
            used += cur->size;

        cur = cur->next;
    }

    return used;
}

/*
 * Retorna a quantidade de memória livre.
 */
uint64_t memory_free(void)
{
    block_t  *cur   = free_list;
    uint64_t livre  = 0;

    while (cur)
    {
        if (cur->free)
            livre += cur->size;

        cur = cur->next;
    }

    return livre;
}

/*
 * Retorna o tamanho total do heap.
 */
uint64_t memory_total(void)
{
    return (uint64_t)HEAP_SIZE;
}