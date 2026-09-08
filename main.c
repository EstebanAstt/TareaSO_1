#include "shell.h"

int main(void) {
    char linea[MAX_LINE];
    char *args[MAX_ARGS];

    while (1) {
        //se muestra el prompt con el directorio actual
        mostrar_prompt();

        //se lee la entrada (Finaliza si el usuario presiona Ctrl+D / EOF)
        if (fgets(linea, sizeof(linea), stdin) == NULL) {
            printf("\n");
            break;
        }

        //Tokenizar la entrada del usuario
        tokenizar(linea, args);

        // Si la línea estaba vacía (solo presionó Enter)
        if (args[0] == NULL) continue;

        // Salida básica por comando interno exit
        if (strcmp(args[0], "exit") == 0) break;

        //Crear proceso e invocar el ejecutable
        ejecutar_comando((args));
    }
    return 0;
}