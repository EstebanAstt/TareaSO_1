// include/job.h
#ifndef JOB_H
#define JOB_H

#include <sys/types.h>

#define MAX_JOBS 100

typedef struct {
    int id;
    pid_t pid;
    char comando[256];
    int activo;
} Job;

// Declaracion extern (avisa que la variable está definida en algún .c, por ahora en main.c)
extern Job lista_jobs[MAX_JOBS];

#endif