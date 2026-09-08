#include "shell.h"

void mostrar_prompt(void) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf(COLOR_VERDE "miShell:" COLOR_AZUL "%s" COLOR_RESET "$ ", cwd);
    } else {
        printf(COLOR_VERDE "miShell" COLOR_RESET "$ ");
    }
    fflush(stdout);
}

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

void ejecutar_comando(char **args) {

    pid_t pid = fork();
    if (pid < 0) {
        perror("Error en fork()");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        //proceso hijo con pid identificador = 0
        if (execvp(args[0], args) < 0) {
            perror("Comando no encontrado");
            exit(EXIT_FAILURE); // Finaliza solo al proceso hijo que falló
        }
    } else {
        //proceso padre con pid identificador != 0
        int status;
        waitpid(pid, &status, 0); // Bloquea hasta que el hijo finalice
    }

}