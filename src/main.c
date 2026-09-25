#include "../include/shell.h"
#include "../include/job.h"
#include "../include/senales.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Job lista_jobs[MAX_JOBS];

int main(void) {
    char linea[MAX_LINE];
    char *args[MAX_ARGS];
    int bandera_bg = 0;

    struct sigaction sa;
    sa.sa_handler = manejador_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("Error instalando manejador SIGCHLD");
        exit(EXIT_FAILURE);
    }

    //ignora el  controlc y y control\ en la shell principal
    configurar_senales_shell();

    while (1) {
        // Generar prompt compatible con readline sin tocar la funcion de los compañeros
        char prompt_buffer[1024];
        char cwd[512];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            // \001 y \002 indican a readline que ignore los caracteres de color al medir la linea
            snprintf(prompt_buffer, sizeof(prompt_buffer), "\001\033[1;32m\002miShell:\001\033[1;34m\002%s\001\033[0m\002$ ", cwd);
        } else {
            snprintf(prompt_buffer, sizeof(prompt_buffer), "\001\033[1;32m\002miShell\001\033[0m\002$ ");
        }

        // Le pasamos el prompt directamente a readline
        char *input = readline(prompt_buffer);
        
        if (input == NULL) {
            printf("\n");
            break;
        }

        if (strlen(input) > 0) {
            add_history(input); 
            strncpy(linea, input, MAX_LINE - 1);
            linea[MAX_LINE - 1] = '\0';
        } else {
            free(input);
            continue;
        }
        free(input);

        tokenizar(linea, args, &bandera_bg);

        if (args[0] == NULL) continue;
        if (strcmp(args[0], "exit") == 0) break;

        if (strcmp(args[0], "jobs") == 0) {
            for (int i = 0; i < MAX_JOBS; i++) {
                if (lista_jobs[i].activo) {
                    printf("[%d] Ejecutando %s\n", lista_jobs[i].id, lista_jobs[i].comando);
                }
            }
            continue;
        }

        if (strcmp(args[0], "cd") == 0) {
            const char *dir = (args[1] != NULL) ? args[1] : getenv("HOME");
            char linea_sin_comillas[MAX_LINE];
            int p = 0;

            for (int i = 0; dir[i] != '\0'; i++) {
                if (dir[i] != '\'') {
                    linea_sin_comillas[p] = dir[i];
                    p++;
                }               
            }
            linea_sin_comillas[p] = '\0';
            
            if (dir == NULL || chdir(linea_sin_comillas) < 0) {
                perror("cd");
            }
            continue;
        }

        ejecutar_comando(args, bandera_bg);
    }
    return 0;
}