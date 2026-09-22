cmake_minimum_required(VERSION 3.20)
project(TareaSO_1 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -std=gnu11")

# Directorio donde CLion buscará las cabeceras (.h)
include_directories(include)

# Fuentes ubicadas dentro de src/
add_executable(mishell
        src/main.c
        src/shell.c
        src/pipes.c
        src/redireccion.c
)