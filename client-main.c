// CREATED BY
// Bernard Wijaya - Z120140
// Winston Rusli  - Z120215

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int readInput_withTimeout(int seconds, char** str_buffer){
    // Reference from: https://stackoverflow.com/questions/7226603/timeout-function
    char in_buffer[BUFSIZ];

    fd_set fileDesc_set;
    // Emptying fileDesc_set
    FD_ZERO(&fileDesc_set);
    // Adds File Descriptor '0' to fileDesc_set
    FD_SET(0, &fileDesc_set);

    // Setting timeout
    struct timeval timeout;
    /* Waiting for some seconds */
    timeout.tv_sec  = seconds;    // WAIT seconds
    timeout.tv_usec = 0;    // 0 milliseconds

    printf("Play Style:\n");
    printf("[1] Offensive (130%% Att & 70%% Def)\n");
    printf("[2] Defensive (130%% Def & 70%% Att)\n");
    printf("Input Format: [Style Answer]\n");
    printf("Input your answer in (%d) second(s): ", seconds);
    fflush(stdout);

    // Listen for any input stream activity
    int read_ready = 0;
    int bytes_read = 0;
    read_ready = select(1, &fileDesc_set, NULL, NULL, &timeout);

    if(read_ready == -1){
        printf("\nInput Error!\n\n");
        fflush(stdout);
        return -1;
    }
    if(!read_ready){
        printf("\nYou missed your chance to answer!\n\n");
        fflush(stdout);
    }
    else{
        bytes_read = read(0, in_buffer, BUFSIZ-1);
        if(in_buffer[bytes_read-1] == '\n'){
            --bytes_read;
            in_buffer[bytes_read] = '\0';
            *str_buffer = in_buffer;
        }
    }

    return bytes_read;
}

void matchmake(){
    char server_ip[] = "127.0.0.1";
    int server_port = 12345;
    printf("Server IP   : %s\n", server_ip);
    fflush(stdout);
    printf("Server Port : %d\n\n", server_port);
    fflush(stdout);

    int socket_fd;
    if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("\nSocket failed to be created!!!\n");
        exit(1);
    }
    printf("socket_fd: %d\n", socket_fd);

    struct sockaddr_in server_sockadd;
    server_sockadd.sin_family = PF_INET;
    server_sockadd.sin_port = htons(server_port);
    inet_aton(server_ip, &server_sockadd.sin_addr);
    
    if (connect (socket_fd, (struct sockaddr *)&server_sockadd, sizeof(server_sockadd)) < 0) {
        perror("\nAttempted connection to server failed!\n");
        exit(1);
    }
    printf("Connected to server!\n");
    fflush(stdout);

    int length;
    char send_buffer[BUFSIZ];
    char recv_buffer[BUFSIZ];
    while((length = recv (socket_fd, recv_buffer, BUFSIZ, 0)) < 0){
        perror("Failed to receive Player ID!");
        return;
    };
    recv_buffer[length] = '\0';
    printf("Your Player ID: %s\n", recv_buffer);
    fflush(stdout);

    memset(recv_buffer, 0, BUFSIZ);
    printf("Matchmaking ongoing...\n\n");
    fflush(stdout);
    while((length = recv (socket_fd, recv_buffer, BUFSIZ, 0)) < 0){
        perror("Failed to receive match ID and Opponent's ID");
        return;
    };
    
    recv_buffer[length] = '\0';
    
    char* match_id;
    match_id = strtok(recv_buffer, "|");
    char * player_id;
    player_id = strtok(NULL, "|");
    
    printf("Match Created!\n");
    fflush(stdout);
    printf("Match ID: %s\n", match_id);
    fflush(stdout);
    printf("Opponent's Player ID: %s\n\n", player_id);
    fflush(stdout);
    
    memset(send_buffer, 0, BUFSIZ);
    sprintf(send_buffer, "Received the IDs");
    if(send(socket_fd, send_buffer, strlen(send_buffer), 0) < 0){
        perror("Failed to send ID received confirmation!\n");
    };

    int my_health = 100;
    int opponent_health = 100;
    int own_damage = 0;
    int opp_damage = 0;
    char *problem;
    while(my_health > 0 && opponent_health > 0){
        fflush(stdout);

        memset(recv_buffer, 0, BUFSIZ);
        while((length = recv (socket_fd, recv_buffer, BUFSIZ, 0)) < 0){
            perror("Failed to receive damage history!\n");
            return;
        };
        recv_buffer[length] = '\0';
        memset(send_buffer, 0, BUFSIZ);
        strcpy(send_buffer, "1");
        if(send(socket_fd, send_buffer, strlen(send_buffer), 0) < 0){
            perror("Failed to send damage history confirmation!\n");
            continue;
        }
        own_damage = opp_damage = 0;
        if(strcmp(strtok(recv_buffer, "|"), "1") == 0){
            own_damage = (int)strtol(strtok(NULL, "|"), NULL, 10);
            opp_damage = (int)strtol(strtok(NULL, "|"), NULL, 10);
            printf("======= Damage History =======\n");
            printf("You dealt %d damage to your opponent!\n", own_damage);
            printf("Your opponent dealt you %d damage!\n", opp_damage);
            printf("==============================\n\n");
            fflush(stdout);
        }

        memset(recv_buffer, 0, BUFSIZ);
        while((length = recv (socket_fd, recv_buffer, BUFSIZ, 0)) < 0){
            perror("Failed to receive information!\n");
            return;
        };
        recv_buffer[length] = '\0';
        int turn_bool = strtol(strtok(recv_buffer, "|"), NULL, 10);
        if (turn_bool == 1){
            //Active turn
            printf("====== Your Turn! ======\n");
            fflush(stdout);
            my_health = strtol(strtok(NULL, "|"), NULL, 10);
            opponent_health = strtol(strtok(NULL, "|"), NULL, 10);
            problem = strtok(NULL, "|");
            
            if(my_health < 0 || opponent_health < 0){
                if(my_health < 0){
                    printf("Your Health: %d | Opponent's Health: %d\n", 0, opponent_health);
                }
                else{
                    printf("Your Health: %d | Opponent's Health: %d\n", my_health, 0);
                }
            }
            else{
                printf("Your Health: %d | Opponent's Health: %d\n\n", my_health, opponent_health);
            }
            fflush(stdout);
            printf("%s\n", problem);
            fflush(stdout);
            
            char* answer_str;

            if(my_health < 0 || opponent_health < 0){
                answer_str  = "0 -999";
            }
            else{
                if(readInput_withTimeout(15, &answer_str) == 0){
                    answer_str  = "0 -999";
                }
                printf("You inputted: %s\n", answer_str);
                fflush(stdout);
            }
            
            memset(send_buffer, 0, BUFSIZ);
            strcpy(send_buffer, answer_str);
            if(send(socket_fd, send_buffer, strlen(send_buffer), 0) < 0){
                perror("Failed to send answer!\n");
                continue;
            }
            
            memset(recv_buffer, 0, BUFSIZ);
            // Evaluating server response time
            clock_t begin = clock();
	        while((length = recv (socket_fd, recv_buffer, BUFSIZ, 0)) < 0){
	            perror("Failed to receive correction!\n");
	            return;
	        };
            clock_t end = clock();
            double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
            printf("Server Response Time: %lf second(s)\n", time_spent);
            // End of evaluation
	        recv_buffer[length] = '\0';
	        fflush(stdout);
	        if (strcasecmp(recv_buffer, "true") == 0){
	        	printf("You were correct!!\n");
        	    fflush(stdout);
			} else if (strcasecmp(recv_buffer, "false") == 0){
				printf("You were incorrect!!\n");
        	    fflush(stdout);
			}
            memset(send_buffer, 0, BUFSIZ);
            strcpy(send_buffer, "1");
            if(send(socket_fd, send_buffer, strlen(send_buffer), 0) < 0){
                perror("Failed to send correction confirmation!\n");
                continue;
            }
            printf("===============================\n");
            fflush(stdout);
        } else if (turn_bool == 0) {
            //Waiting for other player
            printf("====== Opponent's' Turn! ======\n");
            fflush(stdout);
            if (length > 1){
                my_health = strtol(strtok(NULL, "|"), NULL, 10);
                opponent_health = strtol(strtok(NULL, "|"), NULL, 10);
                printf("Your Health: %d | Opponent's Health: %d\n\n", my_health, opponent_health);
                fflush(stdout);
            }
            printf("Please wait for the opponent's turn to finish...\n");
            fflush(stdout);
            printf("===============================\n");
            fflush(stdout);
        }
        printf("Local Data: %d %d %d %d", my_health, opp_damage, opponent_health, own_damage);
        printf("\n\n");
        fflush(stdout);
        if(my_health - opp_damage < 0 || opponent_health - own_damage < 0){
            break;
        }
    }

    if(my_health < 0){
        printf("\n\n\n==========================\n");
        printf("You Lost! GOOD LUCK NEXT TIME!\n");
        printf("==========================\n");
    }
    else{
        printf("\n\n\n==========================\n");
        printf("Congratulations! YOU WON!!!\n");
        printf("==========================\n");
    }
    fflush(stdout);

    sleep(2);
    close(socket_fd);
}

int main(){
    printf("Hello there, player!\n\n");

    matchmake();
    return 0;
}
