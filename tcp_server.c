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
// #include <gtk/gtk.h>

#define BUFF_SIZE 2047
#define BACKLOG 2

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int PORT; /* Port that will be opened */
// char buff[BUFF_SIZE], buffTemp[BUFF_SIZE];
int thread_status[BACKLOG] = {0};
char *MSGC[] = {"MSGC01", "MSGC02", "MSGC03", "MSGC04", "MSGC05", "MSGC06", "MSGC07", "MSGC08", "MSGC09", "MSGC10", "MSGC11", "MSGC12"};
char *MSGS[] = {"MSGS01", "MSGS02", "MSGS03", "MSGS04", "MSGS05", "MSGS06", "MSGS07", "MSGS08", "MSGS09", "MSGS10", "MSGS11", "MSGS12"};

typedef struct
{
	int session;	//logging[session]
	int conn_sock;
	char name[1000];
	char password[1000];
	char status;	   // 0: blocked, 1: free
	char login_status; // 0: not here, 1: in active, 2: busy, 3: finding
	int wrong_password_count;
	int ELO;
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

void gameStart(int roomSession)
{
	// Lay Quiz
	char buff[BUFF_SIZE - 1];
	int bytes_sent, bytes_received;
	FILE *f = fopen("quiz.txt", "r");
	int row = 1;
	int space;
	char answer[100];
	char hints[100];
	fscanf(f, "%d %s", &space, answer);
	fscanf(f, " %99[^\n]", hints);
	li[roomSession]->quiz.collum = space;
	strcpy(li[roomSession]->quiz.colAnswer, answer);
	strcpy(li[roomSession]->quiz.colHints, hints);
	while (fscanf(f, "%d %s", &space, answer) == 2)
	{
		fscanf(f, " %99[^\n]", hints);
		li[roomSession]->quiz.rowData[row].spacing = space;
		strcpy(li[roomSession]->quiz.rowData[row].rowAnswer, answer);
		strcpy(li[roomSession]->quiz.rowData[row].rowHints, hints);
		row++;
	}
	// Khởi tạo trận đấu
	memset(buff, '\0', sizeof(buff));
	strcpy(buff, "MSGS04#");
	int player1Session = li[roomSession]->player1Session;
	// char player1Name = logging[li[roomSession]->player1Session]->name;
	strcat(buff, logging[player1Session]->name);
	strcat(buff, "#");
	int player2Session = li[roomSession]->player2Session;
	// char player2Name = logging[li[roomSession]->player2Session]->name;
	strcat(buff, logging[player2Session]->name);
	strcat(buff, "#");
	char tempStr[BUFF_SIZE - 1];
	snprintf(tempStr, BUFF_SIZE - 1, "%d#%d#%d#", roomSession, li[roomSession]->quiz.row, li[roomSession]->quiz.collum);
	strcat(buff, tempStr);
	strcat(buff, li[roomSession]->quiz.colHints);
	pthread_mutex_lock(&mutex);
	bytes_sent = send(logging[player1Session]->conn_sock, buff, BUFF_SIZE - 1, 0);
	bytes_sent = send(logging[player2Session]->conn_sock, buff, BUFF_SIZE - 1, 0);
	pthread_mutex_unlock(&mutex);
	for (int i = 1; i <= row; i++)
	{
		memset(buff, '\0', sizeof(buff));
		strcpy(buff, "MSGS12#");
		snprintf(tempStr, BUFF_SIZE - 1, "%d#%d#%d#", roomSession, row, li[roomSession]->quiz.rowData[i].spacing);
		strcat(buff, tempStr);
		strcat(buff, li[roomSession]->quiz.rowData[i].rowAnswer);
		strcat(buff, "#");
		strcat(buff, li[roomSession]->quiz.rowData[i].rowHints);
		pthread_mutex_lock(&mutex);
		bytes_sent = send(logging[player1Session]->conn_sock, buff, BUFF_SIZE - 1, 0);
		bytes_sent = send(logging[player2Session]->conn_sock, buff, BUFF_SIZE - 1, 0);
		pthread_mutex_unlock(&mutex);
	}
}

void clearChallangeData(int session)
{
	li[session]->roomStatus = 0;
	li[session]->player1Session = -1;
	li[session]->player2Session = -1;
	li[session]->player1Score = 0;
	li[session]->player2Score = 0;
	li[session]->playerTurn = 0;
	li[session]->quiz.row = 0;
	li[session]->quiz.collum = 0;
	li[session]->quiz.colAnswered = 0;
	strcpy(li[session]->quiz.colAnswer, "");
	strcpy(li[session]->quiz.colHints, "");
	for (int i = 0; i < 20; i++)
	{
		li[session]->quiz.rowData[i].spacing = 0;
		li[session]->quiz.rowData[i].rowAnswered = 0;
		strcpy(li[session]->quiz.rowData[i].rowAnswer, "");
		strcpy(li[session]->quiz.rowData[i].rowHints, "");
	}
}

void addNewAccount(char name[], char password[], char status, char login_status, int wrong_password_count, int ELO)
{
    Account *newAccount = (Account *)malloc(sizeof(Account));
    if (newAccount == NULL) {
        // Xử lý lỗi khi cấp phát bộ nhớ không thành công
        return;
    }
    newAccount->next = NULL;
    strcpy(newAccount->name, name);
    strcpy(newAccount->password, password);
    newAccount->wrong_password_count = wrong_password_count;
    newAccount->status = status;
    newAccount->login_status = login_status;
	newAccount->ELO=ELO;
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

int checkAccountStatus(char name[])
{
	Account *temp = list;
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

int AccountExisted(char name[])
{
	Account *temp = list;
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
	Account *temp = list;
	fprintf(f, "%s %s %c %c %d %d\n", temp->name, temp->password, temp->status, temp->login_status, temp->wrong_password_count, temp->ELO);
	while (temp->next != NULL)
	{
		temp = temp->next;
		fprintf(f, "%s %s %c %c %d %d\n", temp->name, temp->password, temp->status, temp->login_status, temp->wrong_password_count, temp->ELO);
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
	int ELO;
	if (f == NULL)
		return;
	while (fscanf(f, "%s %s %c %c %d %d", name, password, &status, &login_status, &wrong_password_count, &ELO) == 6)
	{
		addNewAccount(name, password, status, login_status, wrong_password_count, ELO);
	}
	fclose(f);
}

int signIn(char name[], char password[]) {
    readDataFromFile();
	Account *tempx = list;
    while (tempx != NULL)
	{
		printf("%s %s %d %d %d %d\n", tempx->name, tempx->password, tempx->status, tempx->login_status, tempx->wrong_password_count, tempx->ELO);
		tempx = tempx ->next;
	}
    Account *temp = list;
    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) {
            if (strcmp(temp->password, password) == 0) {
                if (temp->status == '0') {
                    return 0; // Tài khoản bị khóa
                }
                if (temp->login_status == '1') {
                    return 2; // Tài khoản đã đăng nhập từ trước
                }
                temp->login_status = '1';
                writeDataToFile();
                return 1; // Đăng nhập thành công
            } else {
                if (temp->wrong_password_count < 2) {
                    temp->wrong_password_count++;
                    writeDataToFile();
                    return 3; // Sai mật khẩu
                } else {
                    temp->status = '0';
                    temp->wrong_password_count = 0;
                    writeDataToFile();
                    return 0; // Tài khoản bị khóa
                }
            }
        }
        temp = temp->next;
    }
	
    return 3; // Tài khoản không tồn tại
}

void signOut(char name[]) {
    Account *temp = list;

    if (temp == NULL) {
        printf("List is empty!"); // Handle the case where the list is empty
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

    printf("Name not found in the list!"); // Handle the case where name isn't found
}

void *sendChallengeMSG(void *arg)
{
	int bytes_sent, bytes_received;
	int roomSession = (int)arg;
	int player2_conn_sock = logging[li[roomSession]->player2Session]->conn_sock;
	int player1Session = li[roomSession]->player1Session;
	char player1Name[1000];
	memset(player1Name, '\0', (strlen(player1Name) + 1));
	strcpy(player1Name, logging[player1Session]->name);
	char buffT[BUFF_SIZE - 1];
	memset(buffT, '\0', (strlen(buffT) + 1));
	strcpy(buffT, MSGS[5]);
	buffT[strlen(buffT)] = '#';
	strcat(buffT, player1Name);
	pthread_mutex_lock(&mutex);
	bytes_sent = send(player2_conn_sock, buffT, BUFF_SIZE - 1, 0);
	pthread_mutex_unlock(&mutex);
	memset(buffT, '\0', (strlen(buffT) + 1));

	pthread_mutex_lock(&mutex);
	bytes_received = recv(player2_conn_sock, buffT, BUFF_SIZE - 1, 0);
	pthread_mutex_unlock(&mutex);

	char answerChallenge[100];
	for (int i = 0; i < strlen(buffT); i++)
	{
		if (buffT[i] == '#')
		{
			answerChallenge[i] = '\0';
			break;
		}
		answerChallenge[i] = buffT[i];
	}
	if (strcmp(answerChallenge, MSGC[4]) != 0)
		return 2;
	if (buffT[7] == '0')
		return 2;
	return 1;
}

int sendChallenge(int x, char opponentName[])
{ // 0: không tồn tại, 1: đối thủ đồng ý, 2: không đồng ý, 3: bận;
	int conn_sock = logging[x];
	int roomSession = -1;
	for (int i = 0; i < BACKLOG; i++)
	{
		if (strcpy(logging[x]->name, opponentName) == 0)
		{
			if (logging[x]->login_status != 1)
			{
				return 3;
			}
			else
			{
				for (int j = 0; j < BACKLOG; j++)
				{
					if (li[j]->roomStatus == 0)
					{
						roomSession = j;
						break;
					}
				}
				if (roomSession == -1)
					return 2;
				li[roomSession]->roomStatus = 1;
				li[roomSession]->player1Session = x;
				li[roomSession]->player2Session = i;
				break;
			}
		}
	}
	// đợi đồng ý;
	int accepted;
	pthread_t waitingAccept;
	pthread_create(&waitingAccept, NULL, &sendChallengeMSG, (int)roomSession);
	pthread_join(waitingAccept, accepted);
	return accepted;
}

void *handle_client(int x)
{
	char buffTemp[BUFF_SIZE - 1];
	char buff[BUFF_SIZE - 1];
	pthread_detach(pthread_self());
	int bytes_sent, bytes_received;
	int session = (int)logging[x]->session;
	thread_status[session] = 1;
	int conn_sock = logging[x]->conn_sock;
	readDataFromFile();
	char msgType[7];
	while (1)
	{
		memset(buff, '\0', (strlen(buff) + 1));
		memset(msgType, '\0', (strlen(msgType) + 1));
		fflush(stdin);
		pthread_mutex_lock(&mutex);
		bytes_received = recv(conn_sock, buff, BUFF_SIZE - 1, 0);
		pthread_mutex_unlock(&mutex);
		for (int i = 0; i < strlen(buff); i++)
		{
			if (buff[i] == '#')
			{
				printf("\n");
				break;
			}
			msgType[i] = buff[i];
		}
		int msgTypeInt = -1;
		// kiem tra loai thong diep
		for (int i = 0; i < sizeof(MSGC) / sizeof(MSGC[0]); i++)
		{
			if (strcmp(msgType, MSGC[i]) == 0)
			{
				msgTypeInt = i;
				break;
			}
		}
		strcpy(buffTemp, buff);
		printf("MsgType: %d\n", msgTypeInt);
		char username[1000];
		char password[1000];
		memset(username, '\0', sizeof(username));
		memset(password, '\0', sizeof(password));
		switch (msgTypeInt)
		{
		case 0:
			/* code */
			// MSGC01#hust#hust123
			for(int i=7; i<strlen(buffTemp); i++){
				if(buffTemp[i] == '#'){
					i++;
					int temp = i;
					for(;i<strlen(buffTemp);i++){
						if(buffTemp[i] == '\n')	break;
						password[i-temp] = buffTemp[i];
					}
					break;
				}
				username[i-7] = buffTemp[i];
			}
			int i;
			printf("%s?\n%s?", username, password);
			int sign =  signIn(username, password);
			printf("Sign: %d\n", sign);
			switch (sign)
			{
			case 0:
				/* code: tk bi khoa */
				pthread_mutex_lock(&mutex);
				bytes_sent = send(logging[x]->conn_sock, "MSGS01#0", BUFF_SIZE - 1, 0);
				pthread_mutex_unlock(&mutex);
				break;
			case 1:
				/* code: dang nhap thanh cong */
				writeDataToFile();
				strcpy(logging[x]->name, username);
				strcpy(logging[x]->password, password);
				logging[x]->login_status = 1;
				logging[x]->status = 1;
				Account *temp = list;
				while(temp!=NULL){
					if(strcmp(temp->name, username) == 0){
						logging[x]->ELO = temp->ELO;
						break;
					}
					temp = temp->next;
				}
				pthread_mutex_lock(&mutex);
				memset(buff, '\0', sizeof(buff));
				sprintf(buff, "MSGS01#1#%s#%d", logging[x]->name, logging[x]->ELO);
				bytes_sent = send(logging[x]->conn_sock, buff, BUFF_SIZE - 1, 0);
				pthread_mutex_unlock(&mutex);
				break;
			case 2:
				/* code: tk da duoc dang nhap o noi khac */
				pthread_mutex_lock(&mutex);
				bytes_sent = send(logging[x]->conn_sock, "MSGS01#2", BUFF_SIZE - 1, 0);
				pthread_mutex_unlock(&mutex);
				printf("Đã gửi\n");
				break;
			case 3:
				/* code: sai password*/
				pthread_mutex_lock(&mutex);
				bytes_sent = send(logging[x]->conn_sock, "MSGS01#3", BUFF_SIZE - 1, 0);
				pthread_mutex_unlock(&mutex);
				break;
			default:
				break;
			}
			break;
		case 1:
			/* code */
			for(int i=7; i<strlen(buffTemp); i++){
				if(buffTemp[i] == '\n')	break;
				username[i-7] = buffTemp[i]; 
			}
			signOut(username);
			pthread_mutex_lock(&mutex);
			bytes_sent = send(logging[x]->conn_sock, "MSGS03#1", BUFF_SIZE - 1, 0);
			pthread_mutex_unlock(&mutex);
			break;
		case 2:
			/* code */
			for(int i=7; i<strlen(buffTemp); i++){
				if(buffTemp[i] == '#'){
					i++;
					int k = i;
					username[i-7] = '\0';
					for(;i<strlen(buffTemp);i++){
						if(buffTemp[i] == '\n')	break;
						password[i-k] = buffTemp[i];
					}
					break;
				}
				username[i-7] = buffTemp[i];
			}
			addNewAccount(username, password, '1', '0', 0, 0);
			writeDataToFile();
			signIn(username, password);
			break;
		case 3:
			/* code */
			char opponentName[1000];
			for (int i = 7; i < strlen(buffTemp); i++)
			{
				if (buffTemp[i] == '\0')
					break;
				opponentName[i - 7] = buffTemp[i];
				opponentName[i - 6] = '\0';
			}
			int answered = sendChallenge(x, opponentName);
			if (answered == 1)
			{
				// Lời thách đấu được chấp nhận
				logging[x]->login_status = 2;
				pthread_mutex_lock(&mutex);
				bytes_sent = send(logging[x]->conn_sock, "MSGS07#1", BUFF_SIZE - 1, 0);
				pthread_mutex_unlock(&mutex);
				// Tim phong
				int roomSession;
				for (int i = 0; i < BACKLOG; i++)
				{
					if (li[i]->player1Session == x)
					{
						roomSession = i;
						break;
					}
				}
				// Bat dau tran dau
				gameStart(roomSession);
			}
			else
			{
				// lời thách đấu bị từ chối
				pthread_mutex_lock(&mutex);
				bytes_sent = send(logging[x]->conn_sock, "MSGS07#0", BUFF_SIZE - 1, 0);
				pthread_mutex_unlock(&mutex);
			}
			break;
		case 4:
			/* code */
			break;
		case 5:
			/* code */
			break;
		case 6:
			logging[x]->login_status = 3;
			int y = -1;
			for (int i = 0; i < BACKLOG; i++)
			{
				if (logging[i]->login_status == 3)
				{
					y = i;
					break;
				}
			}
			if (y == -1)
				break;
			int z = -1;
			for (int i = 0; i < BACKLOG; i++)
			{
				if (li[i]->roomStatus == 0)
				{
					li[i]->roomStatus = 1;
					li[i]->player1Session = x;
					li[i]->player2Session = y;
					gameStart(i);
					break;
				}
			}
			break;
		case 7:
			for (int i = 0; i < BACKLOG; i++)
			{
				if (logging[i]->login_status == 1)
				{
					memset(buff, '\0', sizeof(buff));
					strcpy(buff, "MSGS12#");
					strcat(buff, logging[i]->name);
					bytes_sent = send(logging[x]->conn_sock, buff, BUFF_SIZE - 1, 0);
					pthread_mutex_unlock(&mutex);
					break;
				}
			}
			break;
		case 8:
			memset(buff, '\0', sizeof(buff));
			strcpy(buff, "MSGS11#1");
			for (int i = 0; i < BACKLOG; i++)
			{
				if (li[i]->player1Session == x)
				{
					int player2Session = li[i]->player2Session;
					pthread_mutex_lock(&mutex);
					bytes_sent = send(logging[player2Session]->conn_sock, buff, BUFF_SIZE - 1, 0);
					pthread_mutex_unlock(&mutex);
					break;
				}
				else if (li[i]->player2Session == x)
				{
					int player1Session = li[i]->player1Session;
					pthread_mutex_lock(&mutex);
					bytes_sent = send(logging[player1Session]->conn_sock, buff, BUFF_SIZE - 1, 0);
					pthread_mutex_unlock(&mutex);
					break;
				}
				else
					continue;
			}
			break;
		case 9:
			/* code */
			break;
		case 10:
			/* code */
			int row, time = 0;
			char answer[100];
			sscanf(buff, "MSGC11#%d#%d#%s", &row, &time, answer);
			for (int i = 0; i < BACKLOG; i++)
			{
				if (li[i]->player1Session == x)
				{
					if (li[i]->playerTurn == 0)
					{
						// trong luot
					}
					else
					{
						// sai luot
					}
				}
				else if (li[i]->player2Session == x)
				{
					if (li[i]->playerTurn == 1)
					{
						// trong luot
					}
					else
					{
						// sai luot
					}
				}
				else
					continue;
			}
			if (row == 0)
			{
				// Tra loi cot
			}
			else if (row == -1)
			{
				// khong tra loi hoac sai turn
			}
			else
			{
				// Tra loi hang
			}
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
}

int main(int argc, char *argv[])
{
    for (int i = 0; i < BACKLOG; ++i) {
        logging[i] = (Account *)malloc(sizeof(Account));
        li[i] = (challangeData *)malloc(sizeof(challangeData));
        // Check for malloc failure
        if (!logging[i] || !li[i]) {
            // Handle allocation failure
            // Example: printf("Memory allocation failed");
            return 1; // Return an error code
        }
    }
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
	server.sin_port = htons(PORT);				/* Remember htons() from "Conversions" section? =) */
	server.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY puts your IP address automatically */
	bzero(&(server.sin_zero), 8);				/* zero the rest of the structure */

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
		if ((conn_sock = accept(listen_sock, (struct sockaddr *)&client, &sin_size)) == -1)
		{
			perror("\nError: ");
		}
		else
		{
			int x;
			for (int i = 0; i < BACKLOG; i++)
			{
				if (thread_status[i] == 0)
				{
					printf("You got a connection from %s\n", inet_ntoa(client.sin_addr));
					x = i;
					thread_status[i] = 1;
					break;
				}
			}
			logging[x]->conn_sock = conn_sock;
			pthread_create(&client_thread[x], NULL, &handle_client, (int)x);
		}
	}
	printf("di qua");
	for (int i = 0; i < BACKLOG; i++)
	{
		pthread_join(client_thread[i], NULL);
	}
	
	for (int i = 0; i < BACKLOG; ++i) {
        free(logging[i]);
        free(li[i]);
    }
	close(listen_sock);
}
