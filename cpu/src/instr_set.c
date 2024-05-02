#include <instr_set.h>

int current_exec_process_has_finished = 0;

void set(char **args, t_log *logger);
void sum(char **args, t_log *logger);
void sub(char **args, t_log *logger);
void jnz(char **args, t_log *logger);
void io_gen_sleep(char **args, t_log *logger);
void mov_in(char **args, t_log *logger);
void mov_out(char **args, t_log *logger);
void resize(char **args, t_log *logger);
void copy_string(char **args, t_log *logger);
void wait_instr(char **args, t_log *logger);
void signal_instr(char **args, t_log *logger); // cambio el nombre por confilcto con el signal para el ctrl+C
void io_stdin_read(char **args, t_log *logger);
void io_stdout_write(char **args, t_log *logger);
void io_fs_create(char **args, t_log *logger);
void io_fs_delete(char **args, t_log *logger);
void io_fs_truncate(char **args, t_log *logger);
void io_fs_write(char **args, t_log *logger);
void io_fs_read(char **args, t_log *logger);
// para no tener conflitcto
void instruction_exit(char **args, t_log *logger);

static void send_io_gen_sleep(char *interface_name, int work_units);
enum StdType
{
    IN,
    OUT
};
static void send_io_std(enum StdType type, char *interface_name, char *physical_address, int offset, uint32_t size);
static void send_wait_resource(char *resource_name);
static void send_signal_resource(char *resource_name);

t_instruction INSTRUCTION_SET[] = {{"SET", set}, {"SUM", sum}, {"SUB", sub}, {"JNZ", jnz}, {"IO_GEN_SLEEP", io_gen_sleep}, {"MOV_IN", mov_in}, {"MOV_OUT", mov_out}, {"RESIZE", resize}, {"COPY_STRING", copy_string}, {"WAIT", wait_instr}, {"SIGNAL", signal_instr}, {"IO_STDIN_READ", io_stdin_read}, {"IO_STDOUT_WRITE", io_stdout_write}, {"IO_FS_CREATE", io_fs_create}, {"IO_FS_DELETE", io_fs_delete}, {"IO_FS_TRUNCATE", io_fs_truncate}, {"IO_FS_WRITE", io_fs_write}, {"IO_FS_READ", io_fs_read}, {"EXIT", instruction_exit}, {NULL, NULL}};

// CON ESTA NOMENCLATURA CADA INSTRUCCION DEBE DESERIALIZAR SUS PARAMETROSs

t_instruction *instruction_get_by_name(char *name)
{
    for (uint8_t i = 0; INSTRUCTION_SET[i].name != NULL; i++)
    {
        if (!strcmp(INSTRUCTION_SET[i].name, name))
            return &INSTRUCTION_SET[i];
    }
    return NULL;
}

void set(char **args, t_log *logger)
{
    t_register *reg = register_get_by_name(args[0]);

    if (sizeof(uint8_t) == reg->size)
    {
        uint8_t value = (uint8_t)atoi(args[1]);
        memcpy(reg->address, &value, reg->size);
        return;
    }

    uint32_t value = (uint32_t)atoi(args[1]);
    memcpy(reg->address, &value, reg->size);
}

void sum(char **args, t_log *logger)
{
    t_register *dest = register_get_by_name(args[0]);
    t_register *src = register_get_by_name(args[1]);

    if (sizeof(uint8_t) == dest->size)
    {
        uint8_t *src_value = (uint8_t *)src->address;
        uint8_t *dest_value = (uint8_t *)dest->address;

        *dest_value += *src_value;
        return;
    }

    uint32_t *src_value = (uint32_t *)src->address;
    uint32_t *dest_value = (uint32_t *)dest->address;

    *dest_value += *src_value;
}

void sub(char **args, t_log *logger)
{
    t_register *dest = register_get_by_name(args[0]);
    t_register *src = register_get_by_name(args[1]);

    if (sizeof(uint8_t) == dest->size)
    {
        uint8_t *src_value = (uint8_t *)src->address;
        uint8_t *dest_value = (uint8_t *)dest->address;

        *dest_value -= *src_value;
        return;
    }

    uint32_t *src_value = (uint32_t *)src->address;
    uint32_t *dest_value = (uint32_t *)dest->address;

    *dest_value -= *src_value;
}

void jnz(char **args, t_log *logger)
{
    uint32_t *pc = &context.registers.pc;
    t_register *reg = register_get_by_name(args[0]);
    if (sizeof(uint8_t) == reg->size)
    {
        uint8_t *value = (uint8_t *)reg->address;
        *pc = *value ? atoi(args[1]) : *pc;
        return;
    }

    uint32_t *value = (uint32_t *)reg->address;
    *pc = *value ? atoi(args[1]) : *pc;
}

void mov_in(char **args, t_log *logger)
{
    // t_register* data = register_get_by_name(args[0]);
    // t_register* dir = register_get_by_name(args[1]);

    // luego de la operacion
    // log_info(logger,"PID: %d - Accion: LEER - Direccion Fisica: %d - Valor: %d",context.pid)
}
void mov_out(char **args, t_log *logger)
{
    // t_register* dir = register_get_by_name(args[0]);
    // t_register* value = register_get_by_name(args[1]);
    // luego de la operacion
    // log_info(logger,"PID: %d - Accion: ESCRIBIR - Direccion Fisica: %d - Valor: %d",context.pid)
}
void resize(char **args, t_log *logger)
{
}
void copy_string(char **args, t_log *logger)
{
}
void wait_instr(char **args, t_log *logger)
{
    clear_interrupt();
    char *resource_name = args[0];
    send_wait_resource(resource_name);
    wait_for_context(&context);
    log_debug(logger, "me llego: pid: %d, quantum: %d", context.pid, context.quantum);
}
void signal_instr(char **args, t_log *logger)
{
    clear_interrupt();
    char *resource_name = args[0];
    send_signal_resource(resource_name);
    wait_for_context(&context);
    log_debug(logger, "me llego: pid: %d, quantum: %d", context.pid, context.quantum);
}

void io_gen_sleep(char **args, t_log *logger)
{
    clear_interrupt();
    char *interface_name = args[0];
    uint32_t work_units = atoi(args[1]);
    send_io_gen_sleep(interface_name, work_units);
    wait_for_context(&context);
    log_debug(logger, "me llego: pid: %d, quantum: %d", context.pid, context.quantum);
}

void io_stdin_read(char **args, t_log *logger)
{
    clear_interrupt();
    char *interface_name = args[0];
    char *virtual_address = register_get_by_name(args[1])->address;
    uint32_t *size_dir = (uint32_t *)register_get_by_name(args[2])->address;

    t_physical_address *physical_mem_dir = translate_address(virtual_address);

    send_io_std(IN, interface_name, physical_mem_dir->address, physical_mem_dir->offset, *size_dir);
    wait_for_context(&context);
    log_debug(logger, "me llego: pid: %d, quantum: %d", context.pid, context.quantum);
}

void io_stdout_write(char **args, t_log *logger)
{
    clear_interrupt();
    char *interface_name = args[0];
    char *virtual_address = register_get_by_name(args[1])->address;
    uint32_t *size_dir = (uint32_t *)register_get_by_name(args[2])->address;

    t_physical_address *physical_mem_dir = translate_address(virtual_address);

    send_io_std(OUT, interface_name, physical_mem_dir->address, physical_mem_dir->offset, *size_dir);

    wait_for_context(&context);
    log_debug(logger, "me llego: pid: %d, quantum: %d", context.pid, context.quantum);
}

void io_fs_create(char **args, t_log *logger)
{
}
void io_fs_delete(char **args, t_log *logger)
{
}
void io_fs_truncate(char **args, t_log *logger)
{
}

void io_fs_write(char **args, t_log *logger)
{
}
void io_fs_read(char **args, t_log *logger)
{
}
// para no tener conflitcto
void instruction_exit(char **args, t_log *logger)
{
    clear_interrupt();
    send_dispatch_reason(END_PROCESS, &context);
    current_exec_process_has_finished = 1;
}

// WAIT and SIGNAL protocol

static void send_wait_resource(char *resource_name)
{
    t_packet *packet = packet_new(WAIT);
    packet_add_context(packet, &context);
    packet_addString(packet, resource_name);
    packet_send(packet, cli_dispatch_fd);
    packet_free(packet);
}

static void send_signal_resource(char *resource_name)
{
    t_packet *packet = packet_new(SIGNAL);
    packet_add_context(packet, &context);
    packet_addString(packet, resource_name);
    packet_send(packet, cli_dispatch_fd);
    packet_free(packet);
}

// IO protocol
static void send_io_gen_sleep(char *interface_name, int work_units)
{
    t_packet *packet = packet_new(IO_GEN_SLEEP);
    packet_add_context(packet, &context);
    packet_addString(packet, interface_name);
    packet_addUInt32(packet, work_units);
    packet_send(packet, cli_dispatch_fd);
    packet_free(packet);
}

static void send_io_std(enum StdType type, char *interface_name, char *physical_address, int offset, uint32_t size)
{
    t_packet *packet = packet_new(type == IN ? IO_STDIN_READ : IO_STDOUT_WRITE);
    packet_add_context(packet, &context);
    packet_addString(packet, interface_name);
    packet_addString(packet, physical_address);
    packet_addUInt32(packet, offset);
    packet_addUInt32(packet, size);
    packet_send(packet, cli_dispatch_fd);
    packet_free(packet);
}
