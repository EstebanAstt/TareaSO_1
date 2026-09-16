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

        //Comando cd puedes cambiar de directorio
        if (strcmp(args[0], "cd") == 0){
            
            const char *dir = (args[1] != NULL) ? args[1] : getenv("HOME"); //Se utiliza en vez de un doble if
            char linea_sin_comillas[MAX_LINE];
            int p = 0;

            //Iteracion para limpiar las comillas simples, ya que hay problemas con chdir
            for(int i = 0; dir[i] != '\0';i++){
                if(dir[i] != '\''){
                    linea_sin_comillas[p] = dir[i];
                    p++;
                }               
            }
            linea_sin_comillas[p] = '\0';
            
            //si uno se cumple es porque hay un error, si no chdir se ejecuta con normalidad
            if(dir == NULL || chdir(linea_sin_comillas) < 0){
                perror("cd");
            }
            
            continue;
        }

        //Crear proceso e invocar el ejecutable
        ejecutar_comando((args));
    }
    return 0;
}