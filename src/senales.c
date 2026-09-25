#include "../include/senales.h"
#include <stdio.h>
#include <stdlib.h>

void configurar_senales_shell(void) {
    struct sigaction sa_ignora;
    sa_ignora.sa_handler = SIG_IGN;
    sigemptyset(&sa_ignora.sa_mask);
    sa_ignora.sa_flags = 0;

    if (sigaction(SIGINT, &sa_ignora, NULL) < 0 || sigaction(SIGQUIT, &sa_ignora, NULL) < 0) {
        perror("miShell: Error al configurar senales iniciales");
    }
}

void restaurar_senales_hijo_fg(void) {
    struct sigaction sa_dfl;
    sa_dfl.sa_handler = SIG_DFL;
    sigemptyset(&sa_dfl.sa_mask);
    sa_dfl.sa_flags = 0;

    sigaction(SIGINT, &sa_dfl, NULL);
    sigaction(SIGQUIT, &sa_dfl, NULL);
}