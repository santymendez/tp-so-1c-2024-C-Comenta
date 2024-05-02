#include <scheduler.h>

static void set_scheduling_algorithm(void);

static int paused_threads = 0;
static pthread_mutex_t MUTEX_PAUSE;
static bool scheduler_paused = false;

t_scheduler scheduler;

static void init_scheduler_sems(void)
{
    sem_init(&scheduler.sem_ready, 0, 0);
    sem_init(&scheduler.sem_ready_plus, 0, 0);
    sem_init(&scheduler.sem_new, 0, 0);

    sem_init(&scheduler.sem_paused, 0, 0);
    pthread_mutex_init(&MUTEX_PAUSE, NULL);
    // me reservo las dos primeras para el corto y largo plazo
}

static void destroy_scheduler_sems(void)
{
    sem_destroy(&scheduler.sem_ready);
    sem_destroy(&scheduler.sem_new);
    sem_destroy(&scheduler.sem_ready_plus);
    sem_destroy(&scheduler.sem_paused);
    pthread_mutex_destroy(&MUTEX_PAUSE);
}

void handle_pause(void)
{
    bool res;
    pthread_mutex_lock(&MUTEX_PAUSE);
    if ((res = scheduler_paused))
    {
        paused_threads++;
    }
    pthread_mutex_unlock(&MUTEX_PAUSE);
    if(res) 
        sem_wait(&scheduler.sem_paused);
}

void pause_threads(void){
    pthread_mutex_lock(&MUTEX_PAUSE);
    scheduler_paused = true;
    pthread_mutex_unlock(&MUTEX_PAUSE);
}

void resume_threads(void)
{
    pthread_mutex_lock(&MUTEX_PAUSE);
    if (scheduler_paused)
    {
        scheduler_paused = false;
        while (paused_threads > 0)
        {
            sem_post(&scheduler.sem_paused);
            paused_threads--;
        }
    }
    pthread_mutex_unlock(&MUTEX_PAUSE);
}

void init_scheduler(void)
{
    init_scheduler_sems();
    set_scheduling_algorithm();
}

void destroy_scheduler(void)
{
    destroy_scheduler_sems();
}

static void set_scheduling_algorithm(void)
{
    if (!strcmp(cfg_kernel->algoritmo_planificacion, "FIFO"))
    {
        scheduler.ready_to_exec = ready_to_exec_fifo;
        scheduler.dispatch = dispatch_fifo;
        scheduler.block_to_ready = block_to_ready_fifo;
        scheduler.move_pcb_to_blocked = move_pcb_to_blocked_fifo;
        return;
    }
    if (!strcmp(cfg_kernel->algoritmo_planificacion, "RR"))
    {
        scheduler.ready_to_exec = ready_to_exec_rr;
        scheduler.dispatch = dispatch_rr;
        scheduler.block_to_ready = block_to_ready_rr;
        scheduler.exec_to_ready = exec_to_ready_rr;
        scheduler.move_pcb_to_blocked = move_pcb_to_blocked_rr;
        return;
    }
    scheduler.ready_to_exec = ready_to_exec_vrr;
    scheduler.dispatch =  dispatch_vrr;
    scheduler.exec_to_ready = exec_to_ready_vrr;
    scheduler.block_to_ready = block_to_ready_vrr;
    scheduler.move_pcb_to_blocked = move_pcb_to_blocked_vrr;
}

void handle_short_term_scheduler(void *args_logger)
{
    t_log *logger = (t_log *)args_logger;
    set_scheduling_algorithm();
    log_debug(logger,"el algoritmo seleccionado fue: %s",cfg_kernel->algoritmo_planificacion);

    while (1)
    {
        handle_pause();
        t_pcb *pcb = scheduler.ready_to_exec();
        log_info(logger, "PID: %d - Estado Anterior: READY - Estado Actual: EXEC", pcb->context->pid); // solo lo saco, la referencia creo que ya la tengo
        scheduler.dispatch(pcb, logger);
        queue_sync_pop(exec_queue);
        // para no perder memoria por ahora pero no va
    }
}

// int move_pcb_to_blocked(t_pcb *pcb, char *resource_name, t_log *logger)
// {
//     if(blocked_queue_push(resource_name, pcb) == -1)
//         return -1;
//     pcb->state = BLOCKED;
//     log_info(logger, "PID: %d - Estado Anterior: EXEC - Estado Actual: BLOCKED", pcb->context->pid);
//     return 0;
// }

void move_pcb_to_exit(t_pcb *pcb, t_log *logger)
{
    queue_sync_push(exit_queue, pcb);
    log_info(logger, "PID: %d - Estado Anterior: %s - Estado Actual: EXIT", pcb->context->pid, pcb_state_to_string(pcb));
    pcb->state = EXIT;
    send_end_process(pcb->context->pid);
}

void handle_long_term_scheduler(void *args_logger)
{

    return;
    handle_pause();
    // while (1)
    //{
    //  while ( haya procesos en new)
    //  {
    //       lo paso a ready
    //       pcb->state = READY;
    //  }

    // while ( haya procesos en exit)
    // {
    //      libero los recursos
    // }
    //}
}