#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define pic_len 1000000
#define BUFSIZE 4096
#define PORTNUM 9090
#define MAXLINE 1024

char buffer[pic_len];

/*static void sigchld_handler() {
  pid_t PID;
  int status;

  while (PID = waitpid(-1, &status, WNOHANG) > 0)
    printf("子程序 %d 結束.\n", PID);

  // Re-install handler 
  signal(SIGCHLD, sigchld_handler);
}
*/

int main(int argc, char **argv) {
  int  z;
  struct sockaddr_in adr_srvr;
  int  sockfd, connfd;
  pid_t  PID;
  char pack[4096];
  char tmp[4096];

  signal(SIGCHLD, SIG_IGN);

  /*FILE *fp = fopen("./pic.html", "r");
  if(fp == NULL){
    printf("open error\n");
    exist(1);
  }*/

  /* 製作 socket */  
  //socket listen 時會有個 queue 負責裝排隊等待和 server 連線的 client
  sockfd = socket(AF_INET, SOCK_STREAM, 0);   //開好的socket是以fd形式存在
  if (sockfd == -1) {
    printf("socket error");
    exit(1);
  }

  /* 準備連線端指定用的 struct 資料 */  
  bzero(&adr_srvr, sizeof(adr_srvr));
  adr_srvr.sin_family = AF_INET;
  adr_srvr.sin_addr.s_addr = htonl(INADDR_ANY);
  adr_srvr.sin_port = htons(8080);

  int reuseAddrOption = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseAddrOption, sizeof(reuseAddrOption)) < 0)
    {
        printf("Fail to set a socket: %s\n", strerror(errno));
        exit(1);
    }
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseAddrOption, sizeof(reuseAddrOption));

  z = bind(sockfd, (struct sockaddr *)&adr_srvr, sizeof(adr_srvr));
  if (z == -1) {
    printf("bind error : %s",strerror(errno) );
    exit(1);
  }

  z = listen(sockfd, 1024);
  if (z == -1) {
    printf("listen error");
    exit(1);
  }

  printf("Success!!!\n");

  while(1) {
    connfd = accept(sockfd, (struct sockaddr *)NULL, NULL);  //accept() 是從 queue 裡拿一個 client，並分配一個新 socket 
    if (connfd == -1) {
      printf("connect error");
      exit(1);
    }
 
    printf("Accept Packet\n");

    PID = fork();

    if(PID == -1){
      printf("fork error\n");
      exit(1);
    }
    
    if(!PID){
      if(read(connfd, buffer, pic_len) == -1){
        printf("read error\n");
        exit(1);
      }

      printf("子程序處理...\n");
      printf("%s\n", buffer);

      if(strncmp(buffer, "GET ", 4) == 0 && strstr(buffer, "GET /pic.jpeg")) {
				write(connfd, "HTTP/1.1 200 OK\r\n", (sizeof("HTTP/1.1 200 OK\r\n") / sizeof(char)) - 1);
        write(connfd, "Context-Type: image/jpeg\r\n", (sizeof("Context-Type: image/jpeg\r\n") / sizeof(char)) - 1);
        write(connfd, "\r\n", (sizeof("\r\n") / sizeof(char)) - 1);
				FILE *img;
        char tmp[MAXLINE];
				if((img = fopen("./pic.jpeg", "r")) == NULL) {
					printf("fopen error\n");
					exit(1);
				}

        size_t byte_read;
        while((byte_read = fread(tmp, sizeof(char), MAXLINE, img)) > 0){
          if(write(connfd, tmp, byte_read) < 0){
            printf("ERROR writing: %s\n", strerror(errno));
          }
        }
      }

      else if (strncmp(buffer, "GET ", 4) == 0 && strstr(buffer, "Accept: text/html"))
      {
          write(connfd, "HTTP/1.1 200 OK\r\n", (sizeof("HTTP/1.1 200 OK\r\n") / sizeof(char)) - 1);
          write(connfd, "Context-Type: text/html; charset=utf-8\r\n", (sizeof("Context-Type: text/html; charset=utf-8\r\n") / sizeof(char)) - 1);
          write(connfd, "\r\n", (sizeof("\r\n") / sizeof(char)) - 1);
          FILE *html;
          char tmp[MAXLINE];
          if ((html = fopen("./pic.html", "r")) == NULL){
              printf("ERROR opening: %s\n", strerror(errno));
              exit(1);
          }

          size_t byte_read;
          while ((byte_read = fread(tmp, sizeof(char), MAXLINE, html)) > 0)
          {
              if (write(connfd, tmp, byte_read) < 0)
              {
                  printf("ERROR writing: %s\n", strerror(errno));
              }
          }
      }
      
      
        shutdown(connfd, SHUT_RDWR);
        close(connfd);
        exit(0);
    }
  }
}