#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#define BUFF_SIZE 2048
#define BACKLOG 10
#define STR_SIZE 100
#define SENTENCE_SIZE 900

char *MSGC[] = {"MSGC01", "MSGC02", "MSGC03", "MSGC04", "MSGC05", "MSGC06", "MSGC07", "MSGC08", "MSGC09", "MSGC10", "MSGC11", "MSGC12", "MSGC13", "MSGC14"};
char *MSGS[] = {"MSGS01", "MSGS02", "MSGS03", "MSGS04", "MSGS05", "MSGS06", "MSGS07", "MSGS08", "MSGS09", "MSGS10", "MSGS11", "MSGS12", "MSGS13", "MSGS14"};

int SERV_PORT;
char SERV_IP[STR_SIZE];
int currentScreen;
int bytes_sent, bytes_received, sin_size;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t threads_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct{
	int spacing;    //cách dòng
    int length;
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
	char P1Name[STR_SIZE];
	char P2Name[STR_SIZE];
	int playerTurn; // 0: player1, 1: player2
	int player1Score;
	int player2Score;
	Quiz quiz;
} ChallangeData;
typedef struct
{
    int conn_sock;
	char name[STR_SIZE];
	char password[STR_SIZE];
	int elo; 
    char send_buff[BUFF_SIZE];
    char recv_buff[BUFF_SIZE];
} Logging;
Logging account;
ChallangeData gameInfo;

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
GtkWidget *backButton;
GtkWidget *reloadButton;
GtkWidget *gridTable;

void setupLogin();
void setupSignup();
void setupHome();
void setupPlayscreen();

void on_window_closed(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}
//Login handle
void on_loginButton_clicked(GtkButton *button, gpointer user_data) {
    char username[STR_SIZE], password[STR_SIZE];
    usernameEntry = GTK_WIDGET(gtk_builder_get_object(builderLogin, "usernameEntry"));
    passwordEntry = GTK_WIDGET(gtk_builder_get_object(builderLogin, "passwordEntry"));
    strcpy(username, gtk_entry_get_text(GTK_ENTRY(usernameEntry)));
    strcpy(password, gtk_entry_get_text(GTK_ENTRY(passwordEntry)));
    strcpy(account.name, username);
    strcpy(account.password, password);
    memset(account.send_buff, '\0', BUFF_SIZE);
    sprintf(account.send_buff, "MSGC01#%s#%s", username, password);
    bytes_sent = send(account.conn_sock, account.send_buff, BUFF_SIZE, 0);
}
void switch_to_signup(GtkButton *button, gpointer user_data) {
    setupSignup();
}
// END login handle
// SIGNUP handle
void on_signupButton_clicked(GtkButton *button, gpointer user_data){
    char username[STR_SIZE], password[STR_SIZE], resignPassword[STR_SIZE];
    strcpy(username, gtk_entry_get_text(GTK_ENTRY(usernameEntry)));
    strcpy(password, gtk_entry_get_text(GTK_ENTRY(passwordEntry)));
    strcpy(resignPassword, gtk_entry_get_text(GTK_ENTRY(resignPasswordEntry)));
    if(strcmp(password, resignPassword) !=0){
        gtk_label_set_text(GTK_LABEL(notifyLabel), "Mật khẩu không trùng nhau");
        return;
    }
    memset(account.send_buff, '\0', BUFF_SIZE);
    sprintf(account.send_buff, "MSGC03#%s#%s", username, password);
    bytes_sent = send(account.conn_sock, account.send_buff, BUFF_SIZE, 0);
}
void switch_to_login(GtkButton *button, gpointer user_data) {
    setupLogin();
}
// END signup handle
// HOME handle
void findGame(GtkButton *button, gpointer user_data){
    bytes_sent = send(account.conn_sock, "MSGC07#", BUFF_SIZE, 0);
}
void findActive(GtkButton *button, gpointer user_data){
    bytes_sent = send(account.conn_sock, "MSGC08#", BUFF_SIZE, 0);
}
void signout(GtkButton *button, gpointer user_data){
    bytes_sent = send(account.conn_sock, "MSGC02#", BUFF_SIZE, 0);
}
// END home handle
// PLAYSCREEN hanlde

// END playcreen handle
// INACTIVE handle
void on_challengeButton_clicked(GtkButton *button, gpointer user_data){
    gchar *player_name = (const gchar *)user_data;
    memset(account.send_buff, '\0', BUFF_SIZE);
    sprintf(account.send_buff, "MSGC04#%s", player_name);
    bytes_sent = send(account.conn_sock, account.send_buff, BUFF_SIZE, 0);
}
void reloadInActive(GtkButton *button, gpointer user_data){
    bytes_sent = send(account.conn_sock, "MSGC08#", BUFF_SIZE, 0);
}
void backHome(GtkButton *button, gpointer user_data){
    setupHome();
}
// END inactive handle
void hideWindow(){
    switch (currentScreen){
        case 0:{
            //Login
            gtk_widget_hide(GTK_WIDGET(WindowLogin));
            break;
        }
        case 1:{
            //Signup
            gtk_widget_hide(GTK_WIDGET(WindowSignup));
            break;
        }
        case 2:{
            //Home
            gtk_widget_hide(GTK_WIDGET(WindowHome));
            break;
        }
        case 3:{
            //PlayScreen
            gtk_widget_hide(GTK_WIDGET(WindowPlayscreen));
            break;
        }
        case 4:{
            //InActive
            gtk_widget_hide(GTK_WIDGET(WindowInActive));
            break;
        }
        default :{
            break;
        }
    }
}
void setupLogin(){
    hideWindow();
    currentScreen = 0;
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
    gtk_widget_show(GTK_WIDGET(WindowLogin)); // Show the login window
}
void setupSignup(){
    hideWindow();
    currentScreen = 1;
    WindowSignup = GTK_WIDGET(gtk_builder_get_object(builderSignup, "window"));
    usernameLabel = GTK_WIDGET(gtk_builder_get_object(builderSignup, "usernameLabel"));
    passwordLabel = GTK_WIDGET(gtk_builder_get_object(builderSignup, "passwordLabel"));
    resignPasswordLabel = GTK_WIDGET(gtk_builder_get_object(builderSignup, "resignPasswordLabel"));
    usernameEntry = GTK_WIDGET(gtk_builder_get_object(builderSignup, "usernameEntry"));
    passwordEntry = GTK_WIDGET(gtk_builder_get_object(builderSignup, "passwordEntry"));
    resignPasswordEntry = GTK_WIDGET(gtk_builder_get_object(builderSignup, "resignPasswordEntry"));
    loginButton = GTK_WIDGET(gtk_builder_get_object(builderSignup, "loginButton"));
    signupButton = GTK_WIDGET(gtk_builder_get_object(builderSignup, "signupButton"));
    notifyLabel = GTK_WIDGET(gtk_builder_get_object(builderSignup, "notifyLabel"));
    g_signal_connect(signupButton, "clicked", G_CALLBACK(on_signupButton_clicked), NULL);
    g_signal_connect(loginButton, "clicked", G_CALLBACK(switch_to_login), NULL);
    g_signal_connect(WindowLogin, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show(GTK_WIDGET(WindowSignup)); 
}
void setupHome(){
    hideWindow();
    currentScreen  = 2;
    WindowHome = GTK_WIDGET(gtk_builder_get_object(builderHome, "window"));
    usernameData = GTK_WIDGET(gtk_builder_get_object(builderHome, "usernameData"));
    ELOData = GTK_WIDGET(gtk_builder_get_object(builderHome, "ELOData"));
    findGameButton = GTK_WIDGET(gtk_builder_get_object(builderHome, "findGameButton"));
    findActiveButton = GTK_WIDGET(gtk_builder_get_object(builderHome, "findActiveButton"));
    logoutButton = GTK_WIDGET(gtk_builder_get_object(builderHome, "logoutButton"));
    g_signal_connect(findGameButton, "clicked", G_CALLBACK(findGame), NULL);
    g_signal_connect(findActiveButton, "clicked", G_CALLBACK(findActive), NULL);
    g_signal_connect(logoutButton, "clicked", G_CALLBACK(signout), NULL);
    g_signal_connect(WindowHome, "destroy", G_CALLBACK(on_window_closed), NULL);
    gtk_widget_show(GTK_WIDGET(WindowHome)); 
};
void setupPlayscreen(){
    hideWindow();
    currentScreen = 3;
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

    // g_signal_connect(timeoutButton, "clicked", G_CALLBACK(on_timeoutButton_clicked), NULL);
    // g_signal_connect(surrenderButton, "clicked", G_CALLBACK(on_surrenderButton_clicked), NULL);
    // g_signal_connect(submitButton, "clicked", G_CALLBACK(on_submitButton_clicked), NULL);
    g_signal_connect(WindowPlayscreen, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show(GTK_WIDGET(WindowPlayscreen)); 
};
void setupInActive(){
    hideWindow();
    currentScreen = 4;
    WindowInActive = GTK_WIDGET(gtk_builder_get_object(builderInActive, "window"));
    gridTable = GTK_WIDGET(gtk_builder_get_object(builderInActive, "gridTable"));
    backButton = GTK_WIDGET(gtk_builder_get_object(builderInActive, "backButton"));
    reloadButton = GTK_WIDGET(gtk_builder_get_object(builderInActive, "reloadButton"));
    g_signal_connect(backButton, "clicked", G_CALLBACK(backHome), NULL);
    g_signal_connect(reloadButton, "clicked", G_CALLBACK(reloadInActive), NULL);
    g_signal_connect(WindowInActive, "destroy", G_CALLBACK(on_window_closed), NULL);
    gtk_widget_show(GTK_WIDGET(WindowInActive)); 
}




