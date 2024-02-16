#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFF_SIZE 2048
#define BACKLOG 10
#define STR_SIZE 100
#define SENTENCE_SIZE 900

#define ACCOUNT_LOCKED 0
#define ACCOUNT_ACTIVE 1
#define ACCOUNT_LOGGED_IN 2
#define MAX_WRONG_PASSWORD_ATTEMPTS 3

int PORT; /* Port that will be opened */
char *MSGC[] = {"MSGC01", "MSGC02", "MSGC03", "MSGC04", "MSGC05", "MSGC06", "MSGC07", "MSGC08", "MSGC09", "MSGC10", "MSGC11", "MSGC12", "MSGS13", "MSGS14"};
char *MSGS[] = {"MSGS01", "MSGS02", "MSGS03", "MSGS04", "MSGS05", "MSGS06", "MSGS07", "MSGS08", "MSGS09", "MSGS10", "MSGS11", "MSGS12", "MSGS13", "MSGS14"};
time_t currentTime;
int bytes_sent, bytes_received;
int currentRoomID;

pthread_t client_threads[BACKLOG];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t threads_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct{
	char name[STR_SIZE];
	char password[STR_SIZE];
	int status;
	int login_status;  // 0: not here, 1: in active, 2: busy, 3: finding match
	int wrong_password_count;
    int elo;
	struct Account *next;
} Account;

typedef struct{
	int conn_sock;
	char name[STR_SIZE];
	char password[STR_SIZE];
	int elo;
    char send_buff[BUFF_SIZE];
    char recv_buff[BUFF_SIZE];
} Logging;

typedef struct{
	int spacing;    //cách dòng
	char rowAnswer[STR_SIZE];
	char rowHints[SENTENCE_SIZE];
	int rowAnswered; // 0: chưa đc trả lời,1 đã được trả lời
} Row;

typedef struct{
	int row;    //số dòng
	int collum; //cách dòng của cột đặc biệt
	int colAnswered; // 0: chưa đc trả lời,1 đã được trả lời
	char colAnswer[STR_SIZE];
	char colHints[SENTENCE_SIZE];
	Row rowData[STR_SIZE];
} Quiz;

typedef struct{
	int roomStatus; // 0: phòng trống, 1: phòng bận
	int roomID;
	int player1Session;
	int player2Session;
	int playerTurn; // 0: player1, 1: player2
	int player1Score;
	int player2Score;
	Quiz quiz;
} ChallangeData;

Account *list = NULL;
Logging logging[BACKLOG];
ChallangeData li[BACKLOG];

void writeLogGame(int ){

}
void writeDatatoLog(char username[]){
	pthread_mutex_lock(&file_mutex);
	char file_direc[STR_SIZE];
	sprintf(file_direc, "Account/%s", username);
	FILE *fi= fopen(file_direc, "a+");

	fclose(fi);
	pthread_mutex_unlock(&file_mutex);
}
void clearThreadData(int session){
	logging[session].conn_sock = 0;
}
void addNewAccount(char name[], char password[], int status, int login_status, int wrong_password_count, int elo){
	if (list == NULL)
	{
		list = (Account *)malloc(sizeof(Account));
		list->next = NULL;
		strcpy(list->name, name);
		strcpy(list->password, password);
		list->wrong_password_count = wrong_password_count;
		list->status = status;
		list->login_status = login_status;
		list->elo = elo; // Add the ELO assignment
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
	temp->elo = elo; // Add the ELO assignment
	temp->next = NULL;
	return;
}
void readDataFromFile(){
	list = NULL;
	FILE *f = fopen("account.txt", "r");
	char name[STR_SIZE];
	char password[STR_SIZE];
	int status;
	int login_status;
	int wrong_password_count;
	int elo; // New variable for ELO

	if (f == NULL)
		return;

	while (fscanf(f, "%s %s %d %d %d %d", name, password, &status, &login_status, &wrong_password_count, &elo) == 6)
	{
		addNewAccount(name, password, status, login_status, wrong_password_count, elo);
	}

	fclose(f);
}
void writeDataToFile(){
	pthread_mutex_lock(&file_mutex);
	FILE *f = fopen("account.txt", "w");

	if (f == NULL)
	{
		return;
	}

	Account *temp = list;
	fprintf(f, "%s %s %d %d %d %d\n", temp->name, temp->password, temp->status, temp->login_status, temp->wrong_password_count, temp->elo);
	while (temp->next != NULL)
	{
		temp = temp->next;
		fprintf(f, "%s %s %d %d %d %d\n", temp->name, temp->password, temp->status, temp->login_status, temp->wrong_password_count, temp->elo);
	}

	fclose(f);
	pthread_mutex_unlock(&file_mutex);
}
int login(char username[], char password[]){
	readDataFromFile();
	Account *temp = list;
	while (temp != NULL) {
        if (strcmp(temp->name, username) == 0) {
            if (strcmp(temp->password, password) == 0) {
                if (temp->status == 0) {
                    return 0; // Account is locked
                }
                if (temp->login_status >= 1) {
                    return 2; // Account is already logged in
                }
                temp->login_status = 1;
                writeDataToFile();
                return 1; // Login successful
            } else {
                if (temp->wrong_password_count < 2) {
                    temp->wrong_password_count++;
                    writeDataToFile();
                    return 3; // Incorrect password
                } else {
                    temp->status = 0;
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
int signup(char username[], char password[]){
	//account exist?
    if (list == NULL) {
        return 0; // If the list is empty, no account can exist
    }
	Account *temp = list;
	while (temp != NULL) {
        if (strcmp(temp->name, username) == 0) {
            return 1;
        }
        temp = temp->next;
    }
	return 0;
}
void signout(char username[]){
	Account *temp = list;
	while (temp != NULL){
		if(strcmp(temp->name, username) == 0){
			temp->login_status = 0;
			break;
		}
		temp = temp->next;
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

	memset(logging[session1].send_buff, '\0', BUFF_SIZE);
	sprintf(logging[session1].send_buff, "MSGS04#%s#%s#%d#%d#%s#", logging[session1].name, logging[session2].name, row, li[roomSession].quiz.collum, li[roomSession].quiz.colHints);
	bytes_sent = send(logging[session1].conn_sock, logging[session1].send_buff, BUFF_SIZE, 0);
	bytes_sent = send(logging[session2].conn_sock, logging[session1].send_buff, BUFF_SIZE, 0);
	for(int i = 0; i<row; i++){
		memset(logging[session1].send_buff, '\0', BUFF_SIZE);
		sprintf(logging[session1].send_buff, "MSGS12#%d#%d#%d#%s#", i, li[roomSession].quiz.rowData[i].spacing, strlen(li[roomSession].quiz.rowData[i].rowAnswer), li[roomSession].quiz.rowData[i].rowHints);
		bytes_sent = send(logging[session1].conn_sock, logging[session1].send_buff, BUFF_SIZE, 0);
		bytes_sent = send(logging[session2].conn_sock, logging[session1].send_buff, BUFF_SIZE, 0);
	}
}





















