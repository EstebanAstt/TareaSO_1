#include "../include/parser.h"
#include <string.h>
#include <stdlib.h>

// Funcion que separa la cadena en tokens individuales (por cada espacio, excepto si esta entre comillas simples '')
void tokenizar(char *linea, char **args, int *bandera_bg) {
    *bandera_bg = 0; // Inicializamos la bandera en 0 por defecto

    // se elimina el salto de línea al final de fgets
    linea[strcspn(linea, "\n")] = '\0';

    char *inicio_palabra = linea;
    int saltar_espacio = 0;
    int cant_argumentos = 0;

    // Tokenizacion
    for (int i = 0; linea[i] != '\0'; i++) {
        // Verificamos si estamos dentro de comillas utilizando saltar_espacio
        if (linea[i] == '\'' || linea[i] == '\"') saltar_espacio = !saltar_espacio;

        // Encontramos un espacio y verificamos que no estemos dentro de comillas
        if (linea[i] == ' ' && saltar_espacio == 0) {
            linea[i] = '\0';

            if (*inicio_palabra != '\0') {
                args[cant_argumentos] = inicio_palabra;
                cant_argumentos++;
            }
            // Para que empiece luego del '\0'
            inicio_palabra = &linea[i + 1];
        }
    }

    // Agrega la ultima palabra si no es '\0'
    if (*inicio_palabra != '\0') {
        args[cant_argumentos] = inicio_palabra;
        cant_argumentos++;
    }
    args[cant_argumentos] = NULL; // El último puntero en NULL para execvp

    // Recorremos todos los argumentos guardados para quitarles las comillas de los extremos (en caso de que se utilizaron comillas
    for (int j = 0; j < cant_argumentos; j++) {
        int len = strlen(args[j]);
        // Si el argumento tiene al menos 2 caracteres y está envuelto en comillas simples o dobles
        if (len >= 2 && ((args[j][0] == '\"' && args[j][len-1] == '\"') ||
                         (args[j][0] == '\'' && args[j][len-1] == '\''))) {

            args[j][len-1] = '\0'; // Reemplazamos la comilla final por un fin de cadena
            args[j]++;             // Avanzamos el puntero un espacio para ignorar la comilla inicial
                         }
    }

    // se Verifica el background (se ejecuta cuando args ya está lleno)
    if (cant_argumentos > 0 && strcmp(args[cant_argumentos - 1], "&") == 0) {
        *bandera_bg = 1;                   // Le avisamos al llamador que es en background
        args[cant_argumentos - 1] = NULL;  // Eliminamos el "&" para que execvp no falle
    }
}

void expandir_variables(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        if (args[i][0] == '$') { // si el argumento comienza con '$', es una variable de entorno
            char *nombre_var = args[i] + 1; // obtener el valor de la variable de entorno
            char *valor_var = getenv(nombre_var); //utiliza la funcion getenv para obtener el valor de la variable de entorno
            if (valor_var != NULL) {
                args[i] = valor_var; // reemplazar el argumento con el valor de la variable
            } else {
                args[i] = ""; // si la variable no existe, reemplazar con cadena vacía
            }
        }
    }
}