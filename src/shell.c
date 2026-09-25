#include "../include/shell.h"
#include "../include/redireccion.h"
#include "../include/pipes.h"
#include "../include/job.h"
#include "../include/senales.h"
#include <sys/wait.h>

void mostrar_prompt(void) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf(COLOR_VERDE "miShell:" COLOR_AZUL "%s" COLOR_RESET "$ ", cwd);
    } else {
        printf(COLOR_VERDE "miShell" COLOR_RESET "$ ");
    }
    fflush(stdout);
}

void tokenizar(char *linea, char **args, int *bandera_bg) {
    *bandera_bg = 0;
    linea[strcspn(linea, "\n")] = '\0';

    char *inicio_palabra = linea;
    int saltar_espacio = 0;
    int cant_argumentos = 0;

    for (int i = 0; linea[i] != '\0'; i++) {
        if (linea[i] == '\'') saltar_espacio = !saltar_espacio;

        if (linea[i] == ' ' && saltar_espacio == 0) {
            linea[i] = '\0';

            if (*inicio_palabra != '\0') {
                args[cant_argumentos] = inicio_palabra;
                cant_argumentos++;
            }
            inicio_palabra = &linea[i + 1];
        }
    }

    if (*inicio_palabra != '\0') {
        args[cant_argumentos] = inicio_palabra;
        cant_argumentos++;
    }
    args[cant_argumentos] = NULL;

    if (cant_argumentos > 0 && strcmp(args[cant_argumentos - 1], "&") == 0) {
        *bandera_bg = 1;
        args[cant_argumentos - 1] = NULL;
    }
}

void ejecutar_comando(char **args, int bandera_bg) {

    if (crear_pipes(args, bandera_bg)) {
        return;
    }
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("Error en fork()");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        
        //modificado restaurara señales por defecto en fg
        if (!bandera_bg) {
            restaurar_senales_hijo_fg();
        }

        if (buscar_redirecciones(args) < 0) {
            exit(EXIT_FAILURE);
        }

        if (execvp(args[0], args) < 0) {
            perror("Comando no encontrado");
            exit(EXIT_FAILURE);
        }
    } else {
        if (bandera_bg) {
            int job_id = -1;
            for (int j = 0; j < MAX_JOBS; j++) {
                if (!lista_jobs[j].activo) {
                    lista_jobs[j].id = j + 1;
                    lista_jobs[j].pid = pid;
                    strncpy(lista_jobs[j].comando, args[0], sizeof(lista_jobs[j].comando) - 1);
                    lista_jobs[j].comando[sizeof(lista_jobs[j].comando) - 1] = '\0';
                    lista_jobs[j].activo = 1;
                    job_id = lista_jobs[j].id;
                    break;
                }
            }

            if (job_id != -1) {
                printf("[%d] %d\n", job_id, pid);
            } else {
                printf("Error: Límite máximo de jobs alcanzado.\n");
            }
        } else {
            int status;
            waitpid(pid, &status, 0);
        }
    }
}

void manejador_sigchld(int sig) {
    (void)sig;
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < MAX_JOBS; i++) {
            if (lista_jobs[i].activo && lista_jobs[i].pid == pid) {
                lista_jobs[i].activo = 0;

                if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_FAILURE) {
                    printf("\n[%d]+ Error de ejecucion %s\n", lista_jobs[i].id, lista_jobs[i].comando);
                } else {
                    printf("\n[%d]+ Done %s\n", lista_jobs[i].id, lista_jobs[i].comando);
                }

                mostrar_prompt();
                fflush(stdout);
                break;
            }
        }
    }
}