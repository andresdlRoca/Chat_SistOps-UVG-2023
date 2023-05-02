#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>


#define LIMIT_CLIENT 10
#define MSG_LIMIT 500

static _Atomic unsigned int cli_count = 0;
static int uid = 10;

// Estructura del cliente
typedef struct{
	struct sockaddr_in address;
	char state[16]; // "activo", "ocupado" o "inactivo"
	char name[32];
	int sockfd;
	int uid;
} client_obj;

client_obj *clients[LIMIT_CLIENT]; // Array de clientes

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void trim_newline (char* str) {
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] == '\n') {
            str[i] = '\0';
            break;
        }
        i++;
    }
}


void print_client_addr(struct sockaddr_in addr){
    printf("%d.%d.%d.%d",
        addr.sin_addr.s_addr & 0xff,
        (addr.sin_addr.s_addr & 0xff00) >> 8,
        (addr.sin_addr.s_addr & 0xff0000) >> 16,
        (addr.sin_addr.s_addr & 0xff000000) >> 24);
}

// Registro de usuarios
void register_user(client_obj *cl){
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i < LIMIT_CLIENT; ++i){
		if(!clients[i]){
			clients[i] = cl;
			break;
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

// Liberacion de usuarios
void free_user(int uid){
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i < LIMIT_CLIENT; ++i){
		if(clients[i]){
			if(clients[i]->uid == uid){
				clients[i] = NULL;
				break;
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

//Mensaje global
void send_message(char *s, int uid){
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i<LIMIT_CLIENT; i++){
		if(clients[i]){
			if(clients[i]->uid != uid){
				if(write(clients[i]->sockfd, s, strlen(s)) < 0){
					perror("ERROR: escritura fallida");
					break;
				}
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

void send_one_message(char *msg, int uid_sender, int uid_receiver){
	pthread_mutex_lock(&clients_mutex);

	for(int i = 0; i<LIMIT_CLIENT; i++) {
		if(clients[i]) {
			if(clients[i]->uid == uid_receiver){
				if(write(clients[i]->sockfd, msg, strlen(msg)) < 0){
					perror("Error: Fallo de mensaje privado");
					break;
				}
			}
		}
	}

	printf("Hello");
	pthread_mutex_unlock(&clients_mutex);

}

void list_all_users(int uid) {
	pthread_mutex_lock(&clients_mutex);

	char list_users[MSG_LIMIT] = {};

	strcat(list_users, "Usuarios|Estado\n");

	for(int i = 0; i<LIMIT_CLIENT; ++i){
		if(clients[i]) {
			if(clients[i] -> uid != uid) {
				strcat(list_users, clients[i] -> name);
				strcat(list_users, "|");
				strcat(list_users, clients[i]->state);
				strcat(list_users, "\n");
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
	send_one_message(list_users, uid, uid);
}

// Manejo de la comunicacion con el cliente
void *handle_client(void *arg){
	char buff_out[MSG_LIMIT];
	char name[32];
	int leave_flag = 0;

	cli_count++;
	client_obj *cli = (client_obj *)arg;

	// Name
	if(recv(cli->sockfd, name, 32, 0) <= 0 || strlen(name) <  2 || strlen(name) >= 32-1){
		printf("Nombre invalido.\n");
		leave_flag = 1;
	} else{

		for(int i = 0; i<LIMIT_CLIENT; ++i){
			if(clients[i]) {
				if(strcmp(clients[i] -> name, name) == 0) {
					printf("Usuario ya en uso\n");
					leave_flag = 1;
				}
			}
		}
		if (leave_flag == 0) {
			strcpy(cli->name, name);
			sprintf(buff_out, "%s se ha unido!\n", cli->name);
			printf("%s", buff_out);
			send_message(buff_out, cli->uid);
		}
	}

	bzero(buff_out, MSG_LIMIT);

	while(1){
		if (leave_flag) {
			break;
		}
		int receive = recv(cli->sockfd, buff_out, MSG_LIMIT, 0);
		if (receive > 0){
				send_message(buff_out, cli->uid);
				trim_newline(buff_out);
				printf("%s -> %s\n", buff_out, cli->name);
		} else if(strcmp(buff_out, "/list") == 0) {
			printf("Se mostrara la lista\n");
			list_all_users(cli->uid);
		} else if (receive == 0 || strcmp(buff_out, "/exit") == 0){
			sprintf(buff_out, "%s se ha pirado.\n", cli->name);
			printf("%s", buff_out);
			send_message(buff_out, cli->uid);
			leave_flag = 1;
		} else {
			printf("ERROR: -1\n");
			leave_flag = 1;
		}

		bzero(buff_out, MSG_LIMIT);
	}

  	// Liberar usuario y thread
	close(cli->sockfd);
	free_user(cli->uid);
	free(cli);
	cli_count--;
	pthread_detach(pthread_self());

	return NULL;
}

int main(int argc, char **argv){
	if(argc != 2){
		printf("Uso: %s <puerto>\n", argv[0]);
		return EXIT_FAILURE;
	}

	char *ip = "127.0.0.1";
	int port = atoi(argv[1]);
	int option = 1;
	int listenfd = 0, connfd = 0;
	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;
	pthread_t tid;

  // Settings del socket
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(ip);
  serv_addr.sin_port = htons(port);

	signal(SIGPIPE, SIG_IGN);

	if(setsockopt(listenfd, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0){
		perror("ERROR: setsockopt fallido");
    return EXIT_FAILURE;
	}

  if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("ERROR: Bindeo del socket fallido");
    return EXIT_FAILURE;
  }

  if (listen(listenfd, 10) < 0) {
    perror("ERROR: Fallo al escuchar al socket");
    return EXIT_FAILURE;
	}

	printf("!--- Bienvenidos a La Cueva ---!\n");

	while(1){
		socklen_t clilen = sizeof(cli_addr);
		connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);

		//Chequeo de maximo de clientes
		if((cli_count + 1) == LIMIT_CLIENT){
			printf("Capacidad del clientes excedida ");
			print_client_addr(cli_addr);
			printf(":%d\n", cli_addr.sin_port);
			close(connfd);
			continue;
		}

		// Ajustes del cliente
		client_obj *cli = (client_obj *)malloc(sizeof(client_obj));
		cli->address = cli_addr;
		cli->sockfd = connfd;
		cli->uid = uid++;

		//Añadir cliente a la fila
		register_user(cli);
		pthread_create(&tid, NULL, &handle_client, (void*)cli);

		// Optimizacion para que Linux no pete.
		sleep(1);
	}

	return EXIT_SUCCESS;
}