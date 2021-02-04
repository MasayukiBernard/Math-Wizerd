// CREATED BY
// Bernard Wijaya - Z120140
// Winston Rusli  - Z120215
// gcc -pthread -D_REENTRANT -o server-main server-main.c
#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define CLIENT_LIMIT 100

int socket_fd;
struct sockaddr_in client_sockadd[CLIENT_LIMIT];
int new_socket_fd[CLIENT_LIMIT];
int sin_siz = sizeof(struct sockaddr_in);
long num_of_clients = 0;
long match_ids = 0;

void generate_problem(int *operand_1, int *operand_2, int *result, char *operator_type){
    char operator_types[4] = {'+', '-', '*'};
    *operand_1 = (rand() % 100) + 1;
    *operand_2 = (rand() % 100) + 1;
    *operator_type = operator_types[rand() % 3];
    switch(*operator_type){
        case '+':
            *result = *operand_1 + *operand_2;
            break;
        case '-':
            *result = *operand_1 - *operand_2;
            break;
        case '*':
            *result = *operand_1 * *operand_2;
            break;
    }
    printf("Result should be: %d\n", *result);
    fflush(stdout);
}

void make_match(long current_client_id){
    char send_buffer[BUFSIZ];
    char recv_buffer[BUFSIZ];
    
    long player_ID[2] = {current_client_id-1, current_client_id};
    int player_count = 2;
    int player_turn = 0;
    int player_wait = 0;
    int turn_count = 0;
    
    printf("New Match Created:\n");
    printf("Match ID: %ld\n", match_ids);
    printf("Port 1: %d | Port 2: %d\n", ntohs(client_sockadd[player_ID[0]].sin_port), ntohs(client_sockadd[player_ID[1]].sin_port));
    printf("Player ID 1: %ld | Player ID 2: %ld\n\n", player_ID[0], player_ID[1]);

    int length;
    // Sends match ID and opponent's ID to both clients
    sprintf(send_buffer, "%ld|%ld", match_ids, player_ID[1]);
    if((length = send(new_socket_fd[player_ID[0]], send_buffer, strlen(send_buffer), 0)) < 0){
        perror("Failed to send match ID and Opponent's ID!");
        return;
    };
    printf("Sent %d bytes to Player ID '%ld'\n", length, player_ID[0]+1);
    fflush(stdout);
    memset(recv_buffer, 0, BUFSIZ);
    if(recv (new_socket_fd[player_ID[0]], recv_buffer, BUFSIZ, 0) < 0){
        perror("Failed to receive ID confirmation!\n");
    }
    memset(send_buffer, 0, BUFSIZ);
    sprintf(send_buffer, "%ld|%ld", match_ids, player_ID[0]);
    if((length = send(new_socket_fd[player_ID[1]], send_buffer, strlen(send_buffer), 0)) < 0){
        perror("Failed to send match ID and Opponent's ID!");
        return;
    };
    printf("Sent %d bytes to Player ID '%ld'\n", length, player_ID[1]+1);
    fflush(stdout);
    memset(recv_buffer, 0, BUFSIZ);
    if(recv (new_socket_fd[player_ID[1]], recv_buffer, BUFSIZ, 0) < 0){
        perror("Failed to receive ID confirmation!\n");
    }
    match_ids++;
    
    int player_health[2] = {100, 100};
    int player_style[2] = {0, 0};
    int result = 200;
    // 0 False, 1 True
    int player_result[2] = {0, 0};
    while (player_health[0] > 0 && player_health[1] > 0){
        // Determine which player's turn is it
        player_turn = turn_count % player_count;
        player_wait = (turn_count + 1) % player_count;
        printf("\n====== Player %d\'s turn ======\n", player_turn+1);
        
        int operand_1 = 100;
        int operand_2 = 100;
        char operator_type;
        
        // Send damage update
        // First byte is boolean; 1 to send damage update; 0 to not send
        if(player_style[0] != 0 && player_style[1] != 0){
            // Style damage dealing calculation
            int damage[2] = {10, 10};
            int absorb_damage[2] = {0, 0};
            int total_damage[2];
            int i;
            for (i = 0; i < 2; ++i){
                if(player_style[i] == 1){
                    damage[i] = damage[i] + (damage[i] * 0.3);
                }
                else if (player_style[i] == 2){
                    damage[i] = damage[i] - (damage[i] * 0.3);
                }
            }
            
            if(player_style[0] == 1){
                absorb_damage[0] = damage[1] * 0.3 * -1;
            }
            else if (player_style[0] == 2 && player_result[0]){
                absorb_damage[0] = damage[1] * 0.3;
            }
            if(player_style[1] == 1){
                absorb_damage[1] = damage[0] * 0.3 * -1;
            }
            else if (player_style[1] == 2 && player_result[1]){
                absorb_damage[1] = damage[0] * 0.3;
            }
            
            if(player_style[0] != -1 && player_result[0]){
                total_damage[0] = damage[0] - absorb_damage[1];
                player_health[1] = player_health[1] - total_damage[0];
            }
            else{
                total_damage[0] = 0;
            }
            if(player_style[1] != -1 && player_result[1]){
                total_damage[1] = damage[1] - absorb_damage[0];
                player_health[0] = player_health[0] - total_damage[1];
            }
            else{
                total_damage[1] = 0;
            }
            memset(send_buffer, 0, BUFSIZ);
            sprintf(send_buffer, "1|%d|%d", total_damage[0], total_damage[1]);
            if((length = send(new_socket_fd[player_ID[0]], send_buffer, strlen(send_buffer), 0)) < 0){
                perror("Failed to send damage calculation!");
            };
            printf("Sent %d bytes to Player ID '%ld'\n", length, player_ID[0]);
            fflush(stdout);
            memset(recv_buffer, 0, BUFSIZ);
            while((length = recv (new_socket_fd[player_ID[0]], recv_buffer, BUFSIZ, 0)) < 0){
                perror("Failed to receive damage calculation confirmation!");
            }
            
            memset(send_buffer, 0, BUFSIZ);
            sprintf(send_buffer, "1|%d|%d", total_damage[1], total_damage[0]);
            if((length = send(new_socket_fd[player_ID[1]], send_buffer, strlen(send_buffer), 0)) < 0){
                perror("Failed to send damage calculation!");
            };
            printf("Sent %d bytes to Player ID '%ld'\n", length, player_ID[1]);
            fflush(stdout);
            memset(recv_buffer, 0, BUFSIZ);
            while((length = recv (new_socket_fd[player_ID[1]], recv_buffer, BUFSIZ, 0)) < 0){
                perror("Failed to receive damage calculation confirmation!");
            }
            
            // Reset players' style
            player_style[0] = player_style[1] = 0;
        }
        else{
            int i;
            for(i = 0; i < 2; ++i){
                memset(send_buffer, 0, BUFSIZ);
                sprintf(send_buffer, "0|0|0");
                if((length = send(new_socket_fd[player_ID[i]], send_buffer, strlen(send_buffer), 0)) < 0){
                    perror("Failed to send damage calculation!");
                };
                printf("Sent %d bytes to Player ID '%ld'\n", length, player_ID[player_turn]+1);
                fflush(stdout);
                memset(recv_buffer, 0, BUFSIZ);
                while((length = recv (new_socket_fd[player_ID[i]], recv_buffer, BUFSIZ, 0)) < 0){
                    perror("Failed to receive damage calculation confirmation!");
                }
            }
        }

        generate_problem(&operand_1, &operand_2, &result, &operator_type);
        // Send health update and problem to player with active turn. 
        // First byte is boolean; 1 if active turn, 0 if waiting
        memset(send_buffer, 0, BUFSIZ);
        sprintf(send_buffer, "1|%d|%d|%d %c %d", player_health[player_turn], player_health[player_wait], operand_1, operator_type, operand_2);
        if((length = send(new_socket_fd[player_ID[player_turn]], send_buffer, strlen(send_buffer), 0)) < 0){
            perror("Failed to send health and problem to player with active turn!");
        };
        printf("Sent %d bytes to Player ID '%ld'\n", length, player_ID[player_turn]+1);
        fflush(stdout);
        
        // Tell the other player to wait. If it's the first turn, send health update.
        // First byte is boolean; 1 if active turn, 0 if waiting
        memset(send_buffer, 0, BUFSIZ);
        if (turn_count == 0){
            sprintf(send_buffer, "0|%d|%d", player_health[player_wait], player_health[player_turn]);
            if((length = send(new_socket_fd[player_ID[player_wait]], send_buffer, strlen(send_buffer), 0)) < 0){
                perror("Failed to send health to waiting player!");
            };
        } else {
            sprintf(send_buffer, "0");
            if((length = send(new_socket_fd[player_ID[player_wait]], send_buffer, strlen(send_buffer), 0)) < 0){
                perror("Failed to tell player to wait!");
            }
        }
        printf("Sent %d bytes to Player ID '%ld'\n", length, player_ID[player_wait]+1);
        fflush(stdout);

        int length;
        int result_player = -999;
        int style_player = 0;
        memset(recv_buffer, 0, BUFSIZ);
        while((length = recv (new_socket_fd[player_ID[player_turn]], recv_buffer, BUFSIZ, 0)) < 0){
            perror("Failed to receive problem's answer!");
        }
        if (length > 0){
            recv_buffer[length] = '\0';
            char* token;
            token = strtok(recv_buffer, " ");
            style_player = (int)strtol(token, NULL, 10);
            fflush(stdout);
            if (style_player < 1 || style_player > 2){
                style_player = -1;
            }
            player_style[player_turn] = style_player;

            token = strtok(NULL, " ");
            if(token != NULL){
                result_player = (int)strtol(token, NULL, 10);
            }
            printf("Answer: %d\n", result_player);
            printf("Style: %d\n", style_player);
            fflush(stdout);
        }
        
        memset(send_buffer, 0, BUFSIZ);
        if (result == result_player){
            player_result[player_turn] = 1;
	        sprintf(send_buffer, "true");
        } else {
            player_result[player_turn] = 0;
        	sprintf(send_buffer, "false");
		}
        if((length = send(new_socket_fd[player_ID[player_turn]], send_buffer, strlen(send_buffer), 0)) < 0){
            perror("Failed to send correction");
        };
        memset(recv_buffer, 0, BUFSIZ);
        while((length = recv (new_socket_fd[player_ID[player_turn]], recv_buffer, BUFSIZ, 0)) < 0){
            perror("Failed to receive correction confirmation!");
        }
        printf("Sent %d bytes to Player ID '%ld'\n", length, player_ID[player_turn]+1);
        fflush(stdout);
        turn_count++;
    }
}

void accept_connection(){
    // For accepting connection to new clients
    char buffer[BUFSIZ];
    pthread_t ongoing_matches[50];
    int total_matches = 0;

    while(1){
        if((new_socket_fd[num_of_clients] = accept(socket_fd, (struct sockaddr *)&client_sockadd[num_of_clients], &sin_siz)) < 0) {
            perror("Failed to accept incoming request!");
        }
        printf("New Connection Accepted:\n");
        printf("Socket FD: '%d'\n", new_socket_fd[num_of_clients]);
        printf("Port: %d\n\n", ntohs(client_sockadd[num_of_clients].sin_port));
        num_of_clients++;

        // Sends Player ID
        memset(buffer, 0, BUFSIZ);
        sprintf(buffer, "%ld", num_of_clients-1);
        send(new_socket_fd[num_of_clients-1], buffer, strlen(buffer), 0);

        if(num_of_clients > 0 && (num_of_clients % 2) == 0){
            // Creates a new thread for matchmaking between currently accepted client with previous one
            pthread_create(&ongoing_matches[total_matches++], NULL, (void*)make_match, (void*)(num_of_clients-1));
        }
    }
    
    int i;
    for(i = 0; i < total_matches; ++i){
        pthread_join(ongoing_matches[i], NULL);
    }

    return;
}

int main(){
    char server_ip[] = "127.0.0.1";
    int server_port = 12345;
    srand(time(NULL));
    printf("Hello there, administrator!\n\n");
    printf("Server IP   : %s\n", server_ip);
    printf("Server Port : %d\n\n", server_port);

    printf("Status Histories:\n");
    printf("Game Server Started!\n");
    printf("Initializing Game Server...\n");
    
    if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("\nSocket failed to be created!!!\n");
        exit(1);
    }
    printf("Socket has been succesfully created!\n");

    struct sockaddr_in server_sockadd;
    server_sockadd.sin_family = PF_INET;
    server_sockadd.sin_port = htons(server_port);
    inet_aton(server_ip, &server_sockadd.sin_addr);
    printf("Server Socket Address Describer Created!\n");

    if (bind(socket_fd, (struct sockaddr*)&server_sockadd, sizeof(server_sockadd)) < 0) {
        perror("\nSocket file descriptor binding with address describer failed!\n");
        exit(1);
    }
    printf("Socket File Descriptor Binded with Socket Address Describer!\n");

    if(listen(socket_fd, SOMAXCONN) < 0) {
        perror("\nSocket failed to listen to incoming connections\n");
        exit(1);
    }
    printf("Socket is Listening to Incoming Connections!\n");
    printf("Game Server Initialized!!\n\n");

    accept_connection();

    printf("Server Program Ends!");
    close(socket_fd);

    return 0;
}
