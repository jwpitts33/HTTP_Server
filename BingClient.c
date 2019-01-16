/* BingClient.c
 * 
 * Simple client for testing
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

#define PORT 5000
#define IP_ADDR "192.168.0.10"
   
int main(int argc, char const *argv[]) { 

    int client_fd = 0, valread, flag = 1, i; 
    struct sockaddr_in serv_addr; 
    char str [80]; 
    char buffer[1024] = {0}; 

    printf("Welcome to BingClient");


    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
      perror("socket FAILURE"); 
      exit(EXIT_FAILURE); 
    } 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, IP_ADDR, &serv_addr.sin_addr)<= 0)  
    { 
        perror("address FAILURE"); 
        exit(EXIT_FAILURE); 
    } 
 
    printf("Connecting to BingServer...\n");
    printf("%s at port %i\n", IP_ADDR, PORT);
    if (connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("connection FAILURE"); 
        exit(EXIT_FAILURE); 
    }

    valread = recv(client_fd, buffer, sizeof(buffer), 0);
    printf("%s\n", buffer);


    while(flag > 0) {

      valread = recv(client_fd, buffer, sizeof(buffer), 0);

      printf("\n");
      for (i = 0; i < 3; i++) {
        printf("%c|%c|%c\n", buffer[i*3], buffer[i*3+1], buffer[i*3+2]);
        if (i != 2) {
          printf("-----\n");
        }
      }
      printf("\n");

      memset(str, 0, sizeof(str));
      printf("\nEnter an index to change...\n");
      scanf("%s", str);
      if('\n' == str[strlen(str) - 1])
        str[strlen(str) - 1] = '\0';

      send(client_fd, str, strlen(str), 0);

      printf("Waiting for other player");


      if (strcmp(str, "exit") == 0) {
        flag = -1;
      }

    }

    printf("Disconnected from server...\n");
    close(client_fd);

    return 0; 
}