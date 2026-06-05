#include "memory.h"
#include "uart.h"
#include "task.h"
#include "scheduler.h"

/*
 * Imprime um número inteiro pela UART.
 *
 * Como não existe printf no kernel,
 * a conversão para texto é feita manualmente.
 */
static void print_heap_status(void)
{
    uart_print("Heap total : "); uart_print_uint(memory_total()); uart_print(" bytes\n");
    uart_print("Heap usado : "); uart_print_uint(memory_used());  uart_print(" bytes\n");
    uart_print("Heap livre : "); uart_print_uint(memory_free());  uart_print(" bytes\n");
    uart_print("\n");
}
/*
 * Task de exemplo.
 */
void task1(void)
{
    while (1)
    {
        uart_print("[Task 1] executando\n");
        yield();
    }
}

/*
 * Outra task de exemplo.
 */
void task2(void)
{
    while (1)
    {
        uart_print("[Task 2] executando\n");
        yield();
    }
}

/*
 * Função principal do kernel.
 *
 * Executa vários testes para demonstrar
 * o funcionamento do gerenciador de memória:
 *
 * - alocação de blocos
 * - liberação de memória
 * - reutilização de espaço livre
 * - divisão de blocos
 * - junção de blocos livres
 * - estatísticas do heap
 * - integração com tasks
 */
void kernel_main(void)
{
    memory_init();

    uart_print("=== BasicMicrokernel - Free List Allocator ===\n\n");

    /* Mostra o estado inicial do heap */
    uart_print("--- Estado inicial do heap ---\n");
    print_heap_status();

    /* Teste de múltiplas alocações */
    uart_print("--- Cenario 1: multiplas alocacoes ---\n");

    void *p1 = kmalloc(256);
    void *p2 = kmalloc(512);
    void *p3 = kmalloc(128);

    uart_print("Alocados: 256 + 512 + 128 bytes\n");
    print_heap_status();

    /* Teste da divisão de blocos */
    uart_print("--- Cenario 2: divisao de blocos ---\n");

    void *p4 = kmalloc(1024);

    uart_print("Alocados mais 1024 bytes\n");
    print_heap_status();

    /* Teste de liberação de memória */
    uart_print("--- Cenario 3: liberacao de memoria (kfree) ---\n");

    kfree(p2);

    uart_print("Liberado: bloco de 512 bytes\n");
    print_heap_status();

    /* Teste de reutilização de espaço livre */
    uart_print("--- Cenario 4: reutilizacao de bloco liberado ---\n");

    void *p5 = kmalloc(256);

    uart_print("Realocado: 256 bytes\n");
    print_heap_status();

    /* Teste da coalescência */
    uart_print("--- Cenario 5: coalescencia de blocos adjacentes ---\n");

    kfree(p1);
    kfree(p5);
    kfree(p3);

    uart_print("Blocos livres adjacentes foram unidos\n");
    print_heap_status();

    /* Teste com criação e remoção de tasks */
    uart_print("--- Cenario 6: integracao com tasks ---\n");

    kfree(p4);

    memory_init();

    uart_print("Heap reiniciado para demonstracao das tasks\n");
    print_heap_status();

    xTaskCreate(task1, 2048, 1);
    uart_print("Task 1 criada\n");
    print_heap_status();

    xTaskCreate(task2, 2048, 1);
    uart_print("Task 2 criada\n");
    print_heap_status();

    /* Remove a primeira task */
    xTaskDestroy(0);

    uart_print("Task 1 removida\n");
    print_heap_status();

    /* Cria novamente para mostrar reutilização da memória */
    xTaskCreate(task1, 2048, 1);

    uart_print("Task 1 recriada\n");
    print_heap_status();

    uart_print("=== Iniciando scheduler ===\n\n");

    scheduler_start();

    while (1);
}