# Tarea 1 Sistemas Operativos:
# Implementación de una Shell simple

Una implementación de shell para Linux desarrollada en C, capaz de gestionar la ejecución de procesos, tuberías de $N$ comandos, redirección de entrada/salida, trabajos en segundo plano y monitoreo en vivo de procesos.

---
## Autores

- ### Josefa Arriagada (N° 2025423812)
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
|   └── pmon.h
|   └── parser.h    
├── src/               # Archivos de código fuente (.c)
│   ├── main.c
│   ├── shell.c
│   ├── pipes.c
│   ├── redireccion.c
│   └── senales.c
|   └── pmon.c
|   └── parser.c
├── Makefile           # Script de compilación para la terminal
├── CMakeLists.txt     # Configuración para CLion
├── .gitignore  
├── Informe_TareaSO_1.pdf       
└── README.md
```

---
## Funcionalidades Implementadas

| Componente | Requerimiento              | Descripción | Encargado                         |
| :--- |:---------------------------| :--- |:----------------------------------|
| **Ciclo Básico** | R1                         | Prompt con ruta actual (`getcwd`), lectura con `readline` y cierre con `Ctrl+D` (EOF). | Esteban Astete                    |
| **Comandos Internos** | R2                         | `cd [dir]`, `exit `, `jobs` y `pmon`. | Vicente Carrasco / Esteban Astete |
| **Redirección I/O** | R3                         | Soporte para `<`, `>`, `>>` usando `open()`, `dup2()` y `close()`. | Josefa Arriagada  |                
| **Tuberías (Pipes)** | R4                         | Encadenamiento de N comandos mediante `\|` gestionando descriptores. | Josefa Arriagada  |                
| **Segundo Plano** | R5                         | Detección de `&`, ejecución no bloqueante y recolección asíncrona mediante `SIGCHLD`. | Esteban Astete  |                    
| **Manejo de Señales**| R6                         | La shell ignora `SIGINT` (Ctrl+C) y `SIGTSTP` (Ctrl+Z), restaurándolas en procesos hijo. | Tomas Pizarro |                     
| **Monitor `pmon`** | Punto 3 (comando especial) | Lectura directa de `/proc/[pid]/stat` y `status`, cálculo de %CPU y refresco con `alarm()`. | Vicente Carrasco                  |
| **Historial de comandos navegable con las flechas ↑/↓** |Bonus| Usando biblioteca readline. | Tomás Pizarro                  |
| **CPU en pmon**|Bonus| Agregar orden de la tabla por %CPU (similar a top) y resaltar el proceso con mayor uso. | Vicente Carrasco                  |
| **Asignación a variables de entorno**|Bonus| Variables de entorno propias de la shell y expansión de $VAR dentro de la línea de comandos. | Josefa Arriagada                  |


---

## Ejemplos de uso

### Parsing, espacios, comillas y variables de entorno:
```text
#espacios multiples y pestañas:
miShell:$       ls      -la

#comillas dobles y simples con espacios internos:
miShell:$ echo "hola              si          soy yo estebam"
miShell:$ echo 'argss  con   espacios espaciosos'

#comillas y cadenas vacias:
miShell:$ echo ""
```
### Comandos internos con casos de borde:
```text
#variables de entorno
miShell:$ export VAR=oaMundo
miShell:$ echo $VAR
miShell:$ echo $SOY_UNA_VARIABLE_QUE_NO_EXISTE

#rutas con espacios utilizando comillas 
miShell:$ mkdir "carpeta con espacios"
miShell:$ cd "carpeta con espacios"
miShell:$ cd ..
miShell:$ rmdir "carpeta con espacios"

#navegacion por defecto a HOME 
miShell:$ cd

#directorio inexistente
miShell:$ cd /directorio_que_no_existejeje

#codigo de salida
miShell:$ exit
```

### Redirecciones I/O
```text
#sobreescritura/apendice
miShell:$ echo "Primera Linea" > test_redireccion.txt
miShell:$ echo "Segunda Linea" >> test_redireccion.txt
miShell:$ cat < test_redireccion.txt

#redireccioncon rutas inexistentes de entrada 
miShell:$ cat < archivo_que_no_existe.txt
```

### Pipes multiples

```text
#Encadenamiento de 2 comandos
miShell:$ ls -la | grep src

#encadenamiento largo (mas de 2 comandos)
miShell:$ cat /etc/passwd | grep -v nologin | cut -d: -f1 | sort

#combinacion entre pipes y redireccion de I/O
miShell:$ cat < /etc/passwd | grep root > resultado_root.txt
miShell:$ cat resultado_root.txt
```

### Segundo Plano, señales y pmon
```text

#Multiples trabajos en bg
miShell:$ sleep 12 &
miShell:$ sleep 15 &
miShell:$ sleep 20 &
miShell:$ jobs

#Interrupcion por teclado en un comando fg
miShell:$ sleep 10
(Presionar Ctrl+C antes de acabar el sleep)

#uso de pmon
miShell:$ sleep 30 &
miShell:$ sleep 25 &
miShell:$ sleep 15 &
miShell:$ sleep 10 &
miShell:$ pmon
