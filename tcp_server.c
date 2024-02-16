/*UDP Echo Server*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include <time.h>
#include "function_server.h"

void *handle_client(void *arg){
	pthread_detach(pthread_self());
	int session = *((int*)arg);
	readDataFromFile();
	while (1)
	{
		memset(logging[session].recv_buff, '\0', BUFF_SIZE);
		bytes_received = recv(logging[session].conn_sock, logging[session].recv_buff, BUFF_SIZE, 0);
		if (bytes_received < 0){
			clearThreadData(session);
			perror("\nError: ");
		}
		else{
			printf("Buff recv: %s\n", logging[session].recv_buff);
			int MSGType = -1;
			for(int i = 0; i< sizeof(MSGC) / sizeof(MSGC[0]); ++i){
				if(strncmp(logging[session].recv_buff, MSGC[i], strlen(MSGC[i])) == 0){
					MSGType = i;
					break;
				}
			}
			if(strcmp(logging[session].name, "") != 0){
				writeDatatoLog(logging[session].name);
			}
			switch (MSGType){
				case 0:{
					/* code */
					char username[STR_SIZE];
					char password[STR_SIZE];
					memset(username,'\0', sizeof(username));
					memset(password,'\0', sizeof(password));
					for(int i = 7 ; i< strlen(logging[session].recv_buff); i++){
						if(logging[session].recv_buff[i] == '#'){
							i++;
							int temp;
							for(temp = i; i<strlen(logging[session].recv_buff) && logging[session].recv_buff[i] != '\n';i++){
								password[i-temp] = logging[session].recv_buff[i];
							}
							break;
						}
						username[i-7] = logging[session].recv_buff[i];
					}
					int loginAnswer = login(username, password);
					switch (loginAnswer){
						case 0:{
							/* code */
							bytes_sent = send(logging[session].conn_sock, "MSGS01#0", BUFF_SIZE, 0);
							break;
						}
						case 1:{
							strcpy(logging[session].name, username);
							strcpy(logging[session].password, password);
							Account *temp = list;
							while (temp!= NULL){
								if(strcmp(temp->name, username) == 0){
									logging[session].elo = temp->elo;
									break;
								}
								temp = temp->next;
							}
							memset(logging[session].send_buff, '\0', BUFF_SIZE);
							sprintf(logging[session].send_buff, "MSGS01#1#%d", logging[session].elo);
							bytes_sent = send(logging[session].conn_sock, logging[session].send_buff, BUFF_SIZE, 0);
							break;
						}
						case 2:{
							/* code */
							bytes_sent = send(logging[session].conn_sock, "MSGS01#2", BUFF_SIZE, 0);
							break;
						}
						default:{
							bytes_sent = send(logging[session].conn_sock, "MSGS01#3", BUFF_SIZE, 0);
							break;
						}
					}
					
					break;
				}
				case 1:{
					signout(logging[session].name);
					writeDataToFile();
					memset(logging[session].name, '\0', BUFF_SIZE);
					memset(logging[session].password, '\0', BUFF_SIZE);
					logging[session].elo = 0;
					bytes_sent = send(logging[session].conn_sock, "MSGS02#1", BUFF_SIZE, 0);
					break;
				}
				case 2:{
					char username[STR_SIZE];
					char password[STR_SIZE];
					memset(username,'\0', sizeof(username));
					memset(password,'\0', sizeof(password));
					for(int i = 7 ; i< strlen(logging[session].recv_buff); i++){
						if(logging[session].recv_buff[i] == '#'){
							i++;
							int temp;
							for(temp = i; i<strlen(logging[session].recv_buff) && logging[session].recv_buff[i] != '\n';i++){
								password[i-temp] = logging[session].recv_buff[i];
							}
							break;
						}
						username[i-7] = logging[session].recv_buff[i];
					}
					int signupAnswer = signup(username, password);
					if(signupAnswer == 0){
						addNewAccount(username, password, 1, 0, 0, 0);
						writeDataToFile();
						bytes_sent = send(logging[session].conn_sock, "MSGS03#1", BUFF_SIZE, 0);
					}
					else{
						bytes_sent = send(logging[session].conn_sock, "MSGS03#0", BUFF_SIZE, 0);
					}
					break;
				}
				case 3:{
					char username[STR_SIZE];
					memset(username, '\0', BUFF_SIZE);
					for(int i = 7; i < strlen(logging[session].recv_buff); i++){
						username[i-7] = logging[session].recv_buff[i];
					}
					for(int i = 0; i <BACKLOG; i++){
						if(strcmp(logging[i].name, username) == 0){
							memset(logging[i].send_buff, '\0', BUFF_SIZE);
							sprintf(logging[i].send_buff, "MSGS06#%s", logging[session].name);
							bytes_sent = send(logging[i].conn_sock, logging[i].send_buff, BUFF_SIZE, 0);
						}
					}
					break;
				}
				case 4:{
					/* code */
					break;
				}
				case 5:{
					/* code */
					break;
				}
				case 6:{
					Account *temp = list;
					int c = 3;
					int session2 = -1;
					char xname[STR_SIZE];
					while (temp!=NULL){
						if(temp->login_status == 3 && abs(logging[session].elo - temp->elo) <50){
							temp->login_status = 2;
							c = 2;
							strcpy(xname, temp->name);
							break;
						}
						temp = temp->next;
					}
					if(c == 2){
						for(int i = 0; i < BACKLOG; i++){
							if(strcmp(logging[i].name, xname) == 0){
								session2 = i;
								break;
							}
						}
					}
					temp = list;
					while (temp!=NULL)
					{
						if(strcmp(temp->name, logging[session].name) == 0){
							temp->login_status = c;
							break;
						}
						temp = temp->next;
					}
					writeDataToFile();
					if(session2 != -1){
						printf("player session: %d, %d?\n",session, session2);
						gameStart(session, session2);
					}
					break;
				}
				case 7:{
					memset(logging[session].send_buff, '\0', BUFF_SIZE);
					strcpy(logging[session].send_buff, "MSGS13#");
					Account *temp = list;
					while (temp!=NULL){
						if(temp->login_status == 1 && strcmp(temp->name, logging[session].name) != 0){
							strcat(logging[session].send_buff, temp->name);
							strcat(logging[session].send_buff, "#");
							char cELO[20];
							sprintf(cELO, "%d", temp->elo);
							strcat(logging[session].send_buff, cELO);
							strcat(logging[session].send_buff, "#");
						}
						temp = temp->next;
					}
					bytes_sent = send(logging[session].conn_sock, logging[session].send_buff, BUFF_SIZE, 0);
					break;
				}
				
				default:{
					break;
				}
			}
		}
	}
	

	close(logging[session].conn_sock);
	logging[session].conn_sock = 0;
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	for(int i = 0; i<BACKLOG; i++){
		logging[i].conn_sock = 0;
		strcpy(logging[i].name, "");
	}	
	currentRoomID = 0;
	if (argc != 2)
	{
		fprintf(stdout, "Please enter port number\n");
		return 0;
	}
	PORT = atoi(argv[1]);
	int listen_sock;
	// int server_sock; /* file descriptors */
	struct sockaddr_in server; /* server's address information */
	struct sockaddr_in client; /* client's address information */
	int sin_size;
	// Step 1: Construct a UDP socket
	if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{ /* calls socket() */
		perror("\nError: ");
		exit(0);
	}

	// Step 2: Bind address to socket
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);							/* Remember htons() from "Conversions" section? =) */
	server.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY puts your IP address automatically */
	bzero(&(server.sin_zero), 8);								/* zero the rest of the structure */

	if (bind(listen_sock, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
	{ /* calls bind() */
		perror("\nError: ");
		exit(0);
	}

	// Step 3: Listen request from client
	if (listen(listen_sock, BACKLOG) == -1)
	{ /* calls listen() */
		perror("\nError: ");
		return 0;
	}

	// Step 4: Communicate with clients
	while (1)
	{
		sin_size = sizeof(struct sockaddr_in);
		int conn_sock;
		if ((conn_sock = accept(listen_sock, (struct sockaddr *)&client, &sin_size)) == -1)
			perror("\nError: ");
		else{
			printf("You got a connection from %s\n", inet_ntoa(client.sin_addr));
			for(int i = 0; i < BACKLOG; i++){
				if(logging[i].conn_sock == 0){
					logging[i].conn_sock = conn_sock;
					printf("Current socket: %d\n",logging[i].conn_sock);
					pthread_create(&client_threads[i], NULL, &handle_client, (void *)&i);
					break;
				}
			}
		}
	}
	close(listen_sock);
}
