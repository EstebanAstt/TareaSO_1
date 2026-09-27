#include "../include/shell.h"
#include "../include/redireccion.h"
#include "../include/pipes.h"
#include "../include/job.h"
#include "../include/senales.h"
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

void mostrar_prompt(void) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf(COLOR_VERDE "miShell:" COLOR_AZUL "%s" COLOR_RESET "$ ", cwd);
    } else {
        printf(COLOR_VERDE "miShell" COLOR_RESET "$ ");
    }
    fflush(stdout);
}

//Funcion que separa la cadena en tokens individuales (por cada espacio, excepto si esta entre comillas simples '')
void tokenizar(char *linea, char **args, int *bandera_bg) {
    *bandera_bg = 0; // Inicializamos la bandera en 0 por defecto

    // se elimina el salto de línea al final de fgets
    linea[strcspn(linea, "\n")] = '\0';

    char *inicio_palabra = linea;
    int saltar_espacio = 0;
    int cant_argumentos = 0;

    // Tokenizacion
    for (int i = 0; linea[i] != '\0'; i++) {
        // Verificamos si estamos dentro de comillas utilizando saltar_espacio
        if (linea[i] == '\'' || linea[i] == '\"') saltar_espacio = !saltar_espacio;

        // Encontramos un espacio y verificamos que no estemos dentro de comillas
        if (linea[i] == ' ' && saltar_espacio == 0) {
            linea[i] = '\0';

            if (*inicio_palabra != '\0') {
                args[cant_argumentos] = inicio_palabra;
                cant_argumentos++;
            }
            // Para que empiece luego del '\0'
            inicio_palabra = &linea[i + 1];
        }
    }

    // Agrega la ultima palabra si no es '\0'
    if (*inicio_palabra != '\0') {
        args[cant_argumentos] = inicio_palabra;
        cant_argumentos++;
    }
    args[cant_argumentos] = NULL; // El último puntero en NULL para execvp

    // Recorremos todos los argumentos guardados para quitarles las comillas de los extremos (en caso de que se utilizaron comillas
    for (int j = 0; j < cant_argumentos; j++) {
        int len = strlen(args[j]);
        // Si el argumento tiene al menos 2 caracteres y está envuelto en comillas simples o dobles
        if (len >= 2 && ((args[j][0] == '\"' && args[j][len-1] == '\"') ||
                         (args[j][0] == '\'' && args[j][len-1] == '\''))) {

            args[j][len-1] = '\0'; // Reemplazamos la comilla final por un fin de cadena
            args[j]++;             // Avanzamos el puntero un espacio para ignorar la comilla inicial
                         }
    }

    // se Verifica el background (se ejecuta cuando args ya está lleno)
    if (cant_argumentos > 0 && strcmp(args[cant_argumentos - 1], "&") == 0) {
        *bandera_bg = 1;                   // Le avisamos al llamador que es en background
        args[cant_argumentos - 1] = NULL;  // Eliminamos el "&" para que execvp no falle
    }
}

void ejecutar_comando(char **args, int bandera_bg) {

    if(crear_pipes(args, bandera_bg)){
        return; // si hay pipes, la función crear_pipes se encarga de ejecutar los comandos
    }
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("Error en fork()");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        //proceso hijo con pid identificador = 0
        
        // si el proceso es en primer plano, se restauran las señales para que el comando individual reconozca y obedezca ctrl+c (requerimiento r6)
        if (!bandera_bg) {
            restaurar_senales_hijo_fg();
        }

        if (buscar_redirecciones(args) < 0) {
            exit(EXIT_FAILURE); // si falla el abrir el archivo, termina el hijo
        }

        if (execvp(args[0], args) < 0) {
            perror("Comando no encontrado");
            exit(EXIT_FAILURE); // Finaliza solo al proceso hijo que falló
        }
    } else {
        if (bandera_bg) {
            // significa que ES BACKGROUND (se llena la lista de jobs)
            int job_id = -1;

            // Buscar un espacio libre en la lista_jobs
            for (int j = 0; j < MAX_JOBS; j++) {
                if (!lista_jobs[j].activo) {
                    // Encontramos un espacio vacío
                    lista_jobs[j].id = j + 1; // Asignamos un ID (1-based)
                    lista_jobs[j].pid = pid;  // Guardamos el PID del hijo

                    // guardamos el nombre del comando
                    strncpy(lista_jobs[j].comando, args[0], sizeof(lista_jobs[j].comando) - 1);
                    lista_jobs[j].comando[sizeof(lista_jobs[j].comando) - 1] = '\0';

                    lista_jobs[j].activo = 1; // marcamos como activo
                    job_id = lista_jobs[j].id;
                    break; // Salimos del bucle una vez guardado
                }
            }

            // printear de inmediato el número de job y el PID (Cumpliendo R5)
            if (job_id != -1) {
                printf("[%d] %d\n", job_id, pid);
            } else {
                printf("Error: Límite máximo de jobs alcanzado.\n");
            }

            // IMPORTANTE: NO llamamos a waitpid() aquí.
            // El padre vuelve inmediatamente al ciclo principal para seguir leyendo comandos.

        } else {
            // ES FOREGROUND: El comando normal
            int status;
            waitpid(pid, &status, 0); // Congela la shell esperando al hijo
        }
    }
}

void manejador_sigchld(int sig) {
    (void)sig;
    int status;
    pid_t pid;

    // waitpid con WNOHANG recoge hijos terminados sin bloquear la shell
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {

        for (int i = 0; i < MAX_JOBS; i++) {
            if (lista_jobs[i].activo && lista_jobs[i].pid == pid) {
                lista_jobs[i].activo = 0;

                /* Se verifica si el proceso fallo o cumplio con exito. Anteriormente se mostraba en la terminal
                 * [id]+ Done command indepiendiente si estaba bien escrito o no, con el nuevo condicional si el
                 * comando no existe u ocurre otro error la shell lo notificara
                 */
                if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_FAILURE) {
                    printf("\n[%d]+ Error de ejecucion %s\n", lista_jobs[i].id, lista_jobs[i].comando);
                } else {
                    printf("\n[%d]+ Done %s\n", lista_jobs[i].id, lista_jobs[i].comando);
                }

                /*se restaura el prompt, en un principio interrumpia a la hora de escribir comandos y desordenaba la
                 *terminal, haciendola ver poco profesional, ahora deberia funcionar de manera que cuando llegue la
                 * notificacion se muestre automaticamente el prompt para poder escribir de manera limpia
                 */

                mostrar_prompt();
                fflush(stdout); // Obliga a la terminal a pintar el prompt de inmediato

                break;
            }
        }
    }
}
void expandir_variables (char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        if (args[i][0] == '$') { // si el argumento comienza con '$', es una variable de entorno
            char *nombre_var = args[i] + 1; // obtener el valor de la variable de entorno
            char *valor_var = getenv(nombre_var); //utiliza la funcion getenv para obtener el valor de la variable de entorno
            if (valor_var != NULL) {
                args[i] = valor_var; // reemplazar el argumento con el valor de la variable
            } else {
                args[i] = ""; // si la variable no existe, reemplazar con cadena vacía
            }
        }
    }
}

void ejecutar_export(char **args) { // verifica que el usuario proporciono algo despues del comando export
    if (args[1] == NULL) {
        printf("Uso: export VAR=valor\n");
        return; //finaliza la funcion si no hay argumento
    }

    char *signo_igual = strchr(args[1], '='); // usamos el signo = para indicar que es una variable de entorno y separar el nombre del valor
    
    if (signo_igual != NULL) {
        *signo_igual = '\0'; 
        char *nombre = args[1];          
        char *valor = signo_igual + 1; // el valor es la parte después del '='
        
        setenv(nombre, valor, 1); 
    } else {
        printf("Error: Formato incorrecto. Uso: export VAR=valor\n");
    }
}