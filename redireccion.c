#include "redireccion.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int buscar_redirecciones(char **args) { // verifica si hay redirecciones en el comando
    for (int i = 0; args[i] != NULL; i++){
        // redireccion de entrada (<)
        if (strcmp(args[i], "<") == 0) {
            char *archivo = args[i + 1];
            if (archivo == NULL) {
                fprintf(stderr, "Error: No se especificó un archivo para la redirección de entrada.\n");
                return -1; // error
            }
            int fd = open(archivo, O_RDONLY);
            if (fd < 0) {
                perror("Error al abrir el archivo de redirección de entrada");
                return -1; // error
            }
            dup2(fd, STDIN_FILENO);
            close(fd);

            args[i] = NULL; // cortar los argumentos para que execvp no se confunda con la redirección
        }
        // redireccion de salida con truncado a 0 bytes (>)
        else if (strcmp(args[i], ">") == 0) {
            char *archivo = args[i + 1];
            if (archivo == NULL) {
                fprintf(stderr, "Error: No se especificó un archivo para la redirección de salida.\n");
                return -1; // error
            }
            int fd = open(archivo, O_WRONLY | O_CREAT | O_TRUNC, 0644); // permiso de lectura, de creación en caso de no existir con 0644 y truncado
            if (fd < 0) {
                perror("Error al abrir el archivo de redirección de salida");
                return -1; // error
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);

            args[i] = NULL; // cortar los argumentos para que execvp no se confunda con la redirección
        }
        // redireccion de salida con append (>>)
        else if (strcmp(args[i], ">>") == 0) {
            char *archivo = args[i + 1];
            if (archivo == NULL) {
                fprintf(stderr, "Error: No se especificó un archivo para la redirección de salida.\n");
                return -1; // error
            }
            int fd = open(archivo, O_WRONLY | O_CREAT | O_APPEND, 0644); // permiso de lectura, de creación en caso de no existir con 0644 y append
            if (fd < 0) {
                perror("Error al abrir el archivo de redirección de salida");
                return -1; // error
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);

            args[i] = NULL; // cortar los argumentos para que execvp no se confunda con la redirección
        } 
    }
    return 0; // todo salio bien
}
    