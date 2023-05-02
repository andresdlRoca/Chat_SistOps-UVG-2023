#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
//#include <chat.pb-c.h>

#define LENGTH 2048
#define MAX_NAME_LENGTH 32

// Global variables
int exit_status = 0;
int sockfd = 0;
char name[MAX_NAME_LENGTH];

void manage_exit(int sig) {
    exit_status = 1;
}

void sender() {
  	char message[LENGTH] = {};
	char buffer[LENGTH + MAX_NAME_LENGTH] = {};

	while(1) {
		printf("%s", "$ ");
		fflush(stdout);
		fgets(message, LENGTH, stdin);
		trim_string(message, LENGTH);

		if (strcmp(message, "/exit") == 0) {
			break;
		} else if (strcmp(message, "/list") == 0) {
			printf("Aqui se imprimiria la lista\n");
		} else if (strcmp(message, "/activo") == 0) {
			printf("Aqui se cambia el estado a activo\n");
		} else if (strcmp(message, "/ocupado") == 0) {
			printf("Aqui se cambia el estado a ocupado\n");
		} else {
			sprintf(buffer, "%s: %s\n", name, message);
			send(sockfd, buffer, strlen(buffer), 0);
		}

		bzero(message, LENGTH);
		bzero(buffer, LENGTH + MAX_NAME_LENGTH);
	}
	manage_exit(2);
}

void receiver() {
	char message[LENGTH] = {};
	while (1) {
		int receive = recv(sockfd, message, LENGTH, 0);
		if (receive > 0) {
			printf("%s", message);
			printf("%s", "$ ");
			fflush(stdout);
		} else if (receive == 0) {
			break;
		} else {
			printf("No se logro recibir un mensajo con exito.");
			printf("%s", "$ ");
			fflush(stdout);
		}
		memset(message, 0, sizeof(message));
	}
}

void trim_string (char* arr, int length) {
  int i;
  for (i = 0; i < length; i++) {
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;
    }
  }
}

int main(int argc, char **argv){
	if(argc != 2){
		printf("Uso: %s <puerto>\n", argv[0]);
		return EXIT_FAILURE;
	}

	char *ip = "127.0.0.1";
	int port = atoi(argv[1]);


	printf("Cual es tu nombre?: ");
	fgets(name, MAX_NAME_LENGTH, stdin);
	trim_string(name, strlen(name));


	if (strlen(name) > MAX_NAME_LENGTH || strlen(name) < 2){
		printf("El nombre debe tener una longitud de maxima de 30 caracteres y minima de 2 caracteres\n");
		return EXIT_FAILURE;
	}

	struct sockaddr_in server_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);
	server_addr.sin_port = htons(port);


  // Conectar al server
  int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (err == -1) {
		printf("ERROR: conectar al server\n");
		return EXIT_FAILURE;
	}

	// Send name
	send(sockfd, name, MAX_NAME_LENGTH, 0);

	printf("!--- Bienvenidos a La Cueva ---!\n");

	pthread_t sender_thread;
  if(pthread_create(&sender_thread, NULL, (void *) sender, NULL) != 0){
		printf("ERROR: pthread\n");
    return EXIT_FAILURE;
	}

	pthread_t receiver_thread;
  if(pthread_create(&receiver_thread, NULL, (void *) receiver, NULL) != 0){
		printf("ERROR: pthread\n");
		return EXIT_FAILURE;
	}

	while (1){
		if(exit_status){
			printf("\nNos vemos!\n");
			break;
    	}
	}

	close(sockfd);

	return EXIT_SUCCESS;
}
