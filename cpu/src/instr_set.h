#ifndef INSTR_SET_CPU_H
#define INSTR_SET_CPU_H

#include<string.h>
#include<stdlib.h>
#include<register.h>

typedef struct {
    char* name;
    void (*instr)(char** args);
} t_instruction;

extern t_instruction INSTRUCTION_SET[];
t_instruction *instruction_get_by_name(char *name);

#endif