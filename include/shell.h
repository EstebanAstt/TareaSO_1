#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 1024
#define MAX_ARGS 64

// Colores ANSI
#define COLOR_VERDE "\033[1;32m"
#define COLOR_AZUL  "\033[1;34m"
#define COLOR_RESET "\033[0m"

// Prototipos de funciones
void mostrar_prompt(void);
void tokenizar(char *linea, char **args);
void ejecutar_comando(char **args);

#endif // SHELL_H