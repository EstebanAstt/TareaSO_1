#include "../include/pipes.h"
#include "../include/redireccion.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "job.h"

#define MAX_PIPES 32 // máximo número de pipes que se pueden crear

int crear_pipes(char **args, int bandera_bg) {
    //revisar si hay al menos un "|"
    int existe_pipe = 0;
    for (int i = 0; args[i] != NULL; i++){
        if (strcmp(args[i], "|") == 0) {
            existe_pipe = 1;
            break;
        }
    }
    if (!existe_pipe) {
        return 0; // no hay pipes, se ejecuta la shell normal
    }

    // contar el número de pipes, separando los args
    char *comandos[MAX_PIPES][64]; // array de comandos separados por pipes
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
    num_comandos++; // se cuenta el ultimo comando

    //hacer comandos encadenados con pipes
    int fd_in = STDIN_FILENO; // entrada inicial es la entrada estándar
    int fd_pipe[2];
    pid_t pids[MAX_PIPES];

    for (int i = 0; i < num_comandos; i++) {
        if (i < num_comandos - 1) { // si no es el último comando, crear un pipe
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
        
        if (pid == 0) { // proceso hijo
            if (fd_in != STDIN_FILENO) {
                dup2(fd_in, STDIN_FILENO); // redireccionar la entrada del pipe anterior
                close(fd_in);
            }

            if (i < num_comandos - 1) { // si no es el último comando, redireccionar la salida al pipe
                close(fd_pipe[0]);
                dup2(fd_pipe[1], STDOUT_FILENO);
                close(fd_pipe[1]);
                
            }

            // buscar redirecciones en el comando actual
            if (buscar_redirecciones(comandos[i]) < 0) {
                exit(EXIT_FAILURE); // error en redirección
            }

            execvp(comandos[i][0], comandos[i]);
            perror("miShell: Error al ejecutar el comando");
            exit(EXIT_FAILURE);
            }
            
            pids[i] = pid; // guardar el PID del proceso hijo
            if (fd_in != STDIN_FILENO) {
                close(fd_in); // cerrar la entrada del pipe anterior
            }
            if (i < num_comandos - 1) {
                close(fd_pipe[1]); // cerrar la salida del pipe actual
                fd_in = fd_pipe[0]; // la entrada del siguiente comando será la salida del pipe actual
            }
        } 
    /* se añade este condicional para que se puedan añadir procesos en 2do plano con pipes, ahora no termina siempre todos
     * los procesos hijos */
    if (bandera_bg) {
        // --- EJECUCIÓN EN BACKGROUND (R5) ---
        // Guardamos el trabajo en lista_jobs usando el PID del último comando de la tubería
        int job_id = -1;
        for (int j = 0; j < MAX_JOBS; j++) {
            if (!lista_jobs[j].activo) {
                lista_jobs[j].id = j + 1;
                lista_jobs[j].pid = pids[num_comandos - 1]; // PID del último proceso hijo

                // Guardamos la representación del comando
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

        // no se hace waitpid aqui para retornar inmediatamente y no bloquear la shell.
        return 1;

    } else {
        for (int i = 0; i < num_comandos; i++) {
            waitpid(pids[i], NULL, 0); // esperar a que terminen todos los procesos hijos
        }
        return 1;
    }
    return 1; // exito
}