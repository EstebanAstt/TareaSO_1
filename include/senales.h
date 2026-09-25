#ifndef SENALES_H
#define SENALES_H

#include <signal.h>

// Configura la shell para ignorar Ctrl+C y Ctrl+\ (R6)
void configurar_senales_shell(void);

// Restaura el comportamiento por defecto en el proceso hijo en foreground (R6)
void restaurar_senales_hijo_fg(void);

#endif // SENALES_H