#ifndef SENALES_H
#define SENALES_H

#include <signal.h>

//configura la shell para ignorar control+c y control+\(R6)
void configurar_senales_shell(void);

//restaura el comportamiento por defecto en el proceso hijo en foregroun(R6)
void restaurar_senales_hijo_fg(void);

#endif 