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

#define BUFF_SIZE 8192

int SERV_PORT;
char SERV_IP[100];
int client_sock;
char buff[BUFF_SIZE];
struct sockaddr_in server_addr;
int bytes_sent, bytes_received, sin_size;

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
  while (1)
  {
    memset(buff, '\0', (strlen(buff) + 1));
    fflush(stdout);
    fgets(buff, BUFF_SIZE, stdin);
    if (strcmp(buff, "#\n") == 0 || strcmp(buff, "@\n") == 0)
    {
      close(client_sock);
      exit(0);
    }
    printf("%s\n", buff);
    // if (containsNonAlphaNumeric(buff) == 0)
    // {
    //   printf("Please enter only number and alphabet characters\n");
    //   continue;
    // }
    sin_size = sizeof(struct sockaddr);
    bytes_sent = send(client_sock, buff, strlen(buff), 0);
    if (bytes_sent < 0)
    {
      perror("Error: ");
      close(client_sock);
      return 0;
    }
  }
}

void *recvThread(void *arg)
{
  while (1)
  {
    memset(buff, '\0', (strlen(buff) + 1));
    bytes_received = recv(client_sock, buff, BUFF_SIZE - 1, 0);
    if (bytes_received < 0)
    {
      perror("Error: ");
      close(client_sock);
      return 0;
    }
    buff[bytes_received] = '\0';
    printf("%s\n", buff);
    if (buff[0] == 'G')
    {
      close(client_sock);
      exit(0);
    }
  }
}


int main(int argc, char *argv[])
{
  if (argc != 3)
  {
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

  pthread_t sendThreadID;
  pthread_t recvThreadID;

  pthread_create(&sendThreadID, NULL, sendThread, &client_sock);
  pthread_create(&recvThreadID, NULL, recvThread, &client_sock);
  while (1)
  {
  }
  pthread_join(sendThreadID, NULL);
  pthread_join(recvThreadID, NULL);
}
