#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFF_SIZE 1024
#define BACKLOG 2
#define STR_SIZE 100
#define SENTENCE_SIZE 900

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
char *MSGC[] = {"MSGC01", "MSGC02", "MSGC03", "MSGC04", "MSGC05", "MSGC06", "MSGC07", "MSGC08", "MSGC09", "MSGC10", "MSGC11", "MSGC12"};
char *MSGS[] = {"MSGS01", "MSGS02", "MSGS03", "MSGS04", "MSGS05", "MSGS06", "MSGS07", "MSGS08", "MSGS09", "MSGS10", "MSGS11", "MSGS12"};

typedef struct
{
	char name[STR_SIZE];
	char password[STR_SIZE];
	char status;
	char login_status;
	int wrong_password_count;
    int ELO;
	struct Account *next;
} Account;

typedef struct
{
	int conn_sock;
	char name[STR_SIZE];
	char password[STR_SIZE];
	char status;	   // 0: blocked, 1: free
	char login_status; // 0: not here, 1: in active, 2: busy, 3: finding match
	int wrong_password_count;
	int ELO;
    char buff[BUFF_SIZE-1];
} Logging;

typedef struct
{
	int spacing;
	char rowAnswer[STR_SIZE];
	char rowHints[SENTENCE_SIZE];
	int rowAnswered; // 0: chưa đc trả lời,1 đã được trả lời
} Row;

typedef struct
{
	int row;
	int collum;
	int colAnswered; // 0: chưa đc trả lời,1 đã được trả lời
	char colAnswer[50];
	char colHints[1000];
	Row rowData[20];
} Quiz;

typedef struct
{
	int roomStatus; // 0: phòng trống, 1: phòng bận
	int player1Session;
	int player2Session;
	int playerTurn; // 0: player1, 1: player2
	int player1Score;
	int player2Score;
	Quiz quiz;
} challangeData;
