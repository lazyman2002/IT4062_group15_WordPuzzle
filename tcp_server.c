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

#define BUFF_SIZE 2047
#define BACKLOG 2

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int PORT; /* Port that will be opened */
char buff[BUFF_SIZE], buffTemp[BUFF_SIZE];
int thread_status[BACKLOG] = {0};
char *MSGC[] = {"MSGC01", "MSGC02", "MSGC03", "MSGC04", "MSGC05", "MSGC06", "MSGC07", "MSGC08", "MSGC09", "MSGC10", "MSGC11"};
char *MSGS[] = {"MSGS01", "MSGS02", "MSGS03", "MSGS04", "MSGS05", "MSGS06", "MSGS07", "MSGS08", "MSGS09", "MSGS10", "MSGS11"};

typedef struct
{
	int session;
	int conn_sock;
	char name[1000];
	char password[1000];
	char status;
	char login_status;
	int wrong_password_count;
	struct account *next;
} account;

typedef struct{
	int roomStatus;	//0: phòng trống, 1: phòng bận
	int player1Session;
	int player2Session;
	int playerTurn; //0: player1, 1: player2

} challangeData;

account *list = NULL;
account *logging[BACKLOG];
challangeData *li[BACKLOG];

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

void *gameStarting(void *arg){
	int roomSession = (int) arg;
	// khóa các giá trị trong mutex
	pthread_mutex_lock(&mutex);
	// chạy game ở đây
	pthread_mutex_unlock(&mutex);
}


void *sendChallengeMSG(void *arg){
	int bytes_sent, bytes_received;
	int roomSession = (int) arg;
	int player2_conn_sock = logging[li[roomSession]->player2Session]->conn_sock;
	int player1Session = li[roomSession]->player1Session;
	char player1Name[1000];
	memset(player1Name, '\0', (strlen(player1Name) + 1));
	strcpy(player1Name, logging[player1Session]->name);
	char buffT[BUFF_SIZE];
	memset(buffT, '\0', (strlen(buffT) + 1));
	strcpy(buffT, MSGS[5]);
	buffT[strlen(buffT)] = '#';
	strcat(buffT, player1Name);
	bytes_sent = send(player2_conn_sock, buffT, BUFF_SIZE - 1, 0);
	memset(buffT, '\0', (strlen(buffT) + 1));
	bytes_received = recv(player2_conn_sock, buffT, BUFF_SIZE - 1, 0);
	pthread_t gameStart;
	pthread_create(&gameStart, NULL, &gameStarting, (int) roomSession);
	pthread_join(gameStart, NULL);
}

int sendChallenge(int x, char opponentName[]){\
	//0: không tồn tại, 1: đối thủ đồng ý, 2: không đồng ý, 3: bận; 
	int conn_sock = logging[x];
	for(int i=0; i<BACKLOG; i++){
		if(strcpy(logging[x]->name, opponentName) == 0){
			if(logging[x]->login_status != 1){
				return 3;
			}
			else{
				// đợi đồng ý;
				pthread_t waitingAccept;
				int roomSession = -1;
				for(j = 0; j< BACKLOG;j++){
					if(li[j]->roomStatus == 0){
						roomSession = j;
						break;
					}
				}
				if(roomSession == -1)	return 2;
				li[roomSession]->roomStatus = 1;
				li[roomSession]->player1Session = x;
				li[roomSession]->player2Session = i;
				pthread_create(&waitingAccept, NULL, &sendChallengeMSG, (int) roomSession);
				pthread_join(waitingAccept, NULL);
			}
		}
	}
	return 0;
}

void *handle_client(int x)
{
	pthread_detach(pthread_self());
	int bytes_sent, bytes_received;
	int session = (int)logging[x]->session;
	thread_status[session] = 1;
	int conn_sock = logging[x]->conn_sock;
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
		printf("MsgType: %d\n", msgTypeInt);
		strcpy(buffTemp, buff);
		switch (msgTypeInt)
		{
		case 0:
			/* code */
			break;
		case 1:
			/* code */
			break;
		case 2:
			/* code */
			break;
		case 3:
			/* code */
			char opponentName[1000]; 
			for(int i= 7; i< strlen(buffTemp);i++){
				if(buffTemp[i] == '\0')	break;
				opponentName[i-7] = buffTemp[i];
				opponentName[i-6] = '\0';
			}
			printf("%s\n", opponentName);
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
			logging[x]->conn_sock = conn_sock;
			pthread_create(&client_thread[x], NULL, &handle_client, (int) x);
		}
	}
	for (int i = 0; i < BACKLOG; i++) {
    	pthread_join(client_thread[i], NULL);
	}
	close(listen_sock);
}
