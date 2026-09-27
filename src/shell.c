#include "../include/shell.h"
#include "../include/redireccion.h"
#include "../include/pipes.h"
#include "../include/job.h"
#include "../include/senales.h"
#include <sys/time.h>
#include <unistd.h>  
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

//Funcion que separa la cadena en tokens individuales (por cada espacio, excepto si esta entre comillas simples '')
void tokenizar(char *linea, char **args, int *bandera_bg) {
    *bandera_bg = 0; // Inicializamos la bandera en 0 por defecto

    // se elimina el salto de línea al final de fgets
    linea[strcspn(linea, "\n")] = '\0';

    char *inicio_palabra = linea;
    int saltar_espacio = 0;
    int cant_argumentos = 0;

    // Tokenización que
    for (int i = 0; linea[i] != '\0'; i++) {
        // Verificamos si estamos dentro de comillas utilizando saltar_espacio
        if (linea[i] == '\'') saltar_espacio = !saltar_espacio;

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

    // se Verifica el background (se ejecuta cuando args ya está lleno)
    if (cant_argumentos > 0 && strcmp(args[cant_argumentos - 1], "&") == 0) {
        *bandera_bg = 1;                   // Le avisamos al llamador que es en background
        args[cant_argumentos - 1] = NULL;  // Eliminamos el "&" para que execvp no falle
    }
}


void ejecutar_jobs(){
    // Recorremos el arreglo de jobs
    for (int i = 0; i < MAX_JOBS; i++) {
        // Si el trabajo está marcado como activo, lo mostramos
        if (lista_jobs[i].activo) {
            printf("[%d] Ejecutando %s\n", lista_jobs[i].id, lista_jobs[i].comando);
        }
    }
}

//banderas para utilizar en funcion ejecutar_pmon.
volatile sig_atomic_t salir_pmon = 0;
volatile sig_atomic_t tiempo_agotado = 0;

//Defino tiempo pmon fuera para evitar bugs.
int tiempo_pmon = 2;

void manejador_alarma(int signum) {
    (void)signum;
    tiempo_agotado = 1; //Da la señal para volver a ejecutar.
    alarm(tiempo_pmon); //Programa la siguiente alarma.
}

//Funcion que se utiliza cuando aparece CTRL + C
void manejador_sigint_pmon(int signum) {
    (void)signum;
    salir_pmon = 1;
}

void ejecutar_pmon(char **args){
//Limpia las entradas
    fflush(stderr);

    //Le asigna un valor al tiempo en el que se recargan los datos, si no se le agrega un valor se deja por defecto por 2.
    tiempo_pmon = (args[1] != NULL) ? atoi(args[1]) : 2;
    
    //Se actualizan los valores de las banderas proximas a utilizar
    salir_pmon = 0;
    tiempo_agotado = 1;

    //Estructura que refresca la alarma
    struct sigaction sa;
    sa.sa_handler = manejador_alarma; //Funcion a ejecutar
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGALRM, &sa, NULL);

    //Estructura que se encarga de la señal CTRL+C
    struct sigaction sa_int;
    sa_int.sa_handler = manejador_sigint_pmon; //Funcion a ejecutar
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0; //Sin SA_RESTART para despertar a pause() de inmediato
    sigaction(SIGINT, &sa_int, NULL);

    //Programa la primer alarma
    alarm(tiempo_pmon);

    //Se imprime el nombre de las variables con sus respectivos espacios
    printf("PID\tCOMANDO\tESTADO\t%%CPU\tRSS(KB)\n");
    
    //Bucle en del cual solo se puede salir apretando CTRL + C,y que mantendra en ejecucion pmon
    while(!salir_pmon){
        if(tiempo_agotado){
            tiempo_agotado = 0;
            for(int i = 0;i < MAX_JOBS; i++){
                if(lista_jobs[i].activo){

                    //Se guarda los datos del proceso, se crea la ruta para obtener informacion del proceso
                    pid_t pid_pmon = lista_jobs[i].pid;
                    char ruta_arch[300];
                    snprintf(ruta_arch, sizeof(ruta_arch),"/proc/%d/stat",pid_pmon);
                    
                    //Se abre el archivo stat, en modo lectura
                    FILE *archivo = fopen(ruta_arch,"r");

                    if(archivo != NULL){
    
                        //obtener el tiempo real en segundos
                        struct timeval tv;
                        gettimeofday(&tv, NULL);
                        double tiempo_actual = tv.tv_sec + (tv.tv_usec / 1000000.0);

                        char estado;
                        unsigned long utime,stime;
                        
                        //Se lee lo que esta dentro del archivo y utilizando %* salto lineas no deseadas, y leo solamente lo necesario
                        fscanf(archivo, "%*d %*s %c %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %lu %lu", &estado, &utime, &stime);
                        
                        //Se cambia el contenido de estado dependiendo de la lectura, para cumplir con la pauta.
                        estado = 
                        (estado == 'R') ? 'Ejecutando': 
                        (estado == 'S') ? "ejecutando":
                        (estado == 'S') ? "durmiendo":
                        (estado == 'D') ? "esperando":
                        (estado == 'Z') ? "zombie":
                        (estado == 'T') ? "detenido":"desconocido";

                        //Se cierra el archivo_stat
                        fclose(archivo);

                        //Se inicializan las variables para VmRSS y se crea puntero al archivo para posterior lectura.
                        int rss_kb = 0;
                        char ruta_rss[300];
                        snprintf(ruta_rss, sizeof(ruta_rss), "/proc/%d/status", pid_pmon);
                        FILE *archivo_status = fopen(ruta_rss, "r");

                        if(archivo_status != NULL){
                            char linea_status[256];

                            //Se lee el archivo y mediante el if se lee solamente el valor de VmRSS
                            while (fgets(linea_status, sizeof(linea_status), archivo_status)) {

                                if (strncmp(linea_status, "VmRSS:", 6) == 0) {
                                    sscanf(linea_status,"VmRSS: %d",&rss_kb);
                                    break;
                
                            }
                        }
                        //Cerramos el archivo status
                        fclose(archivo_status);
                        
                        unsigned long tiempo_cpu_actual = utime + stime;
                        double cpu_porcentaje = 0.0; //Variable para almacenar e imprimir el resultado

                        //Calculo de %CPU
                        if(lista_jobs[i].prim_lect == 1){
                            //En la primera lectura no hay datos por lo que se almacenan los iniciales para su posterior lectura.
                            lista_jobs[i].ant_cpu_time = tiempo_cpu_actual;
                            lista_jobs[i].prev_timestamp = tiempo_actual;
                            lista_jobs[i].prim_lect = 0; //Apagamos la bandera para la próxima lectura
                            cpu_porcentaje = 0.0;
                        }
                        else {
                            //Si no es la primera lectura se calcula la diferencia de ticks y tiempo en segundos
                            unsigned long delta_cpu_ticks = tiempo_cpu_actual - lista_jobs[i].ant_cpu_time;
                            double delta_tiempo_seg = tiempo_actual - lista_jobs[i].prev_timestamp;

                            //Convertir ticks a segundos consultando la configuración del sistema
                            double delta_cpu_seg = (double)delta_cpu_ticks / sysconf(_SC_CLK_TCK);

                            //Se calcula el porcentaje CPU y evita posible error si es que el tiempo llegara a dar 0.
                            if (delta_tiempo_seg > 0) {
                                cpu_porcentaje = (delta_cpu_seg / delta_tiempo_seg) * 100.0;
                            }

                            // Actualizamos el historial para el siguiente refresco de pmon
                            lista_jobs[i].ant_cpu_time = tiempo_cpu_actual;
                            lista_jobs[i].prev_timestamp = tiempo_actual;
                        }
                        //Se imprimen los datos en pantalla para cada proceso
                        printf("%d\t%s\t%c\t%.1f\t%d\n", lista_jobs[i].pid, lista_jobs[i].comando, estado, cpu_porcentaje, rss_kb);            
                        }
                    }
                }
            }
        }
        pause(); //Duerme hasta que llegue una señal
    }
    //Limpieza final al salir del while
    alarm(0); //Cancela cualquier alarma pendiente para que no interrumpa a la shell
    configurar_senales_shell(); //Restaura la protección de la shell contra Ctrl+C
    printf("\nSaliendo de pmon...\n");
}

void ejecutar_cd(char **args){
    //Comando cd puedes cambiar de directorio
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
    //Termina la cadena
    linea_sin_comillas[p] = '\0';

    //Se cambia de directorio y se comprueba si hay error
    if(chdir(linea_sin_comillas) < 0){
        perror("cd");
    }
}

int manejador_entradas(char **args){

    if(args == NULL || args[0] == NULL){
        return 0; //No ingresaron ningun comando
    }

    if(strcmp(args[0],"exit") == 0){
        int codigo = (args[0] != NULL) ? atoi(args[0]): 0;
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

    return 0;
}

void ejecutar_comando(char **args, int bandera_bg) {

    if(crear_pipes(args, bandera_bg)){
        return; // si hay pipes, la función crear_pipes se encarga de ejecutar los comandos
    }

    pid_t pid = fork();
    if(pid < 0) {
        perror("Error en fork()");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) {
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
                    lista_jobs[j].prim_lect = 1;
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