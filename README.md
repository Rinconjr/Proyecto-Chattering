# Proyecto-Chattering

**Explicacion proyecto:**

Sistema Chattering con arquitectura Cliente/Servidor en la cual hay un proceso Servidor (Manager) y varios procesos Cliente (Talker).

* **Talker:** Cada usuario que desee interactuar con otros usuarios a través del Chattering, deberá invocar un proceso talker (el cliente). Ese proceso le permitirá enviar y recibir mensajes, crear grupos, listar los usuarios y grupos en el sistema, etc. 

* **Manager:** Registrara la información de clientes y grupos en el sistema y será intermediario para la comunicación entre los diferentes talkers.

Este proyecto esta basado en **pipes nominales y señales** utilizando llamadas al sistema operativo para su manejo.

**Compilacion:**
* Manager:

        gcc manager.c -o manager

* Talker:

        gcc talker.c -o talker

* O Utilizando el makefile:

        make

**Ejecución:**

* Manager:

        ./manager –n N -p pipeNom

* Talker:

        ./talker –i idTalker -p pipeNom

**IMPORTANTE:** Para el correcto funcionamiento del programa, se debe ejecutar primero el manager y despues los talkers.
