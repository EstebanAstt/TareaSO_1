#include "../include/pipes.h"
#include "../include/redireccion.h"
#include "../include/job.h"
#include "../include/senales.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_PIPES 32

int crear_pipes(char **args, int bandera_bg) {
    int existe_pipe = 0;
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            existe_pipe = 1;
            break;
        }
    }
    if (!existe_pipe) return 0;

    char *comandos[MAX_PIPES][64];
    int num_comandos = 0;
    int indice_arg = 0;
    
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            comandos[num_comandos][indice_arg] = NULL;
            num_comandos++;
            indice_arg = 0;
        } else {
            comandos[num_comandos][indice_arg] = args[i];
            indice_arg++;
        }
    }
    comandos[num_comandos][indice_arg] = NULL;
    num_comandos++;

    int fd_in = STDIN_FILENO;
    int fd_pipe[2];
    pid_t pids[MAX_PIPES];

    for (int i = 0; i < num_comandos; i++) {
        if (i < num_comandos - 1) {
            if (pipe(fd_pipe) == -1) {
                perror("miShell: Error al crear el pipe");
                return -1;
            }
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("miShell: Error al crear el proceso hijo");
            return -1;
        } 
        
        if (pid == 0) {
            
            //restaurar señales por defecto en fg 
            if (!bandera_bg) {
                restaurar_senales_hijo_fg();
            }

            if (fd_in != STDIN_FILENO) {
                dup2(fd_in, STDIN_FILENO);
                close(fd_in);
            }

            if (i < num_comandos - 1) {
                close(fd_pipe[0]);
                dup2(fd_pipe[1], STDOUT_FILENO);
                close(fd_pipe[1]);
            }

            if (buscar_redirecciones(comandos[i]) < 0) {
                exit(EXIT_FAILURE);
            }

            execvp(comandos[i][0], comandos[i]);
            perror("miShell: Error al ejecutar el comando");
            exit(EXIT_FAILURE);
        }
            
        pids[i] = pid;
        if (fd_in != STDIN_FILENO) {
            close(fd_in);
        }
        if (i < num_comandos - 1) {
            close(fd_pipe[1]);
            fd_in = fd_pipe[0];
        }
    } 

    if (bandera_bg) {
        int job_id = -1;
        for (int j = 0; j < MAX_JOBS; j++) {
            if (!lista_jobs[j].activo) {
                lista_jobs[j].id = j + 1;
                lista_jobs[j].pid = pids[num_comandos - 1];
                strncpy(lista_jobs[j].comando, args[0], sizeof(lista_jobs[j].comando) - 1);
                lista_jobs[j].comando[sizeof(lista_jobs[j].comando) - 1] = '\0';
                lista_jobs[j].activo = 1;
                job_id = lista_jobs[j].id;
                break;
            }
        }

        if (job_id != -1) {
            printf("[%d] %d\n", job_id, pids[num_comandos - 1]);
        }
        return 1;
    } else {
        for (int i = 0; i < num_comandos; i++) {
            waitpid(pids[i], NULL, 0);
        }
        return 1;
    }
}