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

#define BUFF_SIZE 8192
char *MSGC[] = {"MSGC01", "MSGC02", "MSGC03", "MSGC04", "MSGC05", "MSGC06", "MSGC07", "MSGC08", "MSGC09", "MSGC10", "MSGC11", "MSGC12"};
char *MSGS[] = {"MSGS01", "MSGS02", "MSGS03", "MSGS04", "MSGS05", "MSGS06", "MSGS07", "MSGS08", "MSGS09", "MSGS10", "MSGS11", "MSGS12"};

int SERV_PORT;
char SERV_IP[100];
int client_sock;
char buff[BUFF_SIZE];
struct sockaddr_in server_addr;
int bytes_sent, bytes_received, sin_size;
int isChallenged;

typedef struct
{
	int conn_sock;
	char name[1000];
	char password[1000];
	int ELO;
} Account;
Account *account;
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

void *sendThread(void *arg)
{
  // while (1)
  // {
  //   memset(buff, '\0', (strlen(buff) + 1));
  //   fflush(stdout);
  //   fgets(buff, BUFF_SIZE, stdin);
  //   if (strcmp(buff, "#\n") == 0 || strcmp(buff, "@\n") == 0)
  //   {
  //     close(client_sock);
  //     exit(0);
  //   }
  //   printf("%s\n", buff);
  //   // if (containsNonAlphaNumeric(buff) == 0)
  //   // {
  //   //   printf("Please enter only number and alphabet characters\n");
  //   //   continue;
  //   // }
  //   sin_size = sizeof(struct sockaddr);
  //   bytes_sent = send(client_sock, buff, strlen(buff), 0);
  //   if (bytes_sent < 0)
  //   {
  //     perror("Error: ");
  //     close(client_sock);
  //     return 0;
  //   }
  // }
  loginpage();
  gtk_main();
}

void *recvThread(void *arg)
{
  while (1)
  {
    memset(buff, '\0', (strlen(buff) + 1));
    bytes_received = recv(client_sock, buff, BUFF_SIZE - 1, 0);
    printf("%s\n", buff);
    if (bytes_received < 0)
    {
      perror("Error: ");
      close(client_sock);
      return 0;
    }
    char MSGtype[10];
    int MSGT = -1;
    for(int i = 0; i < 6; i++){
      MSGtype[i] = buff[i];
    }
    MSGtype[6] = '\0';
    for(int i = 0; i< sizeof(MSGS); i++){
      if(strcmp(MSGS[i], MSGtype) == 0){
        MSGT = i;
        break;
      }
    }
    printf("MSG Type: %d\n", MSGT);
    switch (MSGT)
    {
    case 0:
      /* code */
      notifyLabel = GTK_WIDGET(gtk_builder_get_object(builderLogin, "notifyLabel"));
      switch (buff[7])
      {
      case '0':
        /* code */
        gtk_label_set_text(GTK_LABEL(notifyLabel), "Tài khoản bị khóa");
        break;
      case '1':
        /* code */
        gtk_label_set_text(GTK_LABEL(notifyLabel), "Đăng nhập thành công");
        char cELO[1000];
        memset(cELO, '\0', sizeof(cELO));
        for(int i = 9;i<strlen(buff);i++){
          if(buff[i] == '#'){

            i++;
            int k = i;
            for(;i<strlen(buff); i++){
              cELO[i-k] = buff[i]; 
            }
          }
        }
        int k = atoi(cELO);
        account->ELO = k;
        printf("ELO: %d\n", account->ELO);
        printf("%s\n%s\n%d", account->name, account->password, account->ELO);
        gtk_widget_hide(GTK_WIDGET(WindowLogin));
        printf("stop\n");
        setupHome();
        gtk_label_set_text(GTK_LABEL(usernameData), account->name);
        printf("stop\n");
        gtk_label_set_text(GTK_LABEL(ELOData), cELO);
        printf("stop\n");
        gtk_widget_show(GTK_WIDGET(WindowHome));
        printf("stop\n");
        break;
      case '2':
        /* code */
        printf("???\n");
        gtk_label_set_text(GTK_LABEL(notifyLabel), "Tài khoản đã được đăng nhập ở nơi khác");
        break;
      case '3':
        /* code */
        notifyLabel = GTK_WIDGET(gtk_builder_get_object(builderHome, "notifyLabel"));
        gtk_label_set_text(GTK_LABEL(notifyLabel), "Sai tài khoản hoặc mật khẩu");
        break;
      default:
        break;
      }
      break;
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
    // printf("%s\n", buff);
    // if (buff[0] == 'G')
    // {
    //   close(client_sock);
    //   exit(0);
    // }
  }
}

void on_loginButton_clicked(GtkButton *button, gpointer user_data) {
    char username[100], password[100];
    usernameEntry = GTK_WIDGET(gtk_builder_get_object(builderLogin, "usernameEntry"));
    passwordEntry = GTK_WIDGET(gtk_builder_get_object(builderLogin, "passwordEntry"));
    strcpy(username, gtk_entry_get_text(GTK_ENTRY(usernameEntry)));
    strcpy(password, gtk_entry_get_text(GTK_ENTRY(passwordEntry)));
    memset(buff, '\0', (strlen(buff) + 1));
    fflush(stdout);
    strcpy(buff, "MSGC01#");
    strcat(buff, username);
    strcat(buff, "#");
    strcat(buff, password);
    bytes_sent = send(client_sock, buff, strlen(buff), 0);
    if (bytes_sent < 0)
    {
      perror("Error: ");
      close(client_sock);
      return 0;
    }
    strcpy(account->name, username);
    strcpy(account->password, password);
}

void on_signupButton_clicked(){}
void switch_to_signup(GtkButton *button, gpointer user_data) {
  gtk_widget_hide(GTK_WIDGET(WindowLogin)); // Hide the login window
  setupSignup();
  gtk_widget_show(GTK_WIDGET(WindowSignup)); // Show the signup window
}
void switch_to_login(GtkButton *button, gpointer user_data) {
  gtk_widget_hide(GTK_WIDGET(WindowSignup)); // Hide the signup window
  setupLogin();
  gtk_widget_show(GTK_WIDGET(WindowLogin)); // Show the login window
}

void findGame(){}
void findActive(){}
void signout(){}
void setupLogin(){
  WindowLogin = GTK_WIDGET(gtk_builder_get_object(builderLogin, "window"));
  usernameLabel = GTK_WIDGET(gtk_builder_get_object(builderLogin, "usernameLabel"));
  passwordLabel = GTK_WIDGET(gtk_builder_get_object(builderLogin, "passwordLabel"));
  usernameEntry = GTK_WIDGET(gtk_builder_get_object(builderLogin, "usernameEntry"));
  passwordEntry = GTK_WIDGET(gtk_builder_get_object(builderLogin, "passwordEntry"));
  loginButton = GTK_WIDGET(gtk_builder_get_object(builderLogin, "loginButton"));
  signupButton = GTK_WIDGET(gtk_builder_get_object(builderLogin, "signupButton"));
  g_signal_connect(loginButton, "clicked", G_CALLBACK(on_loginButton_clicked), NULL);
  g_signal_connect(signupButton, "clicked", G_CALLBACK(switch_to_signup), NULL);
  g_signal_connect(WindowLogin, "destroy", G_CALLBACK(gtk_main_quit), NULL);
}
void setupSignup(){
  WindowSignup = GTK_WIDGET(gtk_builder_get_object(builderSignup, "window"));
  usernameLabel = GTK_WIDGET(gtk_builder_get_object(builderSignup, "usernameLabel"));
  passwordLabel = GTK_WIDGET(gtk_builder_get_object(builderSignup, "passwordLabel"));
  resignPasswordLabel = GTK_WIDGET(gtk_builder_get_object(builderSignup, "resignPasswordLabel"));
  usernameEntry = GTK_WIDGET(gtk_builder_get_object(builderSignup, "usernameEntry"));
  passwordEntry = GTK_WIDGET(gtk_builder_get_object(builderSignup, "passwordEntry"));
  resignPasswordEntry = GTK_WIDGET(gtk_builder_get_object(builderSignup, "resignPasswordEntry"));
  loginButton = GTK_WIDGET(gtk_builder_get_object(builderSignup, "loginButton"));
  signupButton = GTK_WIDGET(gtk_builder_get_object(builderSignup, "signupButton"));
  g_signal_connect(loginButton, "clicked", G_CALLBACK(on_signupButton_clicked), NULL);
  g_signal_connect(signupButton, "clicked", G_CALLBACK(switch_to_login), NULL);
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
void *waitingChallenge(void *inp){
  int tid = *((int *)inp);
  while (1)
  {
    if(isChallenged == 1){
      // pop up có thách đấu
      builderChallengePopup = gtk_builder_new();
      gtk_builder_add_from_file(builderChallengePopup, "challengePopup.glade", NULL);
      g_signal_connect(WindowLogin, "destroy", G_CALLBACK(gtk_main_quit), NULL);
      gtk_widget_show(GTK_WIDGET(WindowChallengePopup)); // Show the login window
      isChallenged = 0;
    }
  }
}

void loginpage(){
  setupLogin();
  gtk_widget_show(WindowLogin);
}

int main(int argc, char *argv[])
{
  account = (Account *)malloc(sizeof(Account));
  if (argc != 3)
  {
    fprintf(stdout, "Please enter required argument\n");
    return 0;
  }
  SERV_PORT = atoi(argv[2]);
  strcpy(SERV_IP, argv[1]);
  
  gtk_init(NULL, NULL);
  builderLogin = gtk_builder_new();
  gtk_builder_add_from_file(builderLogin, "login.glade", NULL);
  builderSignup = gtk_builder_new();
  gtk_builder_add_from_file(builderSignup, "Signup.glade", NULL);
  builderHome = gtk_builder_new();
  gtk_builder_add_from_file(builderHome, "Home.glade", NULL);
  builderPlayscreen = gtk_builder_new();
  gtk_builder_add_from_file(builderPlayscreen, "Playscreen.glade", NULL);

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
  account->conn_sock = client_sock;
  pthread_t challengeThread;
  pthread_create(&challengeThread, NULL, waitingChallenge, &client_sock);
  pthread_t sendThreadID;
  pthread_t recvThreadID;

  pthread_create(&sendThreadID, NULL, sendThread, &client_sock);
  pthread_create(&recvThreadID, NULL, recvThread, &client_sock);

  pthread_join(sendThreadID, NULL);
  pthread_join(recvThreadID, NULL);

  loginpage();
  gtk_main();
  pthread_join(challengeThread, NULL);
}
