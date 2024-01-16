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
#include <stdbool.h>
#include <math.h>
#include "function.h"
// pthread_mutex_lock(&file_mutex);
// pthread_mutex_unlock(&file_mutex);
int PORT; /* Port that will be opened */
char buff[BUFF_SIZE-1];
Account *list = NULL;
Logging logging[BACKLOG];
challangeData li[BACKLOG];

void replySubmit(int roomSession, int turn, int row, bool rightChoice){
	int session1 = li[roomSession].player1Session;
	int session2 = li[roomSession].player2Session;
	int bytes_sent, bytes_received;
	if(rightChoice == false){
		pthread_mutex_lock(&threads_mutex);
		memset(buff, '\0', BUFF_SIZE);
		sprintf(buff, "MSGS05#%d#%d#%d#-1##",li[roomSession].player1Score, li[roomSession].player2Score, turn);
		bytes_sent = send(logging[session1].conn_sock, buff, BUFF_SIZE - 1, 0);
		bytes_sent = send(logging[session2].conn_sock, buff, BUFF_SIZE - 1, 0);
		pthread_mutex_unlock(&threads_mutex);
	}
	else{
		if(row >0){
			pthread_mutex_lock(&threads_mutex);
			memset(buff, '\0', BUFF_SIZE);
			sprintf(buff, "MSGS05#%d#%d#%d#%d#%s",li[roomSession].player1Score, li[roomSession].player2Score, turn, row-1, li[roomSession].quiz.rowData[row-1].rowAnswer);
			bytes_sent = send(logging[session1].conn_sock, buff, BUFF_SIZE - 1, 0);
			bytes_sent = send(logging[session2].conn_sock, buff, BUFF_SIZE - 1, 0);
			pthread_mutex_unlock(&threads_mutex);
		}else{	
			pthread_mutex_lock(&threads_mutex);
			memset(buff, '\0', BUFF_SIZE);
			sprintf(buff, "MSGS05#%d#%d#%d#%d#%s",li[roomSession].player1Score, li[roomSession].player2Score, turn, -2, li[roomSession].quiz.colAnswer);
			bytes_sent = send(logging[session1].conn_sock, buff, BUFF_SIZE - 1, 0);
			bytes_sent = send(logging[session2].conn_sock, buff, BUFF_SIZE - 1, 0);
			pthread_mutex_unlock(&threads_mutex);
		}
		int x = 0;
		for(int i = 0; i<li[roomSession].quiz.row; i++){
			if(li[roomSession].quiz.rowData[i].rowAnswered ==0){
				x = 1;
				break;
			}
		}
		if(x == 1){
			Account *temp = list;
			int session1, session2;
			session1 = li[roomSession].player1Session;
			session2 = li[roomSession].player2Session;
			if(li[roomSession].player1Score > li[roomSession].player2Score){
				while (temp!=NULL)
				{
					if(strcmp(temp->name, logging[session2].name) == 0){
						temp->ELO= (0>(temp->ELO - 10)) ? 0 : temp->ELO - 10;//max(0, temp->ELO - 10);	
						logging[session2].ELO = temp->ELO;
						temp->login_status = '1';
					}
					if(strcmp(temp->name, logging[session1].name) == 0){
						temp->ELO = temp->ELO + 10;
						logging[session1].ELO = temp->ELO;
						temp->login_status = '1';
					}
					temp = temp->next;
				}
				writeDataToFile();
			}
			else if(li[roomSession].player1Score == li[roomSession].player2Score){
				
			}else{
				while (temp!=NULL)
				{
					if(strcmp(temp->name, logging[session1].name) == 0){
						temp->ELO= (0>(temp->ELO - 10)) ? 0 : temp->ELO - 10;//max(0, temp->ELO - 10);	
						logging[session1].ELO = temp->ELO;
						temp->login_status = '1';
					}
					if(strcmp(temp->name, logging[session2].name) == 0){
						temp->ELO = temp->ELO + 10;
						logging[session2].ELO = temp->ELO;
						temp->login_status = '1';
					}
					temp = temp->next;
				}
				writeDataToFile();
			}
			pthread_mutex_lock(&threads_mutex);
			memset(buff, 0, STR_SIZE);
			sprintf(buff, "MSGS08#%d#", logging[session1].ELO);
			bytes_sent = send(logging[session1].conn_sock, buff, BUFF_SIZE - 1, 0);
			memset(buff, 0, STR_SIZE);
			sprintf(buff, "MSGS08#%d#", logging[session2].ELO);
			bytes_sent = send(logging[session2].conn_sock, buff, BUFF_SIZE - 1, 0);			
			pthread_mutex_unlock(&threads_mutex);
			return;
		}
	}
}
void gameStart(int session1, int session2){
	int roomSession;
	int bytes_sent, bytes_received;
	for(int i = 0; i< BACKLOG;i++){
		if(li[i].roomStatus == 0){
			roomSession = i;
			li[i].roomStatus = 1;
			break;
		}
	}
	li[roomSession].player1Session = session1;
	li[roomSession].player2Session = session2;
	FILE *f = fopen("quiz.txt", "r");
	int row = 0;
	int space;
	char answer[STR_SIZE];
	char hints[SENTENCE_SIZE];
	char buff1[SENTENCE_SIZE];
	char buff2[SENTENCE_SIZE];

	fscanf(f, "%d %s", &space, answer);
	fscanf(f, " %99[^\n]", hints);
	li[roomSession].quiz.collum = space;
	strcpy(li[roomSession].quiz.colAnswer, answer);
	strcpy(li[roomSession].quiz.colHints, hints);
	while (fscanf(f, "%d %s", &space, answer) == 2)
	{
		fscanf(f, " %99[^\n]", hints);
		li[roomSession].quiz.rowData[row].spacing = space;
		li[roomSession].quiz.rowData[row].rowAnswered = 0;
		strcpy(li[roomSession].quiz.rowData[row].rowAnswer, answer);
		strcpy(li[roomSession].quiz.rowData[row].rowHints, hints);
		row++;
	}
	pthread_mutex_lock(&threads_mutex);
	memset(buff, '\0', BUFF_SIZE);
	sprintf(buff, "MSGS04#%s#%s#%d#%d#%s#", logging[session1].name, logging[session2].name, row, li[roomSession].quiz.collum, li[roomSession].quiz.colHints);
	bytes_sent = send(logging[session1].conn_sock, buff, BUFF_SIZE - 1, 0);
	bytes_sent = send(logging[session2].conn_sock, buff, BUFF_SIZE - 1, 0);
	pthread_mutex_unlock(&threads_mutex);
	for(int i = 0; i<row; i++){
		pthread_mutex_lock(&threads_mutex);
		memset(buff, '\0', BUFF_SIZE);
		sprintf(buff, "MSGS12#%d#%d#%d#%s#", i, li[roomSession].quiz.rowData[i].spacing, strlen(li[roomSession].quiz.rowData[i].rowAnswer), li[roomSession].quiz.rowData[i].rowHints);
		bytes_sent = send(logging[session1].conn_sock, buff, BUFF_SIZE - 1, 0);
		bytes_sent = send(logging[session2].conn_sock, buff, BUFF_SIZE - 1, 0);
		pthread_mutex_unlock(&threads_mutex);
	}
	li[roomSession].playerTurn  = 0;
	li[roomSession].player1Score = 0;
	li[roomSession].player2Score = 0;
}
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
	pthread_mutex_lock(&file_mutex);
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
	pthread_mutex_unlock(&file_mutex);
}
void readDataFromFile() {
    list = NULL;
    FILE *f = fopen("account.txt", "r");
    char name[STR_SIZE];
    char password[STR_SIZE];
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
	printf("Socket %d: %d", session, logging[session].conn_sock);
	readDataFromFile();
	char MSG[7];
	while (1)
	{
		memset(buff, '\0', sizeof(buff));
		memset(MSG, '\0', sizeof(MSG));
		bytes_received = recv(logging[session].conn_sock, buff, BUFF_SIZE - 1, 0);
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
		printf("MSG Type %d: %d\n", session, msgType);
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
					break;
				}
				username[i-7] = buff[i];
			}
			printf("Login:\n?%s - %s?\n", username, password);
			if(list == NULL)	printf("no\n");
			int signin = signIn(username, password);
			printf("Sign: %d\n", signin);
			switch (signin)
			{
			case 0:
				/* code */
				pthread_mutex_lock(&threads_mutex);
				bytes_sent = send(logging[session].conn_sock, "MSGS01#0", BUFF_SIZE - 1, 0);
				pthread_mutex_unlock(&threads_mutex);
				break;
			case 1:
				/* code */
				logging[session].login_status = 1;
				strcpy(logging[session].name, username);
				strcpy(logging[session].password, password);
				memset(logging[session].buff, '\0', sizeof(logging[session].buff));
				Account *temp = list;
				while (temp!=NULL)
				{
					if(strcmp(temp->name, username) == 0){
						pthread_mutex_lock(&threads_mutex);
						logging[session].ELO = temp->ELO;
						break;
					}
					temp = temp->next;
				}
				sprintf(logging[session].buff, "MSGS01#1#%d", logging[session].ELO);
				bytes_sent = send(logging[session].conn_sock, logging[session].buff, BUFF_SIZE - 1, 0);
				pthread_mutex_unlock(&threads_mutex);
				break;
			case 2:
				pthread_mutex_lock(&threads_mutex);
				bytes_sent = send(logging[session].conn_sock, "MSGS01#2", BUFF_SIZE - 1, 0);
				pthread_mutex_unlock(&threads_mutex);
				printf("sended");
				break;
			default:
				pthread_mutex_lock(&threads_mutex);
				bytes_sent = send(logging[session].conn_sock, "MSGS01#3", BUFF_SIZE - 1, 0);
				pthread_mutex_unlock(&threads_mutex);
				break;
			}
			// pthread_mutex_lock(&mutex);
			
			// pthread_mutex_unlock(&mutex);
			break;
		case 1:
		//Đăng xuất
			timeout = 0;
			Account *temp = list;
			while (temp!=NULL)
			{
				if(strcmp(temp->name, logging[session].name) == 0){
					temp->login_status = '0';
					temp->wrong_password_count = 0;
					break;
				}
			}
			writeDataToFile();
			logging[session].ELO = 0;
			memset(logging[session].name, '\0', STR_SIZE);
			memset(logging[session].password, '\0', STR_SIZE);
			logging[session].login_status = '0';
			pthread_mutex_lock(&threads_mutex);
			bytes_sent = send(logging[session].conn_sock, "MSGS01#0", BUFF_SIZE - 1, 0);
			pthread_mutex_unlock(&threads_mutex);
			logging[session].conn_sock = 0;
			return;
			break;
		case 2:
		//Đăng ký
			timeout = 0;
			for(int i=7; i<strlen(buff); i++){
				if(buff[i] == '#'){
					i++;
					int k = i;
					username[i-7] = '\0';
					for(;i<strlen(buff);i++){
						if(buff[i] == '\n')	break;
						password[i-k] = buff[i];
					}
					break;
				}
				username[i-7] = buff[i];
			}
			int check = accountExisted(username);
			if(check == 1){
				pthread_mutex_lock(&mutex);
				bytes_sent = send(logging[session].conn_sock, "MSGS03#0", BUFF_SIZE - 1, 0);
				pthread_mutex_unlock(&mutex);
				break;
			}
			addNewAccount(username, password, '1', '0', 0, 0);
			writeDataToFile();
			pthread_mutex_lock(&threads_mutex);
			bytes_sent = send(logging[session].conn_sock, "MSGS03#1", BUFF_SIZE - 1, 0);
			pthread_mutex_unlock(&threads_mutex);
			break;
		case 3:
		//Thách đấu
			timeout = 0;
			char oppName[STR_SIZE];
			memset(oppName, '\0', STR_SIZE);
			for(int i = 7; i<strlen(buff); i++){
				oppName[i-7] = buff[i];
			}
			temp = list;
			while (temp!=NULL)
			{
				if(strcmp(temp->name, oppName) == 0){
					temp->login_status = '2';
				}
				temp = temp->next;
			}
			writeDataToFile();
			int session1;
			for(int i = 0; i<BACKLOG; i++){
				if(strcmp(oppName, logging[i].name) == 0){
					session1 = i;
					break;
				}
			}
			// gameStart(session, session1);
			break;
		case 4:
		//Trả lời thách đấu
			timeout = 0;
			break;
		case 5:
		//Đầu hàng
			timeout = 0;
			int session2;
			for(int i = 0; i<BACKLOG; i++){
				if(li[i].player1Session == session){
					session2 = li[i].player2Session;
					temp = list;
					while (temp!= NULL)
					{
						if(strcmp(temp->name, logging[session].name) == 0){
							temp->ELO= (0>(temp->ELO - 10)) ? 0 : temp->ELO - 10;//max(0, temp->ELO - 10);	
							logging[session].ELO = temp->ELO;
							temp->login_status = '1';
						}
						if(strcmp(temp->name, logging[session2].name) == 0){
							temp->ELO = temp->ELO + 10;
							logging[session2].ELO = temp->ELO;
							temp->login_status = '1';
						}
						temp = temp->next;
					}
				}
				else if(li[i].player2Session == session){
					session2 = li[i].player1Session;
					temp = list;
					while (temp!= NULL)
					{
						if(strcmp(temp->name, logging[session].name) == 0){
							temp->ELO= (0>(temp->ELO - 10)) ? 0 : temp->ELO - 10;
							logging[session].ELO = temp->ELO;
							temp->login_status = '1';
						}
						if(strcmp(temp->name, logging[session2].name) == 0){
							temp->ELO = temp->ELO +10;
							logging[session2].ELO = temp->ELO;
							temp->login_status = '1';
						}
						temp = temp->next;
					}
				}
			}
			writeDataToFile();
			printf("OPP: %s\n", logging[session2].name);
			pthread_mutex_lock(&threads_mutex);
			memset(buff, 0, STR_SIZE);
			sprintf(buff, "MSGS08#%d#", logging[session].ELO);
			bytes_sent = send(logging[session].conn_sock, buff, BUFF_SIZE - 1, 0);
			memset(buff, 0, STR_SIZE);
			sprintf(buff, "MSGS08#%d#", logging[session2].ELO);
			bytes_sent = send(logging[session2].conn_sock, buff, BUFF_SIZE - 1, 0);			
			pthread_mutex_unlock(&threads_mutex);
			break;
		case 6:
		//Tìm trận
			timeout = 0;
			temp = list;
			char c = '3';
			session2 = -1;
			char xname[STR_SIZE];
			while (temp!=NULL)
			{
				if(temp->login_status == '3' && abs(logging[session].ELO - temp->ELO) <50){
					temp->login_status = '2';
					c = '2';
					strcpy(xname, temp->name);
					break;
				}
				temp = temp->next;
			}
			if(c == '2'){
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
			printf("player session: %d, %d?\n",session, session2);

			if(session2 != -1){
				gameStart(session, session2);
			}
			break;
		case 7:
		//Tìm người rảnh
			timeout = 0;
			memset(buff, '\0', BUFF_SIZE);
			temp = list;
			strcpy(buff, "MSGS13#");
			char tempC[STR_SIZE];
			while (temp!=NULL)
			{
				if(temp->login_status == '1' && strcmp(temp->name, logging[session].name)!=0){
					sprintf(tempC,"%s#%d#", temp->name, temp->ELO );
					strcat(buff, tempC);
				}
				temp = temp->next;
			}
			pthread_mutex_lock(&threads_mutex);
			bytes_sent = send(logging[session].conn_sock, buff, BUFF_SIZE - 1, 0);
			pthread_mutex_unlock(&threads_mutex);
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
			int tp;
			char Crow[STR_SIZE];
			char answer[STR_SIZE];
			int row;
			memset(Crow, 0, STR_SIZE);
			memset(answer, 0, STR_SIZE);
			for(int i = 7; i<strlen(buff); i++){
				if(buff[i] == '#'){
					i++;
					tp = i;
					break;
				}
				Crow[i-7] = buff[i];
			}
			row = atoi(Crow);
			for(int i = tp; i<strlen(buff); i++){
				if(buff[i] == '#'){
					i++;
					tp = i;
					break;
				}
				answer[i - tp] = buff[i];
			}
			for(int i = 0; i<BACKLOG; i++){
				if(li[i].player1Session == session && li[i].playerTurn == 0){
					printf("p1");
					if(row == 0){
						if(strcmp(li[i].quiz.colAnswer, answer) == 0 && li[i].quiz.colAnswered == 0){
							li[i].quiz.colAnswered = 1;
							li[i].player1Score+=300;
							replySubmit(i, 0, 0, true);
						}
						else{
							li[i].playerTurn = 1;
							replySubmit(i, 1, 0, false);
						}
					}else{
						if(strcmp(li[i].quiz.rowData[row-1].rowAnswer, answer) == 0 && li[i].quiz.rowData[row-1].rowAnswered == 0){
							li[i].quiz.rowData[row-1].rowAnswered = 1;
							li[i].player1Score+=100;
							replySubmit(i, 0, row, true);
						}
						else{
							li[i].playerTurn = 1;
							replySubmit(i, 1, row, false);
						}
					}
					break;
				}
				if(li[i].player2Session == session && li[i].playerTurn == 1){
					if(row == 0){
						if(strcmp(li[i].quiz.colAnswer, answer) == 0 && li[i].quiz.colAnswered == 0){
							li[i].quiz.colAnswered = 0;
							li[i].player2Score+=300;
							replySubmit(i, 1, 0, true);
						}
						else{
							li[i].playerTurn = 0;
							replySubmit(i, 0, 0, false);
						}
					}else{
						if(strcmp(li[i].quiz.rowData[row-1].rowAnswer, answer) == 0 && li[i].quiz.rowData[row-1].rowAnswered == 0){
							li[i].quiz.rowData[row-1].rowAnswered = 1;
							li[i].player2Score+=100;
							replySubmit(i, 1, row, true);
						}
						else{
							li[i].playerTurn = 0;
							replySubmit(i, 0, row, false);
						}
					}
					break;
				}
			}

			break;
		case 11:
			timeout = 0;
			break;
		
		default:
			return;
			timeout++;
			if(timeout>10){
				logging[session].conn_sock = 0;
				
			}
			break;
		}
	}
}
int main(int argc, char *argv[])
{
	for(int i = 0; i<BACKLOG; i++)	logging[i].conn_sock = 0;
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
				if(logging[i].conn_sock == 0){
					logging[i].conn_sock = conn_sock;
					printf("%d\n",logging[i].conn_sock);
					pthread_create(&client_threads[i], NULL, &handle_client, (void *)&i);
					break;
				}
				printf("not here");
			}
			// pthread_create(&client_thread, NULL, &handle_client, (void *)conn_sock);
		}
	}
	close(listen_sock);
}