#include <interface.h>

t_dictionary *interface_dictionary;

static void interface_destroyer(void *_interface);

void interface_init(void)
{
    interface_dictionary = dictionary_create();
}

void interface_add(t_interface *interface)
{
    dictionary_put(interface_dictionary, interface->name, interface);
}

t_interface *interface_get(char *name)
{
    return dictionary_get(interface_dictionary, name);
}

t_interface *interface_get_by_fd(int fd)
{
    bool closure(void *elem)
    {
        t_interface *interface = (t_interface *)elem;
        return interface->fd == fd;
    }
    t_list *list = dictionary_elements(interface_dictionary);
    t_interface *tmp = list_find(list, closure);
    list_destroy(list);
    return tmp;
}

void destroy_interface_dictionary(void)
{
    void close_clients(char *key, void *elem)
    {
        t_interface *interface = (t_interface *)elem;
        socket_freeConn(interface->fd);
    }
    dictionary_iterator(interface_dictionary, close_clients);
    dictionary_destroy_and_destroy_elements(interface_dictionary, interface_destroyer);
}

int interface_is_connected(t_interface *interface)
{
    return socket_isConnected(interface->fd);
}

int interface_can_run_instruction(t_interface *interface, uint8_t instruction_to_run)
{
    switch (instruction_to_run)
    {
    case IO_GEN_SLEEP:
        return strcmp(interface->type, "GENERICA") == 0;
    case IO_STDIN_READ:
        return strcmp(interface->type, "STDIN") == 0;
    case IO_STDOUT_WRITE:
        return strcmp(interface->type, "STDOUT") == 0;
    // the rest of instructions correspond to the file system
    default:
        return strcmp(interface->type, "DIALFS") == 0;
    }
}

t_interface *interface_validate(char *name, uint8_t instruction_to_run)
{
    t_interface *interface = interface_get(name);

    // check if exists
    if (interface == NULL)
        return NULL;
    // comento por ahora pero por si las dudas dejo
    // if (!interface_is_connected(interface))
    // {
    //     interface_destroy(interface);
    //     socket_freeConn(interface->fd);
    //     return NULL;
    // }

    if (!interface_can_run_instruction(interface, instruction_to_run))
        return NULL;

    return interface;
}

void interface_destroy(t_interface *interface)
{
    // aca podriamos mandar a exit los procesos de la blocked queue de la interfaz
    dictionary_remove_and_destroy(interface_dictionary, interface->name, interface_destroyer);
}

static void interface_destroyer(void *_interface)
{
    t_interface *interface = (t_interface *)_interface;
    free(interface->name);
    free(interface->type);
    free(interface);
}

/**
 * should be run at the beginning of every dispatch request that correspond to interfaces
 * handles:
 *  - validation
 *  - process scheduling
 *
 * @returns NULL on error, otherwise the corresponding `t_interface`
 */
t_interface *interface_middleware(t_buffer *buffer, uint8_t instruction_to_run, t_pcb *pcb, t_log *logger)
{
    char *interface_name = packet_getString(buffer);
    t_interface *interface = interface_validate(interface_name, instruction_to_run);
    if (!interface)
    {
        log_info(logger, "Finaliza el proceso %d- Motivo: Error de interfaz %s no conectada", pcb->context->pid, interface_name);
        // nunca pase por bloqueado asi que no deberia explotar
        move_pcb_to_exit(pcb, logger);
        return NULL;
    }
    if (scheduler.move_pcb_to_blocked(pcb, interface->name, logger) == -1)
    {
        log_error(logger, "Could not find blocked queue for %s", interface_name);
        move_pcb_to_exit(pcb, logger);
        return NULL;
    }
    log_info(logger, "PID: %d - Bloqueado por: %s", pcb->context->pid, interface_name);
    free(interface_name);
    return interface;
}