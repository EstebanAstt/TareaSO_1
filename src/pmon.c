#include "../include/pmon.h"
#include "../include/job.h"
#include "../include/senales.h"
#include <sys/time.h>
#include <unistd.h>  
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
volatile sig_atomic_t pmon_activo = 0;

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
    pmon_activo = 1; //Se activa la bandera de pmon

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

    printf("\033[?1049h"); //entra ala pantalla alternativa
    fflush(stdout);

    //Se imprime el nombre de las variables con sus respectivos espacios
    printf("PID\tCOMANDO\tESTADO\t%%CPU\tRSS(KB)\n");
    
    //Bucle en del cual solo se puede salir apretando CTRL + C,y que mantendra en ejecucion pmon
    while(!salir_pmon){
        if(tiempo_agotado){
            tiempo_agotado = 0;
            printf("\033[H\033[J"); //Limpia la pantalla hacia abajo para evitar que se sobreescriba la informacion de pmon
            printf("PID\tCOMANDO\tESTADO\t%%CPU\tRSS(KB)\n");
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
                        const char *estado_txt = 
                        (estado == 'R') ? "Ejecutando": 
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
                            } // <- Llave de cierre del while corregida
                            
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
                            printf("%d\t%s\t%s\t%.1f\t%d\n", lista_jobs[i].pid, lista_jobs[i].comando, estado_txt, cpu_porcentaje, rss_kb);            
                        }
                    }
                }
            }
            fflush(stdout); //Se limpia el buffer para que se vea la informacion en pantalla
        }
        pause(); //Duerme hasta que llegue una señal
    }
    //Limpieza final al salir del while
    alarm(0); //Cancela cualquier alarma pendiente para que no interrumpa a la shell
    configurar_senales_shell(); //Restaura la protección de la shell contra Ctrl+C
   
    printf("\033[?1049l"); //salir de la pantalla alternativa y volver a la normal
    printf("\nSaliendo de pmon...\n");
    fflush(stdout);
}