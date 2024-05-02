#include <proto/memory.h>

int memory_send_read(int fd, int pid, uint32_t address, uint32_t offset, uint32_t size)
{
    t_packet *packet = packet_new(READ_MEM);
    packet_addUInt32(packet, pid);
    packet_addUInt32(packet, address);
    packet_addUInt32(packet, offset);
    packet_addUInt32(packet, size);
    int res = packet_send(packet, fd);
    packet_free(packet);
    return res;
}

void memory_decode_read(t_buffer *buffer, t_memory_read_msg *msg)
{
    msg->pid = packet_getUInt32(buffer);
    msg->address = packet_getUInt32(buffer);
    msg->offset = packet_getUInt32(buffer);
    msg->size = packet_getUInt32(buffer);
}

void memory_destroy_read(t_memory_read_msg *msg)
{
    free(msg);
}

int memory_send_read_ok(int fd, void *value, uint32_t size)
{
    t_packet *packet = packet_new(READ_MEM_OK);
    packet_add(packet, value, size);
    int res = packet_send(packet, fd);
    packet_free(packet);
    return res;
}

void memory_decode_read_ok(t_buffer *buffer, t_memory_read_ok_msg *msg, uint32_t size)
{
    msg->value = malloc(size);
    packet_get(buffer, msg->value, size);
}

void memory_destroy_read_ok(t_memory_read_ok_msg *msg)
{
    free(msg->value);
    free(msg);
}

int memory_send_write(int fd, int pid, uint32_t address, uint32_t offset, uint32_t size, void *value)
{
    t_packet *packet = packet_new(WRITE_MEM);
    packet_addUInt32(packet, pid);
    packet_addUInt32(packet, address);
    packet_addUInt32(packet, offset);
    packet_addUInt32(packet, size);
    packet_add(packet, value, size);
    int res = packet_send(packet, fd);
    packet_free(packet);
    return res;
}

void memory_decode_write(t_buffer *buffer, t_memory_write_msg *msg)
{
    msg->pid = packet_getUInt32(buffer);
    msg->address = packet_getUInt32(buffer);
    msg->offset = packet_getUInt32(buffer);
    msg->size = packet_getUInt32(buffer);
    msg->value = malloc(msg->size);
    packet_get(buffer, msg->value, msg->size);
}

void memory_destroy_write(t_memory_write_msg *msg)
{
    free(msg->value);
    free(msg);
}

int memory_send_write_ok(int fd)
{
    t_packet *packet = packet_new(WRITE_MEM_OK);
    int res = packet_send(packet, fd);
    packet_free(packet);
    return res;
}
