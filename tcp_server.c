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

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int PORT; /* Port that will be opened */
#define BUFF_SIZE 16000
#define BACKLOG 2
char buff[BUFF_SIZE];
int thread_status[BACKLOG] = {0};
char *MSGC[] = {"MSGC01", "MSGC02", "MSGC3", "MSGC04", "MSGC05", "MSGC06", "MSGC07", "MSGC08", "MSGC09", "MSGC10", "MSGC11"};
char *MSGS[] = {"MSGS01", "MSGS02", "MSGS3", "MSGS04", "MSGS05", "MSGS06", "MSGS07", "MSGS08", "MSGS09", "MSGS10", "MSGS11"};

typedef struct{
	char challengerID[1000];
	char challengedID[1000];
	char result[1000];
	int roomID;
} challangeData;

typedef struct
{
	struct sockaddr_in sockaddr;
	char name[1000];
	char password[1000];
	char status;
	char login_status;
	int wrong_password_count;
	struct account *next;
	int session;
	int conn_sock;
} account;

account *list = NULL;
challangeData li[BACKLOG];

void addNewAccount(char name[], char password[], char status, char login_status, int wrong_password_count)
{
	if (list == NULL)
	{
		list = (account *)malloc(sizeof(account));
		list->next = NULL;
		strcpy(list->name, name);
		strcpy(list->password, password);
		list->wrong_password_count = wrong_password_count;
		list->status = status;
		list->login_status = login_status;
		return;
	}
	account *temp = list;
	while (temp->next != NULL)
	{
		temp = temp->next;
	}
	temp->next = (account *)malloc(sizeof(account));
	temp = temp->next;
	strcpy(temp->name, name);
	strcpy(temp->password, password);
	temp->wrong_password_count = wrong_password_count;
	temp->status = status;
	temp->login_status = login_status;
	temp->next = NULL;
	return;
}

int checkAccountStatus(char name[])
{
	account *temp = list;
	if (strcmp(temp->name, name) == 0)
	{
		if (temp->status == '0')
		{
			return 0;
		}
	}
	while (temp->next != NULL)
	{
		temp = temp->next;
		if (strcmp(temp->name, name) == 0)
		{
			if (temp->status == '0')
			{
				return 0;
			}
		}
	}
	return 1;
}

int accountExisted(char name[])
{
	account *temp = list;
	if (strcmp(temp->name, name) == 0)
	{
		return 1;
	}
	while (temp->next != NULL)
	{
		temp = temp->next;
		if (strcmp(temp->name, name) == 0)
		{
			return 1;
		}
	}
	return 0;
}

void writeDataToFile()
{
	FILE *f = fopen("account.txt", "w");

	if (f == NULL)
	{
		return;
	}
	account *temp = list;
	fprintf(f, "%s %s %c %c %d\n", temp->name, temp->password, temp->status, temp->login_status, temp->wrong_password_count);
	while (temp->next != NULL)
	{
		temp = temp->next;
		fprintf(f, "%s %s %c %c %d\n", temp->name, temp->password, temp->status, temp->login_status, temp->wrong_password_count);
	}

	fclose(f);
}

void readDataFromFile()
{
	list = NULL;
	FILE *f = fopen("account.txt", "r");
	char name[100];
	char password[100];
	char status;
	char login_status;
	int wrong_password_count;
	if (f == NULL)
		return;
	while (fscanf(f, "%s %s %c %c %d", name, password, &status, &login_status, &wrong_password_count) == 5)
	{
		addNewAccount(name, password, status, login_status, wrong_password_count);
	}
	fclose(f);
}

int signIn(char name[], char password[])
{
	readDataFromFile();
	account *temp = list;
	if (strcmp(temp->name, name) == 0)
	{
		if (strcmp(temp->password, password) == 0)
		{
			if (temp->status == '0')
			{
				return 0;
			}
			if (temp->login_status == '1')
				return 2;
			temp->login_status = '1';
			return 1;
		}
		if (temp->wrong_password_count < 3)
		{
			temp->wrong_password_count = temp->wrong_password_count + 1;
			writeDataToFile();
			return 3;
		}
		else
		{
			temp->status = '0';
			temp->wrong_password_count = 0;
			writeDataToFile();
			return 0;
		}
	}
	while (temp->next != NULL)
	{
		temp = temp->next;
		if (strcmp(temp->name, name) == 0)
		{
			if (strcmp(temp->password, password) == 0)
			{
				if (temp->status == '0')
				{
					return 0;
				}
				if (temp->login_status == '1')
					return 2;
				temp->login_status = '1';
				return 1;
			}
			if (temp->wrong_password_count < 2)
			{
				temp->wrong_password_count = temp->wrong_password_count + 1;
				writeDataToFile();
				return 3;
			}
			else
			{
				temp->status = '0';
				temp->wrong_password_count = 0;
				writeDataToFile();
				return 0;
			}
		}
	}
}

void signOut(char name[])
{
	readDataFromFile();
	account *temp = list;
	if (strcmp(temp->name, name) == 0)
	{
		temp->login_status = '0';
		return;
	}
	while (temp->next != NULL)
	{
		temp = temp->next;
		if (strcmp(temp->name, name) == 0)
		{
			temp->login_status = '0';
			return;
		}
	}
}

void *handle_client(void *connect_sock)
{
	account *data = (account *)connect_sock;
	pthread_detach(pthread_self());
	int bytes_sent, bytes_received;
	int session = (int)data->session;
	thread_status[session] = 1;
	int conn_sock = data->conn_sock;
	readDataFromFile();
	char msgType[7];
	while(1){
	    memset(buff, '\0', (strlen(buff) + 1));
	    memset(msgType, '\0', (strlen(msgType) + 1));
    	fflush(stdin);
		bytes_received = recv(conn_sock, buff, BUFF_SIZE - 1, 0);
		for(int i=0; i<strlen(buff); i++){
			if(buff[i]=='#'){
				printf("\n");
				break;
			}
			msgType[i] = buff[i];
		}
		int msgTypeInt = -1;
		// kiem tra loai thong diep
		for(int i=0;i<sizeof(MSGC)/sizeof(MSGC[0]);i++){
			if(strcmp(msgType, MSGC[i]) == 0){
				msgTypeInt = i;
				break;
			}
		} 
		printf("%d\n", msgTypeInt);
		switch (msgTypeInt)
		{
		case 1:
			/* code */
			break;
		case 2:
			/* code */
			break;
		case 3:
			/* code */
			break;
		case 4:
			/* code */	
			break;
		case 5:
			/* code */
			break;
		case 6:
			/* code */
			break;
		case 7:
			/* code */
			break;
		case 8:
			/* code */
			break;
		case 9:
			/* code */
			break;
		case 10:
			/* code */
			break;
		case 11:
			/* code */
			break;
		default:
			break;
		}
	}
	thread_status[session] = 0;
    close(conn_sock);
	pthread_exit(NULL);
	// while (1)
	// {
	// 	bytes_sent = send(conn_sock, "ENTER USERNAME", 1024, 0);
	// 	// nhan username
	// 	bytes_received = recv(conn_sock, buff, BUFF_SIZE - 1, 0);
	// 	if (bytes_received < 0)
	// 		perror("\nError: ");
	// 	else
	// 	{
	// 		buff[bytes_received] = '\0';
	// 		buff[strlen(buff) - 1] = '\0';
	// 		if (accountExisted(buff))
	// 		{
	// 			char username[1024];
	// 			strcpy(username, buff);
	// 			bytes_sent = send(conn_sock, "ENTER PASSWORD", 1024, 0);
	// 			// nhan password
	// 			bytes_received = recv(conn_sock, buff, BUFF_SIZE - 1, 0);
	// 			if (bytes_received < 0)
	// 				perror("\nError: ");
	// 			else
	// 			{
	// 				buff[bytes_received] = '\0';
	// 				buff[strlen(buff) - 1] = '\0';
	// 				int sign_in = signIn(username, buff);
	// 				if (sign_in == 1)
	// 				{
	// 					writeDataToFile();
	// 					printf("User %s just logged in\n", username);
	// 					char welcome_message[1024];
	// 					strcpy(welcome_message, "WELCOME ");
	// 					strcat(welcome_message, username);
	// 					strcat(welcome_message, "\n");
	// 					bytes_sent = send(conn_sock, welcome_message, 1024, 0);
	// 					while (1)
	// 					{
	// 						bytes_sent = send(conn_sock, "PRESS ENTER TO LOGOUT", 1024, 0);
	// 						bytes_received = recv(conn_sock, buff, BUFF_SIZE - 1, 0);
	// 						buff[bytes_received] = '\0';
	// 						if (strcmp(buff, "\n") == 0)
	// 						{
	// 							signOut(username);
	// 							writeDataToFile();
	// 							printf("User %s just logged out\n", username);
	// 							char goodbye_message[1024];
	// 							strcpy(goodbye_message, "Goodbye ");
	// 							strcat(goodbye_message, username);
	// 							strcat(goodbye_message, "\n");
	// 							bytes_sent = send(conn_sock, goodbye_message, 1024, 0);
	// 							close(conn_sock);
	// 							// pthread_exit(NULL);
	// 							return;
	// 						}
	// 					}
	// 				}
	// 				else if (sign_in == 2)
	// 				{
	// 					bytes_sent = send(conn_sock, "ACCOUNT IS ALREADY LOGGED IN AT ANOTHER LOCATION\n", 1024, 0);
	// 				}
	// 				else if (sign_in == 3)
	// 				{
	// 					bytes_sent = send(conn_sock, "WRONG PASSWORD\n", 1024, 0);
	// 				}
	// 				else if (sign_in == 0)
	// 				{
	// 					bytes_sent = send(conn_sock, "ACCOUNT IS BLOCKED\n", 1024, 0);
	// 				}
	// 			}
	// 		}
	// 		else
	// 		{
	// 			bytes_sent = send(conn_sock, "ACCOUNT NOT EXIST\n", 1024, 0);
	// 		}
	// 	}
	// }
}

int main(int argc, char *argv[]){
	pthread_t client_thread[BACKLOG];
	if (argc != 2)
	{
		fprintf(stdout, "Please enter port number\n");
		return 0;
	}
	PORT = atoi(argv[1]);
	int listen_sock, conn_sock;
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
		if ((conn_sock = accept(listen_sock, (struct sockaddr *)&client, &sin_size)) == -1){
			perror("\nError: ");
		}
		else  {
			int x;
			for(int i=0;i<BACKLOG;i++){
				if(thread_status[i] == 0){
					printf("You got a connection from %s\n", inet_ntoa(client.sin_addr));
					x = i;
					break;
				}
			}
			account *temp = (account *)malloc(sizeof(account));
			temp->session = x;
			temp->conn_sock = conn_sock;
			pthread_create(&client_thread[x], NULL, &handle_client, (void *)temp);
		}
	}
	for (int i = 0; i < BACKLOG; i++) {
    	pthread_join(client_thread[i], NULL);
	}
	close(listen_sock);
}
