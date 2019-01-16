/* BingServer.c
 *
 * Simple server for testing
 *
 * Jerry Pitts
 * 1/8/2018
 */

#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#define PORT 5000

void tictac(void *);

pthread_mutex_t board_mutex = PTHREAD_COND_INITIALIZER;
pthread_mutex_t condition_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_cond  = PTHREAD_COND_INITIALIZER;

struct arg_struct {
    int player1;
    int player2;
    int tid;
};

void tictac(void *arguments) {

  struct arg_struct *args = arguments;
  int player1_fd = args->player1;
  int player2_fd = args->player2;
  int id = args->tid;

  int flag = 1, valread, x;
  char *ack = "Message received";
  char *o_message = "You are O's";
  char *x_message = "You are X's";
  char buffer[1024] = {0};
  char garbage[1024] = {0};
  char board[9] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
  int turn = 0;

  send(player1_fd, x_message, strlen(x_message), 0);
  send(player2_fd, o_message, strlen(o_message), 0);

  while (flag) {

    printf("player1 turn\n");

    send(player1_fd, board, sizeof(board), 0);

    memset(buffer, 0, sizeof(buffer));
    valread = recv(player1_fd , buffer, 1024, 0);
    x = buffer[0] - '0';

    if (x > -1 && x < 9) {
      board[x] = 'X';
    }

    printf("player2 turn\n");

    send(player2_fd, board, sizeof(board), 0);

    memset(buffer, 0, sizeof(buffer));
    valread = recv(player2_fd , buffer, 1024, 0);
    x = buffer[0] - '0';

    if (x > -1 && x < 9) {
      board[x] = 'O';
    }

    if (strcmp(buffer, "exit") == 0) {
      flag = -1;
    }
  }
}

int main(int argc, char const *argv[]) {

  printf("Welcome to BingServer!\n");

  int server_fd, player1_fd, player2_fd, valread; 
  struct sockaddr_in address; 
  int opt = 1, i = 0, flag = 1; 
  int addrlen = sizeof(address);
  struct arg_struct args; 
  pthread_t tid[6];
  char inp[16] = {0};

  printf("Creating Socket...\n");
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { 
      perror("socket FAILURE"); 
      exit(EXIT_FAILURE); 
  } 
     
  printf("Setting Socket Options...\n"); 
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) 
  { 
      perror("setsockopt FAILURE"); 
  //    exit(EXIT_FAILURE); 
  }

  address.sin_family = AF_INET; 
  address.sin_addr.s_addr = htonl(INADDR_ANY); 
  address.sin_port = htons(PORT); 
     
  printf("Binding Socket...\n");
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) 
  { 
      perror("bind FAILURE"); 
      exit(EXIT_FAILURE); 
  } 

  printf("Listening to localhost at port %i ...\n", PORT);
  if (listen(server_fd, 3) < 0) 
  { 
      perror("listen FAILURE"); 
      exit(EXIT_FAILURE); 
  } 

  while (flag) {
    printf("Accepting connection player1...\n");

    if ((player1_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) 
    { 
        perror("accept FAILURE"); 
        exit(EXIT_FAILURE); 
    }

    printf("Accepting connection player2...\n");
    if ((player2_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) 
    { 
        perror("accept FAILURE"); 
        exit(EXIT_FAILURE); 
    }

    args.player1 = player1_fd;
    args.player2 = player2_fd;
    args.tid = i;

    if (pthread_create(&(tid[i]) , NULL, tictac, (void *)&args) != 0) {
      perror("pthread_create");
    }

    i++;
  } 

  for (int j = 0; j < i; j++) {
    pthread_join(tid[j], NULL);
  }

  printf("Client disconnected...\n");


  return 0;

}