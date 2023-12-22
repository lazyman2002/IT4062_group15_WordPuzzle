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
#include <gtk/gtk.h>

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
	struct Account *next;
} Account;

typedef struct 
{
	int spacing;
	char RAnswer[50];
	char rowHints[1000];
} Row;


typedef struct 
{
	int row;	int collum;
	int rowAnswered; //0: chưa đc trả lời, đã được trả lời
	char CAnswer[50];
	char colHints[1000];
	Row rowData[20];
} Quiz;

typedef struct{
	int roomStatus;	//0: phòng trống, 1: phòng bận
	int player1Session;
	int player2Session;
	int playerTurn; //0: player1, 1: player2
	int player1Score;
	int player2Score;
	Quiz quiz;
} challangeData;

Account *list = NULL;
Account *logging[BACKLOG];
challangeData *li[BACKLOG];

void clearChallangeData(int session){
	li[session]->roomStatus = 0;
	li[session]->player1Session = -1;
	li[session]->player2Session = -1;
	li[session]->player1Score = 0;
	li[session]->player2Score = 0;
	li[session]->playerTurn=0;
	li[session]->quiz.row = 0;
	li[session]->quiz.collum = 0;
	li[session]->quiz.rowAnswered = 0;
	strcpy(li[session]->quiz.CAnswer, "");
	strcpy(li[session]->quiz.colHints, "");
}

int main(){
	char buff[100];
	strcpy(buff, "MSGC11#10#doan");
	int spacing;
	char answer[100];
	sscanf(buff, "MSGC11#%d#%s", &spacing, answer);
	printf("%d\n%s\n", spacing, answer);
}
