#include "../include/shell.h"
#include "../include/job.h"
#include "../include/senales.h"
#include <readline/readline.h>
#include <readline/history.h>

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

    //ignora las señales sigint (ctrl+c) y sigquit (ctrl+\) en la shell para evitar que finalice por accidente (r6)
    configurar_senales_shell();

    while (1) {
        //se muestra el prompt con el directorio actual
        char prompt_buffer[1024];
        char cwd[512];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            // \001 y \002 indican a readline que ignore los caracteres de color al medir la linea para evitar errores visuales al usar las flechas
            snprintf(prompt_buffer, sizeof(prompt_buffer), "\001\033[1;32m\002miShell:\001\033[1;34m\002%s\001\033[0m\002$ ", cwd);
        } else {
            snprintf(prompt_buffer, sizeof(prompt_buffer), "\001\033[1;32m\002miShell\001\033[0m\002$ ");
        }

        //se lee la entrada (Finaliza si el usuario presiona Ctrl+D / EOF)
        //le pasamos el prompt directamente a readline para capturar la entrada del usuario habilitando las flechitas
        char *input = readline(prompt_buffer);
        
        if (input == NULL) {
            printf("\n");
            break;
        }

        if (strlen(input) > 0) {
            //guarda el comando escrito en el historial interno de readline para poder recuperarlo con la felchita hacia arriba (bonus)
            add_history(input); 
            strncpy(linea, input, MAX_LINE - 1);
            linea[MAX_LINE - 1] = '\0';
        } else {
            free(input);
            continue;
        }
        free(input);

        //Tokenizar la entrada del usuario
        tokenizar(linea, args, &bandera_bg);

        // Si la línea estaba vacía (solo presionó Enter)
        if (args[0] == NULL) continue;

        // Salida básica por comando interno exit
        if (strcmp(args[0], "exit") == 0) break;

        //comando interno jobs
        if (strcmp(args[0], "jobs") == 0) {
            // Recorremos el arreglo de jobs
            for (int i = 0; i < MAX_JOBS; i++) {
                // Si el trabajo está marcado como activo, lo mostramos
                if (lista_jobs[i].activo) {
                    printf("[%d] Ejecutando %s\n", lista_jobs[i].id, lista_jobs[i].comando);
                }
            }
            // Saltamos el resto del ciclo (fork/exec) y volvemos a mostrar el prompt
            continue;
        }

        //Comando cd puedes cambiar de directorio
        if (strcmp(args[0], "cd") == 0){
            
            const char *dir = (args[1] != NULL) ? args[1] : getenv("HOME"); //Se utiliza en vez de un doble if
            char linea_sin_comillas[MAX_LINE];
            int p = 0;

            //Iteracion para limpiar las comillas simples, ya que hay problemas con chdir
            for(int i = 0; dir[i] != '\0';i++){
                if(dir[i] != '\''){
                    linea_sin_comillas[p] = dir[i];
                    p++;
                }               
            }
            linea_sin_comillas[p] = '\0';
            
            //si uno se cumple es porque hay un error, si no chdir se ejecuta con normalidad
            if(dir == NULL || chdir(linea_sin_comillas) < 0){
                perror("cd");
            }
            
            continue;
        }

        //Crear proceso e invocar el ejecutable
        ejecutar_comando(args, bandera_bg);
    }
    return 0;
}