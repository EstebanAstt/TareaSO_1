# Tarea 1 Sistemas Operativos:
# Implementación de una Shell simple

Una implementación de shell para Linux desarrollada en C, capaz de gestionar la ejecución de procesos, tuberías de $N$ comandos, redirección de entrada/salida, trabajos en segundo plano y monitoreo en vivo de procesos.

---
## Autores

- ### Josefa Arriagada (N° )
- ### Esteban Astete (N°2025446162)
- ### Vicente Carrasco (N°2025451514])
- ### Tomás Pizarro (N°2025435799)

---

## Requisitos del Sistema

- **Sistema Operativo:** Linux (Ubuntu / Debian recomendado)
- **Compilador:** GCC (compatible con la norma C11)
- **Herramientas de Construcción:** Make
- `libreadline-dev` (necesario para el historial navegable)

Para instalar las dependencias básicas en Debian/Ubuntu:
```bash
sudo apt update && sudo apt install build-essential libreadline-dev
```
---
## Link del repositorio en GitHub
https://github.com/EstebanAstt/TareaSO_1

---

## Compilación y Ejecución

El sistema utiliza un MakeFile para automatizar el proceso de compilación:
- **Compilar la Shell**
```bash
make
```
- **Ejecutar la Shell**
```bash
./mishell
```

- **Limpiar ejecutables y otros**
```bash
make clean
```

---

## Estructura del proyecto

```text
TareaSO_1/
├── include/           # Archivos de cabecera (.h)
│   ├── shell.h
│   ├── job.h
│   ├── pipes.h
│   ├── redireccion.h
│   └── senales.h
├── src/               # Archivos de código fuente (.c)
│   ├── main.c
│   ├── shell.c
│   ├── job.c
│   ├── pipes.c
│   ├── redireccion.c
│   └── senales.c
├── Makefile           # Script de compilación para la terminal
├── CMakeLists.txt     # Configuración para CLion
├── .gitignore         
└── README.md
```

---
## Funcionalidades Implementadas

| Componente | Requerimiento              | Descripción | Encargado                         |
| :--- |:---------------------------| :--- |:----------------------------------|
| **Ciclo Básico** | R1                         | Prompt con ruta actual (`getcwd`), lectura con `fgets` y cierre con `Ctrl+D` (EOF). | Esteban Astete                    |
| **Comandos Internos** | R2                         | `cd [dir]`, `exit [n]`, `jobs` y `pmon`. | Vicente Carrasco / Esteban Astete |
| **Redirección I/O** | R3                         | Soporte para `<`, `>`, `>>` usando `open()`, `dup2()` y `close()`. | Josefa Arriagada  |                
| **Tuberías (Pipes)** | R4                         | Encadenamiento de N comandos mediante `\|` gestionando descriptores. | Josefa Arriagada  |                
| **Segundo Plano** | R5                         | Detección de `&`, ejecución no bloqueante y recolección asíncrona mediante `SIGCHLD`. | Esteban Astete  |                    
| **Manejo de Señales**| R6                         | La shell ignora `SIGINT` (Ctrl+C) y `SIGTSTP` (Ctrl+Z), restaurándolas en procesos hijo. | Tomas Pizarro |                     
| **Monitor `pmon`** | Punto 3 (comando especial) | Lectura directa de `/proc/[pid]/stat` y `status`, cálculo de %CPU y refresco con `alarm()`. | Vicente Carrasco                  |

---

## Ejemplos de uso
```text
# salir de la shell:
exit

# ejecución de comandos en 2° plano:
sleep 30 &
sleep 20 &
jobs

```
