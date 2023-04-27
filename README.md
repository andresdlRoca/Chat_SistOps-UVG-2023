# Sistemas Operativos - Proyecto Chat
## Seccion 10

El objetivo de este proyecto es reforzar los temas de procesos, threads, concurrencia y comunicacion entre procesos para unificar estos y crear una aplicacion de chat en C con el uso de Sockets.  
  
La aplicacion se encuentra separada en dos partes:  
### Servidor  
Mantiene una lista de todos los clientes/usuarios conectados al sistema. Solo pudiendo existir un solo servidor durante la ejecucion del sistema de chat. El servidor se podra ejecutar en una maquina Linux que provea la infraestructura necesaria y los clientes se onectaran al servidor mediante la direccion IP de la maquina donde se ejecute el servidor.

El servidor debera de proveer estos servicios:
- Registro de usuarios
- Liberacion de usuarios
- Listado de usuarios conectados
- Obtencion de informacion de usuario
- Cambio de estado de usuario
- Broadcasting y mensajes directos

### Cliente
Se conecta y registra con el servidor, por medio del sistema de chat se permite la existencia de uno o mas clientes durante su ejecucion. Posee una pantalla mediante la que se despliegan los mensajes que el usuario ha recibido del servidor, enviados por un usuario remoto; y en la que se permite el ingreso de texto para envio de mensaje a otros usuarios. Cada cliente debe presentar su propia y unica pantalla de chat.

El cliente debera de tener una interfaz con la que el usuario pueda:
- Chatear con todos los usuarios (Broadcasting)
- Enviar y recibir mensajes directos, privados, aparte del chat general.
- Cambiar de estado
- Listar los usuarios conectados al sistema de chat
- Desplegar informacion de un usuario en particular
- Ayuda
- Salir

### Prerequisitos
Work in progress...

### Uso
```
Still a WIP
```


## Autores
👤 Andres de la Roca  
- <a href = "https://www.linkedin.com/in/andr%C3%A8s-de-la-roca-pineda-10a40319b/">Linkedin</a> 
- <a href="https://github.com/andresdlRoca">Github</a>  

👤 Jun Woo Lee
- <a href="https://github.com/jwlh00">Github</a>  

