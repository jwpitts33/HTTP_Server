/*
 * bingserver.c
 *
 *
 *
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#define PORT 5000

void handle_request(void*);
int serve_page(int client_fd, char* url);

struct arg_struct {
    int client_fd;
    int t_id;
};

void handle_request(void* arguments) {

  struct arg_struct *args = arguments;
  int client_fd = args->client_fd;
  int i, j;

  int valread = 0;
  char buffer[1024] = {0};
  char method[10] = {0};
  char url[50] = {0};

  valread = recv(client_fd, buffer, sizeof(buffer), 0);
  if (valread > 0) {
    printf("%s\n", buffer);
  }

  i = 0, j = 0;
  while (buffer[i] != ' ') {
    method[j] = buffer[i];
    i++;
    j++;
  }
  method[j] = '\0';
  i++;
  j = 0;
  while (buffer[i] != ' ') {
    url[j] = buffer[i];
    i++;
    j++;
  }
  url[i] = '\0';

  if(strcmp(url,"/") == 0) {
    strcat(url, "index.html");
  }

  if (strcmp("GET", method) == 0) {
    serve_page(client_fd, url);
  }

  close(client_fd);

}

int serve_page(int client_fd, char *url) {

  char filepath[64] = "htdocs";
  char response[1024] = {0};
  char *html;
  FILE *fp;
  long numbytes;
  strcat(filepath, url);

  printf("%s\n",filepath);

  fp = fopen(filepath, "r");

  if(fp == NULL) {
    return -1;
  }

  fseek(fp, 0L, SEEK_END);
  numbytes = ftell(fp);
  fseek(fp, 0L, SEEK_SET);
  html = (char*)calloc(numbytes, sizeof(char)); 

  if(html == NULL)
    return 1;

  fread(html, sizeof(char), numbytes, fp);
  fclose(fp);

  strcat(response, "HTTP/1.0 200 OK\r\n");
  strcat(response, "Server: jdbhttpd/0.1.0\r\n");
  strcat(response, "Content-Type: text/html\r\n");
  strcat(response, html);

  send(client_fd, response, sizeof response, 0);

  free(html);

  return 1;
}

int main(int argc, char *argv[]) {

  int server_fd, client_fd;
  struct sockaddr_in server_ad, client_ad;
  int opt = 1;
  socklen_t sin_size;
  char client_ip[INET_ADDRSTRLEN];
  char server_ip[INET_ADDRSTRLEN];

  pthread_t thread;
  int t_num = 0;
  struct arg_struct args;

  memset(&server_ad, 0, sizeof server_ad);
  server_ad.sin_family = AF_INET;
  server_ad.sin_addr.s_addr = htonl(INADDR_ANY); 
  server_ad.sin_port = htons(PORT);

  printf("---------------------------------------------\n");
  printf("             STARTING SERVER                 \n");
  printf("---------------------------------------------\n\n");

  printf("CREATING SOCKET\n");
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("server: socket");  
  }

  printf("SETTING OPTIONS\n");
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1) {
    perror("setsockopt");
    exit(1);
  } 

  printf("BINDING\n");
  if (bind(server_fd, (struct sockaddr *) &server_ad, sizeof server_ad) == -1) {
    close(server_fd);
    perror("server: bind");
  }

  inet_ntop(AF_INET, (struct sockaddr *)&server_ad, server_ip, sizeof(server_ip));
  printf("LISTENING TO %s ON PORT %i\n\n", server_ip, PORT);
  if (listen(server_fd, 5) == -1) {
    perror("listen");
    exit(1);
  }

  while(1) {

    sin_size = sizeof client_ad;
    client_fd = accept(server_fd, (struct sockaddr *)&client_ad, &sin_size);
    if (client_fd == -1) {
        perror("accept");
        continue;
    }

    inet_ntop(client_ad.sin_family, (struct sockaddr *)&client_ad, client_ip, sizeof(client_ip));
    printf("---------------------------------------------\n");
    printf("server: received connection from %s\n", client_ip);
    printf("---------------------------------------------\n\n");

    args.client_fd = client_fd;
    args.t_id = t_num;
    t_num++;

    if (pthread_create(&thread , NULL, (void *)handle_request, (void *)&args) != 0) {
      perror("pthread_create");
    }

  }

  close(server_fd);

  return 0;

}
