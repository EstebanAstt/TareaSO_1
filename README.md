-Ciclo basico de la shell y comandos internos (R1–
R2)
- Redireccion de entrada/salida (R3) - fefa (listo)
- Pipes de largo arbitrario (R4) - fefa (listo)
- Ejecucion en background y recoleccion con SIGCHLD - esteban(R5)
- Manejo de SIGINT en foreground vs. shell (R6) toms
- Comando pmon 
- Codigo Fuente 

Soporte para Ctrl+Z (SIGTSTP) que detenga el proceso en foreground, junto con comandos
internos fg y bg para reanudarlo en primer o segundo plano (requiere grupos de procesos y
tcsetpgrp()). [1.0 pt.]
Historial de comandos navegable con las flechas ↑/↓ (usando readline o una implementaci´on
propia simplificada). [1.0 pt.] - toms
Variables de entorno propias de la shell y expansi´on de $VAR dentro de la l´ınea de comandos.
[1.0 pt.]
En pmon, agregar orden de la tabla por %CPU (similar a top) y resaltar el proceso con mayor
uso. [1.0 pt.]
