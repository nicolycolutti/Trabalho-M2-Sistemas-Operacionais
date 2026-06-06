#include "memory.h"

/*
 * Configuração do heap
 *
 * O heap é uma área fixa de memória com 64 KB.
 * Quando o sistema inicia, todo esse espaço é tratado
 * como um único bloco livre.
 *
 * Cada bloco possui um cabeçalho com informações sobre:
 * - tamanho do bloco (área útil, sem contar o header)
 * - se está livre ou ocupado
 * - próximo bloco da lista
 */

#define HEAP_SIZE 0x10000 /* 64 KB */

/* Área de memória usada pelo alocador */
static uint8_t heap[HEAP_SIZE];

/* Primeiro bloco da lista */
static block_t* free_list;

/*
 * Ajusta o tamanho para múltiplos de 8 bytes.
 * Isso garante alinhamento correto para RV64.
 *
 * Exemplos:
 * 1  -> 8
 * 8  -> 8
 * 9  -> 16
 */
static inline uint64_t align8(uint64_t n) {
    return (n + 7ULL) & ~7ULL;
}

/*
 * Inicializa o heap.
 *
 * Todo o espaço disponível vira um único bloco livre.
 * O header do primeiro bloco ocupa sizeof(block_t) bytes,
 * e o restante é a área útil disponível para alocação.
 */
void memory_init(void) {
    free_list = (block_t*)heap;
    free_list->size = HEAP_SIZE - sizeof(block_t);
    free_list->free = 1;
    free_list->next = 0;
}

/*
 * Reserva um bloco de memória.
 *
 * Usa a estratégia First Fit:
 * percorre a lista e usa o primeiro bloco livre
 * que tenha espaço suficiente para a solicitação.
 */
void* kmalloc(uint64_t size) {
    if (size == 0)
        return 0;

    /* Ajusta para múltiplos de 8 */
    size = align8(size);

    block_t* cur = free_list;

    while (cur) {
        /* Verifica se o bloco pode ser usado */
        if (cur->free && cur->size >= size) {
            /*
             * Se sobrar espaço suficiente após a alocação,
             * divide o bloco em dois: um ocupado e outro livre.
             * O mínimo para dividir é: header + 8 bytes úteis.
             */
            uint64_t espaco_restante = cur->size - size;

            if (espaco_restante > sizeof(block_t) + 8) {
                block_t* novo = (block_t*)((uint8_t*)(cur + 1) + size);

                novo->size = espaco_restante - sizeof(block_t);
                novo->free = 1;
                novo->next = cur->next;

                cur->size = size;
                cur->next = novo;
            }

            /* Marca como ocupado */
            cur->free = 0;

            /* Retorna apenas a área útil do bloco (após o header) */
            return (void*)(cur + 1);
        }

        cur = cur->next;
    }

    /* Não encontrou espaço disponível */
    return 0;
}

/*
 * Libera um bloco anteriormente alocado.
 *
 * Depois de liberar, executa coalescência em loop:
 * continua passando pela lista e unindo blocos livres
 * adjacentes até não restar mais nenhum par para unir.
 *
 * Isso garante que blocos liberados fora de ordem
 * sejam corretamente unidos em todas as passagens.
 */
void kfree(void* ptr) {
    if (!ptr)
        return;

    /* Recupera o cabeçalho do bloco (posição imediatamente antes do ptr) */
    block_t* bloco = ((block_t*)ptr) - 1;

    /* Valida que o ponteiro pertence ao heap */
    if ((uint8_t*)bloco < heap || (uint8_t*)bloco >= heap + HEAP_SIZE)
        return;

    /* Marca como livre */
    bloco->free = 1;

    /*
     * Coalescência em loop:
     * Repete a varredura enquanto ainda houver blocos
     * livres adjacentes para unir. Isso é necessário porque
     * uma única passagem pode não resolver todos os casos
     * quando blocos são liberados fora de ordem.
     *
     * Exemplo de caso que exige múltiplas passagens:
     *   [livre][ocupado][livre][livre]
     *   -> 1ª passagem une os dois últimos: [livre][ocupado][livre grande]
     *   -> se o do meio for liberado depois, nova passagem une tudo
     */
    int houve_coalescencia;

    do {
        houve_coalescencia = 0;
        block_t* cur = free_list;

        while (cur && cur->next) {
            if (cur->free && cur->next->free) {
                /* Une os dois blocos: absorve o header e o tamanho do próximo */
                cur->size += sizeof(block_t) + cur->next->size;
                cur->next = cur->next->next;
                houve_coalescencia = 1;
                /* Não avança cur: o próximo agora pode também ser livre */
            } else {
                cur = cur->next;
            }
        }
    } while (houve_coalescencia);
}

/*
 * Retorna a quantidade de memória útil atualmente ocupada.
 *
 * Nota: não inclui o overhead dos headers de cada bloco.
 * Para o total real consumido, some memory_overhead().
 */
uint64_t memory_used(void) {
    block_t* cur = free_list;
    uint64_t used = 0;

    while (cur) {
        if (!cur->free)
            used += cur->size;

        cur = cur->next;
    }

    return used;
}

/*
 * Retorna a quantidade de memória útil livre.
 *
 * Nota: não inclui o overhead dos headers de cada bloco.
 */
uint64_t memory_free(void) {
    block_t* cur = free_list;
    uint64_t livre = 0;

    while (cur) {
        if (cur->free)
            livre += cur->size;

        cur = cur->next;
    }

    return livre;
}

/*
 * Retorna o overhead total dos headers presentes no heap.
 *
 * Cada bloco (livre ou ocupado) consome sizeof(block_t) bytes
 * de metadado além da sua área útil. Esta função soma esse custo.
 *
 * Invariante: memory_used() + memory_free() + memory_overhead() == HEAP_SIZE
 */
uint64_t memory_overhead(void) {
    block_t* cur = free_list;
    uint64_t overhead = 0;

    while (cur) {
        overhead += sizeof(block_t);
        cur = cur->next;
    }

    return overhead;
}

/*
 * Retorna o tamanho total do heap em bytes.
 */
uint64_t memory_total(void) {
    return (uint64_t)HEAP_SIZE;
}