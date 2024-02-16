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

#include "function_client.h"
#include <gtk/gtk.h>

struct sockaddr_in server_addr;

gboolean handle_GUI(gpointer user_data) {
  printf("Buff recv: %s\n", account.recv_buff);
  int MSGType = -1;
  char MSG[6];
  for(int i = 0; i < 6; i++){
    MSG[i] = account.recv_buff[i];
  }
  MSG[6] = '\0';
  for(int i = 0; i < (sizeof(MSGS) / sizeof(MSGS[0])); i++){
    if(strcmp(MSG, MSGS[i]) == 0){
      MSGType = i;
      break;
    }
  }
  int timeOut = 0;
  switch (MSGType){
    case 0:{
      timeOut = 0;
      switch (account.recv_buff[7]){
        case '0':{
          gtk_label_set_text(GTK_LABEL(notifyLabel), "Tài khoản bị khóa");
          break;
        }
        case '1':{
          char cELO[STR_SIZE];
          memset(cELO, '\0', STR_SIZE);
          for(int i = 9;i<strlen(account.recv_buff);i++){
            cELO[i-9] = account.recv_buff[i]; 
          }
          account.elo = atoi(cELO);
          setupHome();
          gtk_label_set_text(GTK_LABEL(usernameData), account.name);
          gtk_label_set_text(GTK_LABEL(ELOData), cELO);
          break;
        }
        case '2':{
          gtk_label_set_text(GTK_LABEL(notifyLabel), "Tài khoản đã được đăng nhập ở nơi khác");
          break;
        }
        case '3':{
          gtk_label_set_text(GTK_LABEL(notifyLabel), "Sai tài khoản hoặc mật khẩu");
          break;
        }
        default:{
          break;
        }
      }
      break;
    }
    case 1:{
      timeOut = 0;
      if(account.recv_buff[7] == '1'){
        memset(account.name, '\0',BUFF_SIZE);
        memset(account.password, '\0', BUFF_SIZE);
        account.elo = 0;
        setupLogin();
      }
      break;
    }
    case 2:{
      timeOut = 0;
      if(account.recv_buff[7] == '0'){
        gtk_label_set_text(GTK_LABEL(notifyLabel), "Đăng ký không thành công");
      }
      else{
        gtk_label_set_text(GTK_LABEL(notifyLabel), "Đăng  ký thành công");
      }
      break;
    }
    case 3:{
      setupPlayscreen();
      int temp = 7;
      printf("???1\n");
      memset(gameInfo.P1Name, '\0', STR_SIZE);
      for(int i = temp; i<strlen(account.recv_buff); i++){
        if(account.recv_buff[i] == '#'){
          i++;
          temp = i;
          break;
        }
        gameInfo.P1Name[i-temp] = account.recv_buff[i];
      }
      printf("???2\n");
      memset(gameInfo.P2Name, '\0', STR_SIZE);
      for(int i = temp; i<strlen(account.recv_buff); i++){
        if(account.recv_buff[i] == '#'){
          i++;
          temp = i;
          break;
        }
        gameInfo.P2Name[i-temp] = account.recv_buff[i];
      }
      printf("???3\n");
      char Row[10];
      memset(Row, '\0', 10);
      for(int i = temp; i<strlen(account.recv_buff); i++){
        if(account.recv_buff[i] == '#'){
          i++;
          temp = i;
          break;
        }
        Row[i-temp] = account.recv_buff[i];
      }
      gameInfo.quiz.row = atoi(Row);
      char Collum[10];
      printf("???4\n");
      memset(Collum, '\0', 10);
      for(int i = temp; i<strlen(account.recv_buff); i++){
        if(account.recv_buff[i] == '#'){
          i++;
          temp = i;
          break;
        }
        Collum[i-temp] = account.recv_buff[i];
      }
      printf("???5\n");
      gameInfo.quiz.collum = atoi(Collum);
      memset(gameInfo.quiz.colHints, '\0', SENTENCE_SIZE);
      for(int i = temp; i<strlen(account.recv_buff); i++){
        if(account.recv_buff[i] == '#'){
          i++;
          temp = i;
          break;
        }
        gameInfo.quiz.colHints[i-temp] = account.recv_buff[i];
      }
      break;
    }
    case 4:{
      timeOut = 0;
      break;
    }
    case 5:{
      timeOut = 0;
      break;
    }
    case 6:{
      timeOut = 0;
      break;
    }
    case 7:{
      timeOut = 0;
      break;
    }
    case 8:{
      timeOut = 0;
      break;
    }
    case 9:{
      timeOut = 0;
      break;
    }
    case 10:{
      timeOut = 0;
      break;
    }
    case 11:{
      timeOut = 0;
      int temp = 7;
      int row, spacing, length;
      char Row[10];
      memset(Row, '\0', 10);
      for(int i = temp; i <strlen(account.recv_buff) ; i++){
        if(account.recv_buff[i] == '#'){
          i++;
          temp = i;
          break;
        }
        Row[i-temp] = account.recv_buff[i];
      }
      row = atoi(Row);
      char Spacing[10];
      memset(Spacing, '\0', 10);
      for(int i = temp; i <strlen(account.recv_buff) ; i++){
        if(account.recv_buff[i] == '#'){
          i++;
          temp = i;
          break;
        }
        Spacing[i-temp] = account.recv_buff[i];
      }
      spacing = atoi(Spacing);
      gameInfo.quiz.rowData[row].spacing = spacing;
      char Length[10];
      memset(Length, '\0', 10);
      for(int i = temp; i <strlen(account.recv_buff) ; i++){
        if(account.recv_buff[i] == '#'){
          i++;
          temp = i;
          break;
        }
        Length[i-temp] = account.recv_buff[i];
      }
      length = atoi(Length);
      gameInfo.quiz.rowData[row].length = length;
      memset(gameInfo.quiz.rowData[row].rowAnswer, '\0', STR_SIZE);
      for(int i = temp; i <strlen(account.recv_buff) ; i++){
        if(account.recv_buff[i] == '#'){
          i++;
          temp = i;
          break;
        }
        gameInfo.quiz.rowData[row].rowAnswer[i-temp] = account.recv_buff[i];
      }
      if(row == gameInfo.quiz.row-1){
        printf("ended\n");
      }
      break;
    }
    case 12:{
      timeOut = 0;
      Logging inActiveList[BACKLOG];
      int listNumber;
      int temp = 7;
      for(listNumber = 0; listNumber<BACKLOG; listNumber++){
        if(temp>=strlen(account.recv_buff)) break;
        memset(inActiveList[listNumber].name, '\0', STR_SIZE);
        for(int i = temp; i<strlen(account.recv_buff); i++){
          if(account.recv_buff[i] == '#'){
            i++;
            temp = i;
            break;
          }
          inActiveList[listNumber].name[i-temp] = account.recv_buff[i];
        }
        char cELO[STR_SIZE];
        memset(cELO, '\0', STR_SIZE);
        for(int i = temp; i<strlen(account.recv_buff); i++){
          if(account.recv_buff[i] == '#'){
            i++;
            temp = i;
            break;
          }
          cELO[i-temp] = account.recv_buff[i];
        }
        inActiveList[listNumber].elo = atoi(cELO);

        printf("%d\n", listNumber);
      }
      setupInActive();
      GList *children, *iter;

      // Lấy danh sách các phần tử con trong GtkGrid
      children = gtk_container_get_children(GTK_CONTAINER(gridTable));

      // Lặp qua danh sách và xóa từng phần tử con
      for (iter = children; iter != NULL; iter = g_list_next(iter)) {
          gtk_widget_destroy(GTK_WIDGET(iter->data));
      }

      // Giải phóng danh sách
      g_list_free(children);


      GtkWidget *name_label = gtk_label_new("Player Name");
      GtkWidget *elo_label = gtk_label_new("Elo");
      gtk_grid_attach(GTK_GRID(gridTable), name_label, 0, 0, 1, 1);
      gtk_grid_attach(GTK_GRID(gridTable), elo_label, 1, 0, 1, 1);
      gtk_widget_show(name_label);
      gtk_widget_show(elo_label);
      for (int i = 0; i < listNumber; i++) {
          GtkWidget *name_entry = gtk_label_new(inActiveList[i].name);
          GtkWidget *elo_entry = gtk_label_new(g_strdup_printf("%d", inActiveList[i].elo));
          GtkWidget *challenge_button = gtk_button_new_with_label("Challenge");
          g_signal_connect(challenge_button, "clicked", G_CALLBACK(on_challengeButton_clicked), (gpointer)inActiveList[i].name);
          
          gtk_grid_attach(GTK_GRID(gridTable), name_entry, 0, i+1, 1, 1);
          gtk_grid_attach(GTK_GRID(gridTable), elo_entry, 1, i+1, 1, 1);
          gtk_grid_attach(GTK_GRID(gridTable), challenge_button, 2, i+1, 1, 1);
          gtk_widget_show(name_entry);
          gtk_widget_show(elo_entry);
          gtk_widget_show(challenge_button);
      }
      
      break;
    }
    
    default:{
      timeOut++;
      break;
    }
  }
  return FALSE;
}

void* GUI_thread(void* data) {
  pthread_detach(pthread_self());
  while (1){
    memset(account.recv_buff, '\0', BUFF_SIZE);
    bytes_received = recv(account.conn_sock, account.recv_buff, BUFF_SIZE, 0);
    g_idle_add(handle_GUI, data);
    sleep(1);
  }
  return NULL;
}

int main(int argc, char *argv[]){
  currentScreen = -1;
  int client_sock;
  if (argc != 3){
    fprintf(stdout, "Please enter required argument\n");
    return 0;
  }
  SERV_PORT = atoi(argv[2]);
  strcpy(SERV_IP, argv[1]);

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
  account.conn_sock = client_sock;
  
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
  builderChallengePopup = gtk_builder_new();
  gtk_builder_add_from_file(builderChallengePopup, "challengePopup.glade", NULL);
  setupLogin();
  printf("Socket%d\n", account.conn_sock);
  pthread_t thread_id;
  pthread_create(&thread_id, NULL, GUI_thread, NULL);
  gtk_main();
}