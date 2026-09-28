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

    //Calculo de uso de cpu
    unsigned long ant_cpu_time; //utime + stime de la lectura anterior
    double prev_timestamp; //Tiempo de la lectura anterior
    int prim_lect; //Si es la primera lectura 1, si no es 0
} Job;

// Declaracion extern (avisa que la variable está definida en algún .c, por ahora en main.c)
extern Job lista_jobs[MAX_JOBS];
void ejecutar_jobs(void);

#endif