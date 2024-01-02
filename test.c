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

int PORT; /* Port that will be opened */
// char buff[BUFF_SIZE], buffTemp[BUFF_SIZE];
int thread_status[BACKLOG] = {0};

typedef struct
{
	int session;
	int conn_sock;
	char name[1000];
	char password[1000];
	int status;	   // blocked or not
	int login_status; // 0: not here, 1: in active, 2: busy, 3: finding
	int wrong_password_count;
	struct Account *next;
} Account;

typedef struct
{
	int spacing;
	char rowAnswer[50];
	char rowHints[1000];
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

Account *list = NULL;
Account *logging[BACKLOG];
challangeData *li[BACKLOG];

void addNewAccount(char name[], char password[], char status, char login_status, int wrong_password_count) {
    Account *newAccount = (Account *)malloc(sizeof(Account));
    if (newAccount == NULL) {
        // Xử lý lỗi khi cấp phát bộ nhớ không thành công
        return;
    }
    
    strncpy(newAccount->name, name, sizeof(newAccount->name) - 1);
    newAccount->name[sizeof(newAccount->name) - 1] = '\0'; // Đảm bảo kết thúc chuỗi
    
    strncpy(newAccount->password, password, sizeof(newAccount->password) - 1);
    newAccount->password[sizeof(newAccount->password) - 1] = '\0'; // Đảm bảo kết thúc chuỗi
    
    newAccount->wrong_password_count = wrong_password_count;
    newAccount->status = status;
    newAccount->login_status = login_status;
    newAccount->next = NULL;
    
    if (list == NULL) {
        list = newAccount;
    } else {
        Account *temp = list;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newAccount;
    }
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
int main(){
	char buff[9] = "number47";
	switch (buff[0])
	{
	case 'n':
		printf("???");
		break;
	
	default:
		break;
	}
}