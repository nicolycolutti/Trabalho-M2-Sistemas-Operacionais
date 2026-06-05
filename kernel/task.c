#include "task.h"
#include "memory.h"

TCB  tasks[MAX_TASKS];
int  task_count = 0;

/*
 * Cria uma nova task.
 *
 * Reserva memória para a stack usando kmalloc()
 * e inicializa as informações básicas da task.
 */
void xTaskCreate(void (*task)(void), uint64_t stack_size, int priority)
{
    /* Verifica se ainda há espaço para novas tasks */
    if (task_count >= MAX_TASKS)
        return;

    TCB *t = &tasks[task_count++];

    t->entry      = task;
    t->priority   = priority;
    t->state      = 0;        /* READY */
    t->stack_size = stack_size;

    /* Reserva memória para a stack */
    t->stack = (uint8_t *)kmalloc(stack_size);

    /* Se não houver memória disponível, cancela a criação */
    if (!t->stack)
    {
        task_count--;
        return;
    }

    /* A stack cresce do endereço maior para o menor */
    uint64_t *sp = (uint64_t *)(t->stack + stack_size);

    /* Configura o contexto inicial da task */
    t->regs[0] = (uint64_t)task;   /* ra */
    t->regs[1] = (uint64_t)sp;     /* sp */
    t->sepc    = (uint64_t)task;
}

/*
 * Remove uma task.
 *
 * Libera a memória da stack e remove a task
 * da lista de tarefas.
 */
void xTaskDestroy(int index)
{
    /* Verifica se o índice é válido */
    if (index < 0 || index >= task_count)
        return;

    TCB *t = &tasks[index];

    /* Devolve a memória da stack para o heap */
    if (t->stack)
    {
        kfree(t->stack);
        t->stack = 0;
    }

    /* Move as tasks seguintes uma posição para trás */
    for (int i = index; i < task_count - 1; i++)
    {
        tasks[i].entry      = tasks[i + 1].entry;
        tasks[i].priority   = tasks[i + 1].priority;
        tasks[i].state      = tasks[i + 1].state;
        tasks[i].stack      = tasks[i + 1].stack;
        tasks[i].stack_size = tasks[i + 1].stack_size;
        tasks[i].sepc       = tasks[i + 1].sepc;
        for (int r = 0; r < 31; r++)
            tasks[i].regs[r] = tasks[i + 1].regs[r];
    }

    task_count--;
}