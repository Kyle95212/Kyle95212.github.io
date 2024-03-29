#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

#define MAXLINE 512
#define MAX_MEM 10
#define NAME_LEN 100
#define SERV_PORT 8080
#define LISTENQ 5
#define MAX_GAME 5

int listen_fd, connect_fd[MAX_MEM];
int game_state[MAX_MEM];
int board[MAX_MEM][9];
char user[MAX_MEM][NAME_LEN];

void usage(){
	printf("Available number of user is %d\n", MAX_MEM);
	printf("Maximum user name is %d\n", NAME_LEN);
	printf("Server port is set to be: %d\n", SERV_PORT);
	printf("Maximum meg is %d\n\n", MAXLINE);

	printf("Function Intro:\n");
	printf("/q, /quit :quit the server\n");
}

void server_control(){
	char msg[10];

	while(1){
		scanf("%s", msg);
		if(strcmp(msg, "/quit") == 0 || strcmp(msg, "/q") == 0){
			printf("Server closed\n");
			close(listen_fd);
			exit(0);
		}
		else if (strcmp(msg, "/help") == 0)
			usage();
	}
}

int check(int idx){
	int i;
	int b[9];
	for(int i = 0;i < 9;i++) b[i] = board[idx][i];

	if(b[0] == 0 && b[1] == 0 && b[2] == 0) return 1;
	if(b[3] == 0 && b[4] == 0 && b[5] == 0) return 1;
	if(b[6] == 0 && b[7] == 0 && b[8] == 0) return 1;
	if(b[0] == 0 && b[3] == 0 && b[6] == 0) return 1;
	if(b[1] == 0 && b[4] == 0 && b[7] == 0) return 1;
	if(b[2] == 0 && b[5] == 0 && b[8] == 0) return 1;
	if(b[0] == 0 && b[4] == 0 && b[8] == 0) return 1;
	if(b[2] == 0 && b[4] == 0 && b[6] == 0) return 1;	
	return 0;
}

void game(int p1, int p2){
	usleep(100);
	int i, len, move;
	char msg_send[MAXLINE];
	char msg_rcv[MAXLINE];

	char game_wait[] = "<Wait> Wait for another player...\n";
	char game_turn[] = "<Turn> It's your turn\n";
	char game_lose[] = "<Game> You lose\n";
	char game_win[] =  "<Game> You win\n";
	char game_draw[] = "<Game> Draw\n";

	memset(board[p1], -1, sizeof(board[p1]));
	memset(board[p2], -1, sizeof(board[p2]));

	for(i = 0; i < 5; i++){
		memset(msg_rcv, '\0', sizeof(msg_rcv));
		memset(msg_send, '\0', sizeof(msg_send));


		usleep(100);
		send(connect_fd[p1], game_turn, strlen(game_turn), 0);
		send(connect_fd[p2], game_wait, strlen(game_wait), 0);
		usleep(100);

		len = recv(connect_fd[p1], msg_rcv, MAXLINE, 0);
		move = atoi(msg_rcv);
		board[p1][move] = 0;
		board[p2][move] = 1;

		usleep(100);
		sprintf(msg_send, "%d %d %d %d %d %d %d %d %d", board[p1][0], board[p1][1], board[p1][2], board[p1][3], board[p1][4], board[p1][5], board[p1][6], board[p1][7], board[p1][8]);
		send(connect_fd[p1], msg_send, strlen(msg_send), 0);
		sprintf(msg_send, "%d %d %d %d %d %d %d %d %d", board[p2][0], board[p2][1], board[p2][2], board[p2][3], board[p2][4], board[p2][5], board[p2][6], board[p2][7], board[p2][8]);
		send(connect_fd[p2], msg_send, strlen(msg_send), 0);

		usleep(100);
		if(check(p1)){
			send(connect_fd[p1], game_win, strlen(game_win), 0);
			send(connect_fd[p2], game_lose, strlen(game_lose), 0);
			printf(" %s is win, %s is lose\n", user[p1], user[p2]);
			break;
		}

		if(i == 4){
			send(connect_fd[p1], game_draw, strlen(game_draw), 0);
			send(connect_fd[p2], game_draw, strlen(game_draw), 0);
			break;
		}

		// player 2's turn
		send(connect_fd[p1], game_wait, strlen(game_wait), 0);
		send(connect_fd[p2], game_turn, strlen(game_turn), 0);

		len = recv(connect_fd[p2], msg_rcv, MAXLINE, 0);
		move = atoi(msg_rcv);
		board[p1][move] = 1;
		board[p2][move] = 0;
		
		usleep(100);
		sprintf(msg_send, "%d %d %d %d %d %d %d %d %d", board[p1][0], board[p1][1], board[p1][2], board[p1][3], board[p1][4], board[p1][5], board[p1][6], board[p1][7], board[p1][8]);
		send(connect_fd[p1], msg_send, strlen(msg_send), 0);
		sprintf(msg_send, "%d %d %d %d %d %d %d %d %d", board[p2][0], board[p2][1], board[p2][2], board[p2][3], board[p2][4], board[p2][5], board[p2][6], board[p2][7], board[p2][8]);
		send(connect_fd[p2], msg_send, strlen(msg_send), 0);
		//sprintf(msg_send, "%d %d %d %d %d %d %d %d %d", board[p1][0], board[p1][1], board[p1][2], board[p1][3], board[p1][4], board[p1][5], board[p1][6], board[p1][7], board[p1][8]);
		

		usleep(100);
		if(check(p2)){
			send(connect_fd[p1], game_lose, strlen(game_lose), 0);
			send(connect_fd[p2], game_win, strlen(game_win), 0);
			printf(" %s is win, %s is lose\n", user[p2], user[p1]);
			break;
		}
		// send(connect_fd[p1], msg_send, strlen(msg_send), 0);
	}

	game_state[p1] = -1;
	game_state[p2] = -1;
	printf("game terminated ...\n");
}

void receive_send(int n){
	char msg_notify[MAXLINE];
	char msg_rcv[MAXLINE];
	char msg_send[MAXLINE * 2];
	char target_user[MAXLINE];
	char user_name[NAME_LEN];
	char message[MAXLINE];

	char msg1[] 			= " <Server> Who do you want to send?\n";
	char msg_play_with[] 	= " <Server> Which player do you want to play with: \n";
	char msg_reply[] 		= " <Server> Enter Y or y to accept the game with";
	char msg_reply_reject[] = " <Server> Target player rejected\n";
	char msg_reply_accept[] = " <Server> Target player accept, game start\n";

	char game_start[] 		= "<Game> Game created!\n";

	int i = 0;
	int target_idx;
	int retval;
	memset(user_name, '\0', sizeof(user_name));

	// receiving user name from client
	int length = recv(connect_fd[n], user_name, NAME_LEN, 0);
	if(length > 0){								  
		// receiving succeeded
		if(user_name[length - 1] == '\n') user_name[length - 1] = '\0'; // char is 1 byte
		strcpy(user[n], user_name);
		printf("User name : %s\n", user[n]);
	}

	// receiving message from client
	while(1){
		memset(msg_rcv, '\0', sizeof(msg_rcv));
		memset(msg_send, '\0', sizeof(msg_send));
		memset(message, '\0', sizeof(message));
		target_idx = -1;
		if(game_state[n] != -1){
			usleep(1);
			continue;
		}

		length = recv(connect_fd[n], msg_rcv, MAXLINE, 0);
		if(length > 0){ 
			//Message received
			msg_rcv[length] = 0;
			
			if((strcmp(msg_rcv, "y") == 0 || strcmp(msg_rcv, "Y") == 0) && (game_state[n] != -1)){
				send(connect_fd[game_state[n]], msg_rcv, strlen(msg_rcv), 0);
			}
			// quit
			if(strncmp(msg_rcv, "/quit", 5) == 0 || strncmp(msg_rcv, "/q", 2) == 0){
				printf("%s quitted\n", user_name);
				close(connect_fd[n]);
				connect_fd[n] = -1;
				pthread_exit(&retval);
			}
			// list client name
			else if(strncmp(msg_rcv, "/list", 5) == 0 || strncmp(msg_rcv, "/ls", 3) == 0){
				strcpy(msg_send, "\n<Server> Online:\n");
				for(i = 0; i < MAX_MEM; i++){
					if(connect_fd[i] != -1){
						strcat(msg_send, user[i]);
						strcat(msg_send, "\n");
					}
				}
				send(connect_fd[n], msg_send, strlen(msg_send), 0);
			}
			// talk to specific user
			else if(strncmp(msg_rcv, "/chat", 5) == 0){
				printf("\nprivate message from %s ...\n", user_name);
				// ask for target user's name
				send(connect_fd[n], msg1, strlen(msg1), 0);
				length = recv(connect_fd[n], target_user, MAXLINE, 0);
				target_user[length - 1] = '\0'; // '\n' to '\0'
				sprintf(msg_send, ">> %s : ", target_user);
				send(connect_fd[n], msg_send, strlen(msg_send), 0);

				// get the private message
				length = recv(connect_fd[n], message, MAXLINE, 0);
				message[length] = '\0';
				sprintf(msg_send, " <Private> %s : %s", user_name, message);

				//send private message
				for(i = 0; i < MAX_MEM; i++){
					if(connect_fd[i] != -1 && strncmp(target_user, user[i], strlen(target_user)) == 0)
						send(connect_fd[i], msg_send, strlen(msg_send), 0);
				}
			}

			else if(strncmp(msg_rcv, "/all", 4) == 0){
				printf("\npublic message from %s ...\n", user_name);
				sprintf(msg_send, ">> all : ");
				send(connect_fd[n], msg_send, strlen(msg_send), 0);

				// get the public message
				length = recv(connect_fd[n], message, MAXLINE, 0);
				message[length] = '\0';
				sprintf(msg_send, " <From> %s <to all> : %s", user_name, message);

				//send private message
				for(i = 0; i < MAX_MEM; i++){
					if(strncmp(user_name, user[i], strlen(user_name)) == 0)
						continue;
					else
						send(connect_fd[i], msg_send, strlen(msg_send), 0);
				}
			}

			// tic tac toe
			else if(strncmp(msg_rcv, "/chess", 6) == 0){
				memset(game_state, -1, sizeof(game_state));
				printf("\n %s create a game with ", user_name);

				// ask for target user's name
				send(connect_fd[n], msg_play_with, strlen(msg_play_with), 0);
				length = recv(connect_fd[n], target_user, MAXLINE, 0);
				target_user[length - 1] = '\0';
				printf("%s...\n", target_user);

				// wait for target user acception
				sprintf(msg_send, "%s %s ?\n", msg_reply, user_name);
				for(i = 0; i < MAX_MEM; i++){
					if(connect_fd[i] != -1 && strncmp(target_user, user[i], strlen(target_user)) == 0){
						// check game state
						if(game_state[n] == -1 && game_state[i] == -1){
							printf("Target player founding...\n");
							send(connect_fd[i], msg_send, strlen(msg_send), 0);
							target_idx = i;
							game_state[n] = target_idx;
							game_state[target_idx] = n;
							break;
						}
						else{
							printf("Player is playing now...\n");
							break;
						}
					}
				}
				if(game_state[n] == -1){
					printf("Game failed\n");
					continue;
				}
				// reply from target user
				length = recv(connect_fd[target_idx], message, MAXLINE, 0);
				message[length] = '\0'; // fgets '\n' ?

				// reply response to user
				if(strncmp(message, "y", 1) != 0 && strncmp(message, "Y", 1) != 0){
					printf("Player %s reject...\n", target_user);
					game_state[n] = -1;
					game_state[target_idx] = -1;
					send(connect_fd[n], msg_reply_reject, strlen(msg_reply_reject), 0);
					continue;
				}
				printf("Game start ... %s vs %s\n", user_name, target_user);
				send(connect_fd[n], game_start, strlen(game_start), 0);
				send(connect_fd[target_idx], game_start, strlen(game_start), 0);
				game_state[n] = target_idx;
				game_state[target_idx] = n;
				game(n, target_idx);
				printf("%s vs %s Game terminated ...\n", user_name, target_user);
			}
		}
	}
}

int main(){
	memset(game_state, -1, sizeof(game_state));

	long i;
	pthread_t thread;
	struct sockaddr_in server_addr, client_addr;
	socklen_t sock_len; // length
	char buff[MAXLINE];

	// Create server_fd by socket
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_fd < 0){
		printf("Creating socket failed\n");
		return -1;
	}

	// Internet connection setting
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERV_PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// notify by bind
	if(bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		printf("Bind failed\n");
		return -1;
	}

	// listen
	printf("Listening ...\n");
	listen(listen_fd, LISTENQ);

	// create thread to control server
	pthread_create(&thread, NULL, (void *)(&server_control), NULL);
	
	// initialize connet_fd
	for(i = 0; i < MAX_MEM; i++){
		connect_fd[i] = -1;
	}
	memset(user, '\0', sizeof(user));
	printf("Initialize ...\n");

	// wait for client
	while(1){
		sock_len = sizeof(client_addr);
		for (i = 0; (i < MAX_MEM) && (connect_fd[i] != -1); i++)
			; // look for free connect_fd
		connect_fd[i] = accept(listen_fd, (struct sockaddr *)&client_addr, &sock_len);

		// create threads for client argument = i to control connect_fd
		printf("\nWaiting for user name...\n");
		pthread_create(malloc(sizeof(pthread_t)), NULL, (void *)(&receive_send), (void *)i);
	}
	return 0;
}
