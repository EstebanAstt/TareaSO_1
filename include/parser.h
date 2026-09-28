#ifndef PARSER_H
#define PARSER_H

#include "shell.h"

// funciones de análisis sintáctico y variables
void tokenizar(char *linea, char **args, int *bandera_bg);
void expandir_variables(char **args);

#endif