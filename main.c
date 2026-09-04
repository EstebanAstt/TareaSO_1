#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 1024
#define MAX_ARGS 64

#define COLOR_VERDE "\033[1;32m"
#define COLOR_AZUL  "\033[1;34m"
#define COLOR_RESET "\033[0m"

// funcion que separa la cadena en tokens individuales (por cada espacio basicamente)
void tokenizar(char *linea, char **args) {
    int i = 0;
    // Elimina el salto de línea al final de fgets
    linea[strcspn(linea, "\n")] = '\0';

    // Separa por espacios y tabulaciones
    char *token = strtok(linea, " \t");
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i] = token;
        i++;
        token = strtok(NULL, " \t");
    }
    args[i] = NULL; // el ultimo puntero en NULL para execvp
}

int main(void) {
    char linea[MAX_LINE];
    char *args[MAX_ARGS];
    char cwd[1024];

    while (1) {
        //se muestra el prompt con el directorio actual
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf(COLOR_VERDE "miShell:" COLOR_AZUL "%s" COLOR_RESET"$ ", cwd);
        } else {
            printf(COLOR_VERDE "miShell" COLOR_AZUL"$ ");
        }
        fflush(stdout);

        //se lee la entrada(Finaliza si el usuario presiona Ctrl+D / EOF)
        if (fgets(linea, sizeof(linea), stdin) == NULL) {
            printf("\n");
            break;
        }

        //se tokeniza la entrada del usuario
        tokenizar(linea, args);

        // Si la linea estaba vacia (sin argumentos, solo se presiono enter)
        if (args[0] == NULL) {
            continue;
        }

        // Salida basica por comando interno exit
        if (strcmp(args[0], "exit") == 0) {
            break;
        }

        //se crea rear proceso y se invoca el ejecutable
        pid_t pid = fork();

        if (pid < 0) {
            perror("Error en fork()");
        } else if (pid == 0) {
            //proceso hijo con pid identificador = 0
            if (execvp(args[0], args) < 0) {
                perror("Comando no encontrado");
                exit(EXIT_FAILURE); // Finaliza solo al proceso hijo que fallo
            }
        } else {
            //proceso padre con pid identificador != 0
            int status;
            waitpid(pid, &status, 0); // Bloquea hasta que el hijo finalice
        }
    }

    return 0;
}
