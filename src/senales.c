#include "../include/senales.h"
#include <stdio.h>
#include <stdlib.h>

void configurar_senales_shell(void) {
    struct sigaction sa_ignora;
    //sig_ign indica al sistema operativo que estas señales deben ser descartadas sin tomar accion
    sa_ignora.sa_handler = SIG_IGN; 
    sigemptyset(&sa_ignora.sa_mask);
    sa_ignora.sa_flags = 0;

    //se asigna la estructura de ignorar a sigint (ctrl+c) y sigquit (ctrl+\) para proteger el bucle principal de la shell
    if (sigaction(SIGINT, &sa_ignora, NULL) < 0 || sigaction(SIGQUIT, &sa_ignora, NULL) < 0) {
        perror("miShell: Error al configurar senales iniciales");
    }
}

void restaurar_senales_hijo_fg(void) {
    struct sigaction sa_dfl;
    //sig_dfl devuelve el comportamiento a su estado predeterminado original (terminar el proceso)
    sa_dfl.sa_handler = SIG_DFL; 
    sigemptyset(&sa_dfl.sa_mask);
    sa_dfl.sa_flags = 0;

    //se aplica la restauracion a los procesos hijos permitiendo que comandos como "sleep 10" si mueran con ctrl+c
    sigaction(SIGINT, &sa_dfl, NULL);
    sigaction(SIGQUIT, &sa_dfl, NULL);
}