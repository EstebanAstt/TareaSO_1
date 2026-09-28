#include "../include/shell.h"
#include "../include/redireccion.h"
#include "../include/pipes.h"
#include "../include/job.h"
#include "../include/senales.h"
#include <unistd.h>  
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Job lista_jobs[MAX_JOBS];

void mostrar_prompt(void) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf(COLOR_VERDE "miShell:" COLOR_AZUL "%s" COLOR_RESET "$ ", cwd);
    } else {
        printf(COLOR_VERDE "miShell" COLOR_RESET "$ ");
    }
    fflush(stdout);
}

void ejecutar_cd(char **args){
    const char *dir = (args[1] != NULL) ? args[1] : getenv("HOME"); 
    char linea_sin_comillas[MAX_LINE];
    int p = 0;

    for(int i = 0; dir[i] != '\0'; i++){
        if(dir[i] != '\''){
            linea_sin_comillas[p] = dir[i];
            p++;
        }
    }
    linea_sin_comillas[p] = '\0';

    if(chdir(linea_sin_comillas) < 0){
        perror("cd");
    }
}

void ejecutar_export(char **args) {
    if (args[1] == NULL) {
        printf("Uso: export VAR=valor\n");
        return;
    }

    char *signo_igual = strchr(args[1], '='); 
    
    if (signo_igual != NULL) {
        *signo_igual = '\0'; 
        char *nombre = args[1];          
        char *valor = signo_igual + 1; 
        
        setenv(nombre, valor, 1); 
    } else {
        printf("Error: Formato incorrecto. Uso: export VAR=valor\n");
    }
}

int manejador_entradas(char **args){
    if(args == NULL || args[0] == NULL){
        return 0;
    }

    if(strcmp(args[0],"exit") == 0){
        int codigo = (args[1] != NULL) ? atoi(args[1]): 0;
        exit(codigo);
    }

    if(strcmp(args[0],"jobs") == 0){
        ejecutar_jobs();
        return 1;
    }

    if(strcmp(args[0],"cd") == 0){
        ejecutar_cd(args);
        return 1;
    }

    if(strcmp(args[0],"pmon") == 0){
        ejecutar_pmon(args);
        return 1;
    }
    
    if(strcmp(args[0],"export") == 0){
        ejecutar_export(args);
        return 1;
    }

    return 0;
}

void ejecutar_comando(char **args, int bandera_bg) {
    if(crear_pipes(args, bandera_bg)){
        return; 
    }

    pid_t pid = fork();
    if(pid < 0) {
        perror("Error en fork()");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) {
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
                    lista_jobs[j].prim_lect = 1;
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