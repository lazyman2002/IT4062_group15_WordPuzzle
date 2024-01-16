/*UDP Echo Client*/
#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include <gtk/gtk.h>
#define BUFF_SIZE 1024
#define BACKLOG 2
#define STR_SIZE 100
#define SENTENCE_SIZE 900
pthread_mutex_t socket_mutex = PTHREAD_MUTEX_INITIALIZER;
// pthread_mutex_lock(&socket_mutex);
// pthread_mutex_unlock(&socket_mutex);

int SERV_PORT;
char SERV_IP[100];
int client_sock;
char buff[BUFF_SIZE];
struct sockaddr_in server_addr;
int bytes_sent, bytes_received, sin_size;
int isChallenged = 0;
int spacing[STR_SIZE];
int leng[STR_SIZE];
typedef struct
{
	int conn_sock;
	char name[STR_SIZE];
	char password[STR_SIZE];
	// char status;	   // 0: blocked, 1: free
	char login_status; // 0: not here, 1: in active, 2: busy, 3: finding match
	// int wrong_password_count;
	int ELO;
  char buff[BUFF_SIZE-1];
} Logging;
Logging account;

Logging account; 
GtkBuilder *builderLogin;
GtkWidget *WindowLogin;
GtkBuilder *builderSignup;
GtkWidget *WindowSignup;
GtkBuilder *builderHome;
GtkWidget *WindowHome;
GtkBuilder *builderChallengePopup;
GtkWidget *WindowChallengePopup;
GtkBuilder *builderPlayscreen;
GtkWidget *WindowPlayscreen;
GtkBuilder *builderInActive;
GtkWidget *WindowInActive;

GtkWidget *usernameLabel;
GtkWidget *passwordLabel;
GtkWidget *resignPasswordLabel;
GtkWidget *usernameEntry;
GtkWidget *passwordEntry;
GtkWidget *resignPasswordEntry;
GtkWidget *loginButton;
GtkWidget *signupButton;
GtkWidget *notifyLabel;
GtkWidget *usernameData;
GtkWidget *ELOData;
GtkWidget *findGameButton;
GtkWidget *findActiveButton;
GtkWidget *logoutButton;
GtkWidget *P1Score;
GtkWidget *P2Score;
GtkWidget *playerTurn;
GtkWidget *gridTable;
GtkWidget *hintsText;
GtkWidget *rowEntry;
GtkWidget *answerEntry;
GtkWidget *timeoutButton;
GtkWidget *surrenderButton;
GtkWidget *submitButton;
GtkWidget *labels[10][STR_SIZE];
GtkWidget *playerName1;
GtkWidget *playerName2;
GtkWidget *playerName3;

void setupLogin();
void setupHome();
void setupSignup();
void setupPlayscreen();
void on_timeoutButton_clicked(GtkButton *button, gpointer user_data){}
void on_surrenderButton_clicked(GtkButton *button, gpointer user_data){
  pthread_mutex_lock(&socket_mutex);
  memset(buff, 0, STR_SIZE);
  sprintf(buff, "MSGC06#");
  bytes_sent = send(account.conn_sock, buff, BUFF_SIZE-1, 0);
  pthread_mutex_unlock(&socket_mutex);
}
void on_submitButton_clicked(GtkButton *button, gpointer user_data){
  char Crow[STR_SIZE];
  char answer[STR_SIZE];
  strcpy(Crow, gtk_entry_get_text((rowEntry)));
  strcpy(answer, gtk_entry_get_text((answerEntry)));
  pthread_mutex_lock(&socket_mutex);
  memset(buff, '\0', BUFF_SIZE-1);
  sprintf(buff, "MSGC11#%s#%s#", Crow, answer);
  bytes_sent = send(account.conn_sock, buff, BUFF_SIZE-1, 0);
  pthread_mutex_unlock(&socket_mutex);
}
void *myThreadFunction(void *arg) {
    // Thực hiện các công việc của thread ở đây
  while (1)
  {
    memset(buff, '\0', BUFF_SIZE-1);
    bytes_received = recv(account.conn_sock, buff, BUFF_SIZE - 1, 0);
    printf("buff: %s\n", buff);
    if(buff[5] == '8'){
      char newELO[STR_SIZE];
      memset(newELO, '\0', STR_SIZE);
      for(int i = 7; i<strlen(buff); i++){
        if(buff[i] == '#'){
          break;
        }
        newELO[i-7] = buff[i]; 
      }
      printf("new: %s\n", newELO);
      strcat(newELO, " ");
      gtk_widget_hide(WindowPlayscreen);
      setupHome();
      gtk_label_set_text(GTK_LABEL(ELOData), newELO);
      gtk_widget_show(WindowHome);
      return;
    }
    else if(buff[5] == '5'){
      int temp;
      char Score1[STR_SIZE];
      char Score2[STR_SIZE];
      char Row[STR_SIZE];
      char turn;
      int score1, score2, row;
      memset(Row, '\0', STR_SIZE);
      for(int i = 7; i< strlen(buff); i++){
        if(buff[i] == '#'){
          i++;
					temp = i;
					break;
        }
        Score1[i-7] = buff[i];
      }
      score1 = atoi(Score1);
      for(int i = temp; i< strlen(buff); i++){
        if(buff[i] == '#'){
          i++;
					temp = i;
					break;
        }
        Score2[i-temp] = buff[i];
      }
      score2 = atoi(Score2);
      turn = buff[temp];
      temp= temp+2;
      for(int i = temp; i< strlen(buff); i++){
        if(buff[i] == '#'){
          i++;
					temp = i;
					break;
        }
        Row[i-temp] = buff[i];
      }
      row = atoi(Row);
      if(row == -1){
        // gtk_label_set_text(GTK_LABEL(P1Score), Score1);
        // gtk_label_set_text(GTK_LABEL(P2Score), Score2);
        // gtk_label_set_text(GTK_LABEL(playerTurn), turn);
        continue;
      }
      else if(row >= 0){
        char answer[STR_SIZE];
        memset(answer, 0, STR_SIZE);
        for(int i = temp; i< strlen(buff); i++){
          if(buff[i] == '#'){
            i++;
            temp = i;
            break;
          }
          answer[i-temp] = buff[i];
        }
        printf("answer: %s\n", answer);
        printf("%d, %d, %d\n",row, spacing[row], leng[row]);
        for(int i = spacing[row]; i < spacing[row] + leng[row]; i++){
          char characters[STR_SIZE];
          sprintf(characters, "%c", answer[i-spacing[row]]);
          gtk_label_set_text(GTK_LABEL(labels[row][i]), characters);
        }
        gtk_label_set_text(GTK_LABEL(P1Score), Score1);
        gtk_label_set_text(GTK_LABEL(P2Score), Score2);
        if(turn == '0')
          gtk_label_set_text(GTK_LABEL(playerTurn), "0");
        else
          gtk_label_set_text(GTK_LABEL(playerTurn), "1");

      }
    }
  }  
}
int containsNonAlphaNumeric(const char *str)
{
  for (int i = 0; str[i] != '\n'; i++)
  {
    if ((str[i] >= 'a' && str[i] <= 'z') || (str[i] >= 'A' && str[i] <= 'Z') || (str[i] >= '0' && str[i] <= '9'))
    {
      continue;
    }
    else
    {
      return 0;
    }
  }
  return 1;
}
void on_loginButton_clicked(GtkButton *button, gpointer user_data){
  char username[STR_SIZE], password[STR_SIZE];
  usernameEntry = GTK_WIDGET(gtk_builder_get_object(builderLogin, "usernameEntry"));
  passwordEntry = GTK_WIDGET(gtk_builder_get_object(builderLogin, "passwordEntry"));
  strcpy(username, gtk_entry_get_text(GTK_ENTRY(usernameEntry)));
  strcpy(password, gtk_entry_get_text(GTK_ENTRY(passwordEntry)));
  pthread_mutex_lock(&socket_mutex);
  memset(buff, '\0', BUFF_SIZE-1);
	sprintf(buff, "MSGC01#%s#%s", username, password);
  bytes_sent = send(account.conn_sock, buff, BUFF_SIZE-1, 0);
  if (bytes_sent < 0)
  {
    perror("Error: ");
    close(client_sock);
    return 0;
  }
  pthread_mutex_unlock(&socket_mutex);
  memset(buff, '\0', BUFF_SIZE-1);
  bytes_received = recv(account.conn_sock, buff, BUFF_SIZE - 1, 0);
  printf("Buff: %s\n", buff);
  char res = buff[7];
  switch (res)
  {
  case '0':
    gtk_label_set_text(GTK_LABEL(notifyLabel), "Tài khoản bị khóa");
    /* code */
    break;
  case '1':
    /* code */
    char cELO[STR_SIZE];
    memset(cELO, '\0', STR_SIZE-1);
    for(int i = 9;i<strlen(buff);i++){
      cELO[i-9] = buff[i]; 
    }
    account.ELO = atoi(cELO);
    strcpy(account.name , username);
    printf("Elo: %d\n", account.ELO);
    gtk_widget_hide(GTK_WIDGET(WindowLogin));
    setupHome();
    gtk_label_set_text(GTK_LABEL(usernameData), account.name);
    gtk_label_set_text(GTK_LABEL(ELOData), cELO);
    printf("Mở window Home\n");
    gtk_widget_show(GTK_WIDGET(WindowHome));
    printf("Mở thành công\n");
    break;
  case '2':
    /* code */
    gtk_label_set_text(GTK_LABEL(notifyLabel), "Tài khoản đã được đăng nhập ở nơi khác");
    break;
  default:
    gtk_label_set_text(GTK_LABEL(notifyLabel), "Sai tài khoản hoặc mật khẩu");
    break;
  }
  return;
}
void switch_to_signup(GtkButton *button, gpointer user_data) {
  gtk_widget_hide(GTK_WIDGET(WindowLogin)); // Hide the login window
  setupSignup();
  gtk_widget_show(GTK_WIDGET(WindowSignup)); // Show the signup window
}
void on_signupButton_clicked(GtkButton *button, gpointer user_data){
  char username[STR_SIZE], password[STR_SIZE], resignPassword[STR_SIZE];
  strcpy(username, gtk_entry_get_text(GTK_ENTRY(usernameEntry)));
  strcpy(password, gtk_entry_get_text(GTK_ENTRY(passwordEntry)));
  strcpy(resignPassword, gtk_entry_get_text(GTK_ENTRY(resignPasswordEntry)));
  if(strcmp(password, resignPassword) !=0){
    gtk_label_set_text(GTK_LABEL(notifyLabel), "Mật khẩu không trùng nhau");
    return;
  }
  pthread_mutex_lock(&socket_mutex);
  memset(buff, '\0', BUFF_SIZE-1);
  sprintf(buff, "MSGC03#%s#%s", username, password);
  bytes_sent = send(account.conn_sock, buff, BUFF_SIZE-1, 0);
  pthread_mutex_unlock(&socket_mutex);
  memset(buff, '\0', BUFF_SIZE-1);
  bytes_received = recv(account.conn_sock, buff, BUFF_SIZE - 1, 0);
  if(buff[7] == '0'){
    gtk_label_set_text(GTK_LABEL(notifyLabel), "Đăng ký thất bại");
  }
  else{
    gtk_label_set_text(GTK_LABEL(notifyLabel), "Đăng ký thành công");
  }
  printf("buff: %s\n", buff);
}
void switch_to_login(GtkButton *button, gpointer user_data){
  gtk_widget_hide(GTK_WIDGET(WindowSignup)); // Hide the signup window
  setupLogin();
  gtk_widget_show(GTK_WIDGET(WindowLogin)); // Show the login window
}
void findGame(GtkButton *button, gpointer user_data){
  char P1Name[STR_SIZE];
  char P2Name[STR_SIZE];
  int row, collum;
  char Row[10], Collum[10];
  char colHints[SENTENCE_SIZE];
  pthread_mutex_lock(&socket_mutex);
  memset(buff, '\0', BUFF_SIZE-1);
  sprintf(buff, "MSGC07#");
  bytes_sent = send(account.conn_sock, buff, BUFF_SIZE-1, 0);
  pthread_mutex_unlock(&socket_mutex);
  memset(buff, '\0', BUFF_SIZE-1);
  bytes_received = recv(account.conn_sock, buff, BUFF_SIZE - 1, 0);
  printf("buff:%s\n", buff);
  gtk_widget_hide(GTK_WIDGET(WindowHome));
  setupPlayscreen();
  gtk_widget_show(GTK_WIDGET(WindowPlayscreen));
  int i = 7, temp = i;
  memset(P1Name, '\0',STR_SIZE );
  for(; i<strlen(buff); i++){
    if(buff[i] == '#'){
      i++;
      temp = i;
      break;
    }
    P1Name[i-temp] = buff[i];
  }
  memset(P2Name, '\0',STR_SIZE );
  for(; i<strlen(buff); i++){
    if(buff[i] == '#'){
      i++;
      temp = i;
      break;
    }
    P2Name[i-temp] = buff[i];
  }
  memset(Row, '\0',10);
  for(; i<strlen(buff); i++){
    if(buff[i] == '#'){
      i++;
      temp = i;
      break;
    }
    Row[i-temp] = buff[i];
  }
  row = atoi(Row);
  memset(Collum, '\0',10);
  for(; i<strlen(buff); i++){
    if(buff[i] == '#'){
      i++;
      temp = i;
      break;
    }
    Collum[i-temp] = buff[i];
  }
  collum = atoi(Collum);
  memset(colHints, '\0',SENTENCE_SIZE );
  for(; i<strlen(buff); i++){
    if(buff[i] == '#'){
      i++;
      temp = i;
      break;
    }
    colHints[i-temp] = buff[i];
  }
  char rowHints[row][SENTENCE_SIZE];
  for (int i = 0; i < row; ++i) {
    memset(rowHints[i], '\0', SENTENCE_SIZE);
  }
  for(int i = 0; i<row; i++){
    memset(buff, '\0', STR_SIZE);
    bytes_received = recv(account.conn_sock, buff, BUFF_SIZE - 1, 0);
    printf("buff: %s\n", buff);
    char rowNumber[STR_SIZE], rowSpacing[STR_SIZE], rowLeng[STR_SIZE];
    int R, temp;
    int j = 7;
    for(; j<strlen(buff); j++){
      if(buff[j] == '#'){
        j++;
        temp = j;
        break;
      }
      rowNumber[j-7] = buff[j]; 
    }
    R = atoi(rowNumber);
    for(; j<strlen(buff); j++){
      if(buff[j] == '#'){
        j++;
        temp = j;
        break;
      }
      rowSpacing[j-temp] = buff[j];
    }
    spacing[R] = atoi(rowSpacing);
    for(; j<strlen(buff); j++){
      if(buff[j] == '#'){
        j++;
        temp = j;
        break;
      }
      rowLeng[j-temp] = buff[j];
    }
    leng[R] = atoi(rowLeng);
    for(; j<strlen(buff); j++){
      if(buff[j] == '#'){
        j++;
        temp = j;
        break;
      }
      rowHints[R][j-temp] = buff[j];
    }
  }
  // Gán câu hỏi
  for(int i = 0; i<row; i++){
    for(int j = 0; j<leng[i]+spacing[i];j++){
      if(j<spacing[i]){
				labels[i][j] = gtk_label_new(" ");
				gtk_grid_attach(GTK_GRID(gridTable), labels[i][j], j, i, 1, 1);
				gtk_widget_show(labels[i][j]);
				continue;
      }else{
				labels[i][j] = gtk_label_new("*");
				gtk_grid_attach(GTK_GRID(gridTable), labels[i][j], j, i, 1, 1);
				gtk_widget_show(labels[i][j]);
				continue;
      }
    }
  }
  //Gán gợi ý
  char hintBuff[SENTENCE_SIZE];
  strcpy(hintBuff, "0. ");
  strcat(hintBuff, colHints);
  for(int i = 0; i<row; i++  ){
    char tmp[STR_SIZE];
    memset(tmp, '\0', STR_SIZE);
    sprintf(tmp, "\n%d. %s", i+1, rowHints[i]);
    strcat(hintBuff, tmp);
  }
  GtkTextBuffer *TXTbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(hintsText));
  gtk_text_buffer_set_text(TXTbuffer, hintBuff, -1);
  pthread_t myThread;
  pthread_create(&myThread, NULL, myThreadFunction, NULL);
}
void send_challenge(GtkButton *button, gpointer user_data){
  int stt = GPOINTER_TO_INT(user_data);
  gtk_widget_hide(WindowInActive);
  if(stt == 1){
    gchar *text = gtk_label_get_text(GTK_LABEL(playerName1));
    pthread_mutex_lock(&socket_mutex);
    memset(buff, '\0', BUFF_SIZE-1);
    sprintf(buff, "MSGC04#%s", text);
    bytes_sent = send(account.conn_sock, buff, BUFF_SIZE-1, 0);
    pthread_mutex_unlock(&socket_mutex);
  }
  else if(stt ==2){
    gchar *text = gtk_label_get_text(GTK_LABEL(playerName2));
    pthread_mutex_lock(&socket_mutex);
    memset(buff, '\0', BUFF_SIZE-1);
    sprintf(buff, "MSGC04#%s", text);
    bytes_sent = send(account.conn_sock, buff, BUFF_SIZE-1, 0);
    pthread_mutex_unlock(&socket_mutex); 
  }else if(stt ==3){
    gchar *text = gtk_label_get_text(GTK_LABEL(playerName3));
    pthread_mutex_lock(&socket_mutex);
    memset(buff, '\0', BUFF_SIZE-1);
    sprintf(buff, "MSGC04#%s", text);
    bytes_sent = send(account.conn_sock, buff, BUFF_SIZE-1, 0);
    pthread_mutex_unlock(&socket_mutex);
  }
  setupPlayscreen();
  gtk_widget_show(WindowPlayscreen);
}
void findActive(GtkButton *button, gpointer user_data){
  gtk_widget_hide(GTK_WIDGET(WindowHome));
  WindowInActive = GTK_WIDGET(gtk_builder_get_object(builderInActive, "window"));
  playerName1 = GTK_WIDGET(gtk_builder_get_object(builderInActive, "playerName1"));
  GtkWidget *playerELO1 = GTK_WIDGET(gtk_builder_get_object(builderInActive, "playerELO1"));
  GtkWidget *playerButton1 = GTK_WIDGET(gtk_builder_get_object(builderInActive, "playerButton1"));
  int stt1 = 1;

  playerName2 = GTK_WIDGET(gtk_builder_get_object(builderInActive, "playerName2"));
  GtkWidget *playerELO2 = GTK_WIDGET(gtk_builder_get_object(builderInActive, "playerELO2"));
  GtkWidget *playerButton2 = GTK_WIDGET(gtk_builder_get_object(builderInActive, "playerButton2"));
  int stt2 = 2;

  playerName3 = GTK_WIDGET(gtk_builder_get_object(builderInActive, "playerName3"));
  GtkWidget *playerELO3 = GTK_WIDGET(gtk_builder_get_object(builderInActive, "playerELO3"));
  GtkWidget *playerButton3 = GTK_WIDGET(gtk_builder_get_object(builderInActive, "playerButton3"));
  int stt3 = 3;

  g_signal_connect(playerButton1, "clicked", G_CALLBACK(send_challenge), GINT_TO_POINTER(stt1));
  g_signal_connect(playerButton2, "clicked", G_CALLBACK(send_challenge), GINT_TO_POINTER(stt2));
  g_signal_connect(playerButton3, "clicked", G_CALLBACK(send_challenge), GINT_TO_POINTER(stt3));
  g_signal_connect(WindowInActive, "destroy", G_CALLBACK(gtk_main_quit), NULL);
  
  gtk_widget_show(GTK_WIDGET(WindowInActive));
  pthread_mutex_lock(&socket_mutex);
  memset(buff, '\0', BUFF_SIZE-1);
  sprintf(buff, "MSGC08#");
  bytes_sent = send(account.conn_sock, buff, BUFF_SIZE-1, 0);
  pthread_mutex_unlock(&socket_mutex);
  memset(buff, '\0', BUFF_SIZE-1);
  bytes_received = recv(account.conn_sock, buff, BUFF_SIZE - 1, 0);
  printf("%s\n", buff);
  char username[STR_SIZE];
  char elo[STR_SIZE];
  memset(username, '\0', STR_SIZE);
  memset(elo, '\0', STR_SIZE);
  int loc = 7;
  int token = 0;
  int stt =0;
  Logging inactive[BACKLOG];
  for(int i = 7;i<strlen(buff);i++){
    if(buff[i] == '#'){
      loc = i+1;
      if(token == 0){
        strcpy(inactive[stt].name, username);
        token = 1;
        memset(elo, '\0', STR_SIZE);
      }
      else{
        inactive[stt].ELO = atoi(elo);
        stt++;
        token = 0;
        memset(username, '\0', STR_SIZE);
      }
      continue;
    }
    if(token == 0){
      username[i - loc] = buff[i]; 
    }
    else{
      elo[i-loc] = buff[i];
    }
  }
  if(stt >= 1){
    gtk_label_set_text(GTK_LABEL(playerName1), inactive[0].name);
    sprintf(elo, "%d", inactive[0].ELO);
    gtk_label_set_text(GTK_LABEL(playerELO1), elo);
  }
  else if(stt >= 2){
    gtk_label_set_text(GTK_LABEL(playerName2), inactive[1].name);
    sprintf(elo, "%d", inactive[1].ELO);
    gtk_label_set_text(GTK_LABEL(playerELO2), elo);
  }
  else if(stt >= 3){
    gtk_label_set_text(GTK_LABEL(playerName3), inactive[2].name);
    sprintf(elo, "%d", inactive[2].ELO);
    gtk_label_set_text(GTK_LABEL(playerELO3), elo);
  }
}
void signout(GtkButton *button, gpointer user_data){
  account.login_status = '0';
  memset(account.name, '\0', STR_SIZE);
  memset(account.password, '\0', STR_SIZE);
  account.ELO = 0;
  pthread_mutex_lock(&socket_mutex);
  memset(buff, '\0', BUFF_SIZE-1);
  sprintf(buff, "MSGC02#");
  bytes_sent = send(account.conn_sock, buff, BUFF_SIZE-1, 0);
  pthread_mutex_unlock(&socket_mutex);
  account.conn_sock=0;
  exit(0);
}
void setupLogin(){
  WindowLogin = GTK_WIDGET(gtk_builder_get_object(builderLogin, "window"));
  usernameLabel = GTK_WIDGET(gtk_builder_get_object(builderLogin, "usernameLabel"));
  passwordLabel = GTK_WIDGET(gtk_builder_get_object(builderLogin, "passwordLabel"));
  usernameEntry = GTK_WIDGET(gtk_builder_get_object(builderLogin, "usernameEntry"));
  passwordEntry = GTK_WIDGET(gtk_builder_get_object(builderLogin, "passwordEntry"));
  notifyLabel = GTK_WIDGET(gtk_builder_get_object(builderLogin, "notifyLabel"));
  loginButton = GTK_WIDGET(gtk_builder_get_object(builderLogin, "loginButton"));
  signupButton = GTK_WIDGET(gtk_builder_get_object(builderLogin, "signupButton"));
  g_signal_connect(loginButton, "clicked", G_CALLBACK(on_loginButton_clicked), NULL);
  g_signal_connect(signupButton, "clicked", G_CALLBACK(switch_to_signup), NULL);
  g_signal_connect(WindowLogin, "destroy", G_CALLBACK(gtk_main_quit), NULL);
}
void setupHome(){
  WindowHome = GTK_WIDGET(gtk_builder_get_object(builderHome, "window"));
  usernameData = GTK_WIDGET(gtk_builder_get_object(builderHome, "usernameData"));
  ELOData = GTK_WIDGET(gtk_builder_get_object(builderHome, "ELOData"));
  findGameButton = GTK_WIDGET(gtk_builder_get_object(builderHome, "findGameButton"));
  findActiveButton = GTK_WIDGET(gtk_builder_get_object(builderHome, "findActiveButton"));
  logoutButton = GTK_WIDGET(gtk_builder_get_object(builderHome, "logoutButton"));
  g_signal_connect(findGameButton, "clicked", G_CALLBACK(findGame), NULL);
  g_signal_connect(findActiveButton, "clicked", G_CALLBACK(findActive), NULL);
  g_signal_connect(logoutButton, "clicked", G_CALLBACK(signout), NULL);
  g_signal_connect(WindowHome, "destroy", G_CALLBACK(gtk_main_quit), NULL);
}
void setupSignup(){
  WindowSignup = GTK_WIDGET(gtk_builder_get_object(builderSignup, "window"));
  usernameLabel = GTK_WIDGET(gtk_builder_get_object(builderSignup, "usernameLabel"));
  passwordLabel = GTK_WIDGET(gtk_builder_get_object(builderSignup, "passwordLabel"));
  resignPasswordLabel = GTK_WIDGET(gtk_builder_get_object(builderSignup, "resignPasswordLabel"));
  usernameEntry = GTK_WIDGET(gtk_builder_get_object(builderSignup, "usernameEntry"));
  passwordEntry = GTK_WIDGET(gtk_builder_get_object(builderSignup, "passwordEntry"));
  resignPasswordEntry = GTK_WIDGET(gtk_builder_get_object(builderSignup, "resignPasswordEntry"));
  notifyLabel = GTK_WIDGET(gtk_builder_get_object(builderSignup, "notifyLabel"));
  loginButton = GTK_WIDGET(gtk_builder_get_object(builderSignup, "loginButton"));
  signupButton = GTK_WIDGET(gtk_builder_get_object(builderSignup, "signupButton"));
  g_signal_connect(signupButton, "clicked", G_CALLBACK(on_signupButton_clicked), NULL);
  g_signal_connect(loginButton, "clicked", G_CALLBACK(switch_to_login), NULL);
  g_signal_connect(WindowLogin, "destroy", G_CALLBACK(gtk_main_quit), NULL);
}
void setupPlayscreen(){
  WindowPlayscreen = GTK_WIDGET(gtk_builder_get_object(builderPlayscreen, "window"));
	P1Score = GTK_WIDGET(gtk_builder_get_object(builderPlayscreen, "P1Score"));
	P2Score = GTK_WIDGET(gtk_builder_get_object(builderPlayscreen, "P2Score"));
	playerTurn = GTK_WIDGET(gtk_builder_get_object(builderPlayscreen, "playerTurn"));
	gridTable = GTK_WIDGET(gtk_builder_get_object(builderPlayscreen, "gridTable"));
	hintsText = GTK_WIDGET(gtk_builder_get_object(builderPlayscreen, "hintsText"));
	rowEntry = GTK_WIDGET(gtk_builder_get_object(builderPlayscreen, "rowEntry"));
	answerEntry = GTK_WIDGET(gtk_builder_get_object(builderPlayscreen, "answerEntry"));
	timeoutButton = GTK_WIDGET(gtk_builder_get_object(builderPlayscreen, "timeoutButton"));
	surrenderButton = GTK_WIDGET(gtk_builder_get_object(builderPlayscreen, "surrenderButton"));
  submitButton = GTK_WIDGET(gtk_builder_get_object(builderPlayscreen, "submitButton"));

  g_signal_connect(timeoutButton, "clicked", G_CALLBACK(on_timeoutButton_clicked), NULL);
  g_signal_connect(surrenderButton, "clicked", G_CALLBACK(on_surrenderButton_clicked), NULL);
  g_signal_connect(submitButton, "clicked", G_CALLBACK(on_submitButton_clicked), NULL);
  g_signal_connect(WindowLogin, "destroy", G_CALLBACK(gtk_main_quit), NULL);
}
void *waitingChallenge(void *inp){
  int tid = *((int *)inp);
  while (1)
  {
    if(isChallenged == 1){
      // pop up có thách đấu
      builderChallengePopup = gtk_builder_new();
      gtk_builder_add_from_file(builderChallengePopup, "challengePopup.glade", NULL);
      g_signal_connect(WindowLogin, "destroy", G_CALLBACK(gtk_main_quit), NULL);
      gtk_widget_show(GTK_WIDGET(WindowChallengePopup));
      isChallenged = 0;
    }
  }
}
int main(int argc, char *argv[])
{
  account.conn_sock = 0;
  account.ELO = 0;
  if (argc != 3)
  {
    fprintf(stdout, "Please enter required argument\n");
    return 0;
  }
  SERV_PORT = atoi(argv[2]);
  strcpy(SERV_IP, argv[1]);
  
  gtk_init(NULL, NULL);
  builderLogin = gtk_builder_new();
  gtk_builder_add_from_file(builderLogin, "Login.glade", NULL);
  builderSignup = gtk_builder_new();
  gtk_builder_add_from_file(builderSignup, "Signup.glade", NULL);
  builderHome = gtk_builder_new();
  gtk_builder_add_from_file(builderHome, "Home.glade", NULL);
  builderPlayscreen = gtk_builder_new();
  gtk_builder_add_from_file(builderPlayscreen, "Playscreen.glade", NULL);
  builderInActive = gtk_builder_new();
  gtk_builder_add_from_file(builderInActive, "inActive.glade", NULL);

  // Step 1: Construct a TCP socket
  client_sock = socket(AF_INET, SOCK_STREAM, 0);

  // Step 2: Define the address of the server
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERV_PORT);
  server_addr.sin_addr.s_addr = inet_addr(SERV_IP);

  if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
  {
    printf("\nError!Can not connect to sever! Client exit imediately! ");
    return 0;
  }

  // pthread_t sendThreadID, recvThreadID;
  account.conn_sock = client_sock;
  pthread_t challengeThread;
  pthread_create(&challengeThread, NULL, waitingChallenge, &client_sock);
  // pthread_create(&sendThreadID, NULL, sendThread, &client_sock);
  // pthread_create(&recvThreadID, NULL, recvThread, &client_sock);
  setupLogin();
  gtk_widget_show(WindowLogin);
  gtk_main();
  pthread_join(challengeThread, NULL);
  // pthread_join(sendThreadID, NULL);
  // pthread_join(recvThreadID, NULL);
}
