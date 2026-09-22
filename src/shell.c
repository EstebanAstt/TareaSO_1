#include "../include/shell.h"
#include "../include/redireccion.h"
#include "../include/pipes.h"

void mostrar_prompt(void) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf(COLOR_VERDE "miShell:" COLOR_AZUL "%s" COLOR_RESET "$ ", cwd);
    } else {
        printf(COLOR_VERDE "miShell" COLOR_RESET "$ ");
    }
    fflush(stdout);
}

//Funcion que separa la cadena en tokens individuales (por cada espacio, excepto si esta entre comillas simples '')
void tokenizar(char *linea, char **args) {
    // Elimina el salto de línea al final de fgets
    linea[strcspn(linea, "\n")] = '\0';

    char *inicio_palabra = linea;
    int saltar_espacio = 0;
    int cant_argumentos = 0;

    for(int i = 0;linea[i] != '\0';i++){
        //Verificamos si estamos dentro de comillas utilizando saltar_espacio
        if(linea[i] == '\'') saltar_espacio = !saltar_espacio;

        //Encontramos un espacio y verificamos que no estemos dentro de comillas
        if(linea[i] == ' ' && saltar_espacio == 0){
            linea[i] = '\0';

            if(*inicio_palabra != '\0'){
                args[cant_argumentos] = inicio_palabra;
                cant_argumentos++;
            }
        //para que empiece luego del '\0'
        inicio_palabra = &linea[i+1];
        }
    }
    //ya que el ciclo termina cuando encuentra '\0', se agrega la ultima palabra si esta no es '\0'
    if(*inicio_palabra != '\0'){
        args[cant_argumentos] = inicio_palabra;
        cant_argumentos++;
    }
    args[cant_argumentos] = NULL; // el ultimo puntero en NULL para execvp
}

void ejecutar_comando(char **args) {

    if(crear_pipes(args)){
        return; // si hay pipes, la función crear_pipes se encarga de ejecutar los comandos
    }
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("Error en fork()");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        //proceso hijo con pid identificador = 0
        if (buscar_redirecciones(args) < 0) {
            exit(EXIT_FAILURE); // si falla el abrir el archivo, termina el hijo
        }

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