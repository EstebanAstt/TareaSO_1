#ifndef pipes_h
#define pipes_h

int crear_pipes(char **args, int bandera_bg); // crea los pipes necesarios para la ejecución de los comandos
                                              /* se añade de argumento la bandera de bg para poder utilizar
                                               * en conjunto R4 con R5 (pipes con jobs)
                                               */

#endif // pipes_h