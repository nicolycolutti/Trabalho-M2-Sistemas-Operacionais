#include "memory.h"
#include "scheduler.h"
#include "task.h"
#include "uart.h"

/*
 * Imprime o estado atual do heap pela UART.
 *
 * Exibe used + free + overhead, que somados
 * devem sempre resultar em memory_total().
 * Isso serve como verificação de consistência
 * do alocador durante a apresentação.
 */
static void print_heap_status(void) {
    uint64_t used = memory_used();
    uint64_t livre = memory_free();
    uint64_t overhead = memory_overhead();
    uint64_t total = memory_total();

    uart_print("  Heap total    : ");
    uart_print_uint(total);
    uart_print(" bytes\n");
    uart_print("  Heap usado    : ");
    uart_print_uint(used);
    uart_print(" bytes (area util ocupada)\n");
    uart_print("  Heap livre    : ");
    uart_print_uint(livre);
    uart_print(" bytes (area util livre)\n");
    uart_print("  Overhead      : ");
    uart_print_uint(overhead);
    uart_print(" bytes (headers dos blocos)\n");
    uart_print("  Verificacao   : ");
    uart_print_uint(used + livre + overhead);
    uart_print(" bytes (deve ser igual ao total)\n\n");
}

/*
 * Tasks de exemplo para o scheduler.
 */
void task1(void) {
    while (1) {
        uart_print("[Task 1] executando\n");
        yield();
    }
}

void task2(void) {
    while (1) {
        uart_print("[Task 2] executando\n");
        yield();
    }
}

/*
 * Função principal do kernel.
 *
 * Demonstra todos os cenários exigidos pelo enunciado:
 * 1. Múltiplas alocações
 * 2. Divisão de blocos (splitting)
 * 3. Liberação de memória (kfree)
 * 4. Reutilização de blocos liberados
 * 5. Coalescência de blocos adjacentes
 * 6. Estatísticas do heap
 * 7. Integração com tasks (criação, destruição, recriação)
 */
void kernel_main(void) {
    memory_init();

    uart_print("=== BasicMicrokernel - Free List Allocator ===\n\n");

    /* ── Estado inicial ── */
    uart_print("--- Estado inicial do heap ---\n");
    print_heap_status();

    /* ── Cenário 1: múltiplas alocações ── */
    uart_print("--- Cenario 1: multiplas alocacoes ---\n");

    void* p1 = kmalloc(256);
    void* p2 = kmalloc(512);
    void* p3 = kmalloc(128);

    uart_print("  Alocados: p1=256, p2=512, p3=128 bytes\n");
    uart_print("  Ordem no heap: [p1][p2][p3][livre...]\n");
    print_heap_status();

    /* ── Cenário 2: divisão de blocos (splitting) ── */
    uart_print("--- Cenario 2: divisao de blocos (splitting) ---\n");
    uart_print("  kmalloc(1024): encontra o bloco livre restante\n");
    uart_print("  e o divide em [p4=1024][novo bloco livre]\n");

    void* p4 = kmalloc(1024);

    print_heap_status();

    /* ── Cenário 3: liberação de memória ── */
    uart_print("--- Cenario 3: liberacao de memoria (kfree) ---\n");
    uart_print("  kfree(p2): libera o bloco do meio (512 bytes)\n");
    uart_print("  Heap: [p1 ocup][p2 LIVRE][p3 ocup][p4 ocup][livre]\n");

    kfree(p2);

    print_heap_status();

    /* ── Cenário 4: reutilização de bloco liberado ── */
    uart_print("--- Cenario 4: reutilizacao de bloco liberado ---\n");
    uart_print("  kmalloc(256): First Fit encontra p2 (512 bytes livres)\n");
    uart_print("  Divide: [p5=256 ocup][resto livre]\n");

    void* p5 = kmalloc(256);

    print_heap_status();

    /* ── Cenário 5: coalescência ── */
    uart_print("--- Cenario 5: coalescencia de blocos adjacentes ---\n");
    uart_print("  Liberando p1, p5 e p3 em sequencia...\n");
    uart_print("  Heap antes: [p1 ocup][p5 ocup][resto livre][p3 ocup][p4 ocup][livre]\n");

    kfree(p1);
    uart_print("  Apos kfree(p1): [p1 LIVRE][p5 ocup][resto livre][p3 ocup][p4 ocup][livre]\n");

    kfree(p5);
    uart_print("  Apos kfree(p5): [p1 LIVRE][p5 LIVRE][resto livre][p3 ocup][p4 ocup][livre]\n");
    uart_print("  Coalescencia une p1+p5+resto em um bloco so!\n");

    kfree(p3);
    uart_print("  Apos kfree(p3): blocos livres adjacentes unidos novamente\n");

    print_heap_status();

    /* ── Cenário 6 ── */
    uart_print("--- Cenario 6: integracao com tasks ---\n");

    kfree(p4);
    uart_print("  Heap apos liberar p4:\n");
    print_heap_status();

    xTaskCreate(task1, 2048, 1);
    uart_print("  Task 1 criada (stack = 2048 bytes via kmalloc)\n");
    print_heap_status();

    xTaskCreate(task2, 2048, 1);
    uart_print("  Task 2 criada (stack = 2048 bytes via kmalloc)\n");
    print_heap_status();

    xTaskDestroy(0);
    uart_print("  Task 1 removida (stack devolvida ao heap via kfree)\n");
    print_heap_status();

    xTaskCreate(task1, 2048, 1);
    uart_print("  Task 1 recriada: stack reutiliza o bloco liberado!\n");
    print_heap_status();

    uart_print("=== Iniciando scheduler ===\n\n");
    scheduler_start();

    while (1);
}