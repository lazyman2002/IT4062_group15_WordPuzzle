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

#include "function.h"

int PORT; /* Port that will be opened */
char buff[BUFF_SIZE-1];

Account *list = NULL;
Logging *logging[BACKLOG];

void addNewAccount(char name[], char password[], char status, char login_status, int wrong_password_count, int elo)
{
    if (list == NULL)
    {
        list = (Account *)malloc(sizeof(Account));
        list->next = NULL;
        strcpy(list->name, name);
        strcpy(list->password, password);
        list->wrong_password_count = wrong_password_count;
        list->status = status;
        list->login_status = login_status;
        list->ELO = elo; // Setting the ELO value
        return;
    }
    Account *temp = list;
    while (temp->next != NULL)
    {
        temp = temp->next;
    }
    temp->next = (Account *)malloc(sizeof(Account));
    temp = temp->next;
    strcpy(temp->name, name);
    strcpy(temp->password, password);
    temp->wrong_password_count = wrong_password_count;
    temp->status = status;
    temp->login_status = login_status;
    temp->ELO = elo; // Setting the ELO value for the new Account
    temp->next = NULL;
    return;
}

int checkAccountStatus(char name[]) {
    if (list == NULL) {
        return 1; // If the list is empty, account status is considered active (returning 1)
    }

    Account *temp = list;
    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) {
            if (temp->status == '0') {
                return 0; // Account found and status is inactive (return 0)
            } else {
                return 1; // Account found and status is active (return 1)
            }
        }
        temp = temp->next;
    }
    return 1; // Account not found, assuming active status (return 1)
}

int accountExisted(char name[]) {
    if (list == NULL) {
        return 0; // If the list is empty, no account can exist
    }

    Account *temp = list;
    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) {
            return 1; // Account with the given name exists
        }
        temp = temp->next;
    }
    return 0; // Account does not exist in the list
}

void writeDataToFile() {
    FILE *f = fopen("account.txt", "w");

    if (f == NULL) {
        return;
    }

    Account *temp = list;
    while (temp != NULL) {
        fprintf(f, "%s %s %c %c %d %d\n", temp->name, temp->password, temp->status, temp->login_status, temp->wrong_password_count, temp->ELO);
        temp = temp->next;
    }

    fclose(f);
}

void readDataFromFile() {
    list = NULL;
    FILE *f = fopen("account.txt", "r");
    char name[100];
    char password[100];
    char status;
    char login_status;
    int wrong_password_count;
    int elo;

    if (f == NULL)
        return;

    while (fscanf(f, "%s %s %c %c %d %d", name, password, &status, &login_status, &wrong_password_count, &elo) == 6) {
        addNewAccount(name, password, status, login_status, wrong_password_count, elo);
    }

    fclose(f);
}

int signIn(char name[], char password[]) {
    readDataFromFile();
    Account *temp = list;

    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) {
            if (strcmp(temp->password, password) == 0) {
                if (temp->status == '0') {
                    return 0; // Account is locked
                }
                if (temp->login_status == '1') {
                    return 2; // Account is already logged in
                }
                temp->login_status = '1';
                writeDataToFile();
                return 1; // Login successful
            } else {
                if (temp->wrong_password_count < 2) {
                    temp->wrong_password_count++;
                    writeDataToFile();
                    return 3; // Incorrect password
                } else {
                    temp->status = '0';
                    temp->wrong_password_count = 0;
                    writeDataToFile();
                    return 0; // Account locked due to multiple incorrect attempts
                }
            }
        }
        temp = temp->next;
    }
    return 3; // Account does not exist
}

void signOut(char name[]) {
    Account *temp = list;

    if (temp == NULL) {
        printf("List is empty!\n");
        return;
    }

    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) {
            temp->login_status = '0';
            writeDataToFile();
            return;
        }
        temp = temp->next;
    }

    printf("Name not found in the list!\n");
}

void *handle_client(void *arg)
{
	pthread_detach(pthread_self());
	int bytes_sent, bytes_received;
	int session = *((int *)arg);
	readDataFromFile();
	char MSG[7];
	while (1)
	{
		memset(buff, '\0', sizeof(buff));
		memset(MSG, '\0', sizeof(MSG));
		pthread_mutex_lock(&mutex);
		bytes_received = recv(logging[session]->conn_sock, buff, BUFF_SIZE - 1, 0);
		pthread_mutex_unlock(&mutex);
		printf("%s\n", buff);
		for(int i=0; i < strlen(buff); i++){
			if(buff[i] == '#'){
				break;
			}
			MSG[i] = buff[i];
		}
		int msgType = -1;
		for(int i = 0; i < (sizeof(MSGC) / sizeof(MSGC[0]) ); i++){
			if(strcmp(MSG, MSGC[i]) == 0){
				msgType = i;
				break;
			}
		}
		printf("MSG Type %d: %d\n", session, msgType+1);
		int timeout = 0;
		switch (msgType)
		{
		case 0:
		//Đăng nhập
			timeout = 0;
			char username[STR_SIZE];
			char password[STR_SIZE];
			memset(username,'\0', sizeof(username));
			memset(password,'\0', sizeof(password));
			for(int i = 7 ; i< strlen(buff); i++){
				if(buff[i] == '#'){
					i++;
					int temp;
					for(temp = i; i<strlen(buff) && buff[i] != '\n';i++){
						password[i-temp] = buff[i];
					}
					password[i - (temp + 1)] = '\0'; 
					break;
				}
				username[i-7] = buff[i];
			}
			printf("?\n%s\n%s\n", username, password);
			pthread_mutex_lock(&mutex);
			printf("Sock:%d",logging[session]->conn_sock);
			bytes_sent = send(logging[session]->conn_sock, "OK", BUFF_SIZE - 1, 0);
			pthread_mutex_unlock(&mutex);
			break;
		case 1:
		//Đăng xuất
			timeout = 0;
			break;
		case 2:
		//Đăng ký
			timeout = 0;
			break;
		case 3:
		//Thách đấu
			timeout = 0;
			break;
		case 4:
		//Trả lời thách đấu
			timeout = 0;
			break;
		case 5:
		//Đầu hàng
			timeout = 0;
			break;
		case 6:
		//Tìm trận
			timeout = 0;
			break;
		case 7:
		//Tìm người rảnh
			timeout = 0;
			break;
		case 8:
		//Timeout
			timeout = 0;
			break;
		case 9:
		//Xem replay
			timeout = 0;
			break;
		case 10:
		//Play
			timeout = 0;
			break;
		case 11:
			timeout = 0;
			break;
		
		default:
			timeout++;
			if(timeout>10)	return 0;
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	for (int i = 0; i < BACKLOG; ++i) {
		logging[i] = (Logging *)malloc(sizeof(Logging));
		if (logging[i] == NULL) {
			exit(EXIT_FAILURE);
		}
		logging[i]->conn_sock = 0;
	}
	pthread_t client_threads[BACKLOG];
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
		{
			printf("You got a connection from %s\n", inet_ntoa(client.sin_addr));
			for(int i = 0; i < BACKLOG; i++){
				if(logging[i]->conn_sock == 0){
					int *session = malloc(sizeof(int));
					if (session == NULL) {
						perror("Error allocating memory");
						exit(EXIT_FAILURE);
					}
					*session = i;
					logging[i]->conn_sock = conn_sock;
					pthread_create(&client_threads[i], NULL, &handle_client, (void *)&i);
					break;
				}
			}
			// pthread_create(&client_thread, NULL, &handle_client, (void *)conn_sock);
		}
	}
	close(listen_sock);
}
