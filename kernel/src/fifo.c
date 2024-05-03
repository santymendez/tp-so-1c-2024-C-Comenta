#include <fifo.h>

t_pcb *ready_to_exec_fifo(void)
{
    sem_wait(&scheduler.sem_ready);
    t_pcb *pcb = queue_sync_pop(ready_queue);
    queue_sync_push(exec_queue, pcb);
    pcb->state = EXEC;
    send_context_to_cpu(pcb->context);
    return pcb;
}

void dispatch_fifo(t_pcb *pcb, t_log *logger)
{
    if (wait_for_dispatch_reason(pcb, logger) == -1)
    {
        log_error(logger, "error waiting for cpu context");
    }
}

void block_to_ready_fifo(char *resource, t_log *logger)
{
    //wait sem cola bloqueado
    t_pcb *pcb = blocked_queue_pop(resource);
    if(!pcb)
        log_error(logger,"blocked queue not found");
    pcb->state = READY;
    log_info(logger, "PID: %d - Estado Anterior: BLOCKED - Estado Actual: READY", pcb->context->pid);
    queue_sync_push(ready_queue, pcb);
    sem_post(&scheduler.sem_ready);
}

int move_pcb_to_blocked_fifo(t_pcb *pcb, char *resource_name, t_log *logger)
{
    if(blocked_queue_push(resource_name, pcb) == -1)
        return -1;
    pcb->state = BLOCKED;
    log_info(logger, "PID: %d - Estado Anterior: EXEC - Estado Actual: BLOCKED", pcb->context->pid);
    return 0;
}