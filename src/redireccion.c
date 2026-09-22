#include "../include/redireccion.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int buscar_redirecciones(char **args) {
    int i = 0;
    int corte_args = -1;

    while (args[i] != NULL) {
        // Redirección de entrada (<)
        if (strcmp(args[i], "<") == 0) {
            char *archivo = args[i + 1];
            if (archivo == NULL) {
                fprintf(stderr, "miShell: Error: No se especificó un archivo para la redirección de entrada.\n");
                return -1;
            }
            int fd = open(archivo, O_RDONLY);
            if (fd < 0) {
                perror("miShell: Error al abrir el archivo de redirección de entrada");
                return -1;
            }
            dup2(fd, STDIN_FILENO);
            close(fd);

            if (corte_args == -1) corte_args = i;
            i += 2;
        }
        // Redirección de salida con truncado (>)
        else if (strcmp(args[i], ">") == 0) {
            char *archivo = args[i + 1];
            if (archivo == NULL) {
                fprintf(stderr, "miShell: Error: No se especificó un archivo para la redirección de salida.\n");
                return -1;
            }
            int fd = open(archivo, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("miShell: Error al abrir el archivo de redirección de salida");
                return -1;
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);

            if (corte_args == -1) corte_args = i;
            i += 2;
        }
        // Redirección de salida con append (>>)
        else if (strcmp(args[i], ">>") == 0) {
            char *archivo = args[i + 1];
            if (archivo == NULL) {
                fprintf(stderr, "miShell: Error: No se especificó un archivo para la redirección de salida.\n");
                return -1;
            }
            int fd = open(archivo, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd < 0) {
                perror("Error al abrir el archivo de redirección de salida");
                return -1;
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);

            if (corte_args == -1) corte_args = i;
            i += 2;
        }
        // Argumento común
        else {
            i++;
        }
    }

    if (corte_args != -1) {
        args[corte_args] = NULL;
    }

    return 0;
}