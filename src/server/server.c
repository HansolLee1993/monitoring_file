#include "../conversion.c"
#include "../util.c"
#include "../error.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

struct serverOptions
{
    char *ip_in;
    in_port_t port;
};

static void options_init(struct serverOptions *opts);
static void parse_server_arguments(int argc, char *argv[], struct serverOptions *opts);
static int log_message(int newSocket, struct serverOptions *opts);
static void set_signal_handler();
static void handle_ctrlC(int signum);

#define SIZE 1024
#define DEFAULT_LOG_FILE_LOCATION "../server"
#define DEFAULT_PORT 5000
#define MAX_PENDING 10

int server_socket;
int client_socket;

int main (int argc, char *argv[]) {
    struct serverOptions opts;

    options_init(&opts);
    parse_server_arguments(argc, argv, &opts);
    set_signal_handler();

    int ret;
    struct sockaddr_in serverAddr;
    struct sockaddr_in newAddr;

    socklen_t addr_size;

    pid_t childip;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        error_errno(__FILE__, __func__ , __LINE__, errno, 2);
    }
    printf("[+]Server socket created.\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(opts.port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    //bind ip address to specific port
    ret = bind(server_socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (ret < 0) {
        error_errno(__FILE__, __func__ , __LINE__, errno, 2);
    }

    if (listen(server_socket, MAX_PENDING) == 0)
        printf("Listening...\n");
    else
        error_errno(__FILE__, __func__ , __LINE__, errno, 2);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&newAddr, &addr_size);

        if (client_socket < 0)
            error_errno(__FILE__, __func__ , __LINE__, errno, 2);

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &newAddr.sin_addr, client_ip, sizeof(client_ip));
        opts.ip_in = inet_ntoa(newAddr.sin_addr);
        printf("[+]Connection accept from %s:%d\n", opts.ip_in, opts.port);

        if ((childip = fork()) == 0) {
            close(server_socket);

            if (log_message(client_socket, &opts) == 0) {
                break;
            }
        }
    }

    close(client_socket);
    return EXIT_SUCCESS;
}

static void set_signal_handler() {
    if (signal(SIGINT, handle_ctrlC) == SIG_ERR) {
        fprintf(stderr, "Unable to register signal handler\n");
        perror("Unable to register signal handler\n");
    }
}

static void parse_server_arguments(int argc, char *argv[], struct serverOptions *opts) {
    int c;
    while ((c = getopt(argc, argv, "p:")) != -1) {
        switch(c) {
            case 'p':
            {
                opts->port = parse_port(optarg, 10);
                break;
            }
            case ':':
            {
                error_message(__FILE__, __func__ , __LINE__, "\"Option requires an operand\"", 5);
            }
            case '?':
            {
                error_message(__FILE__, __func__ , __LINE__, "Unknown", 6);
            }
        }
    }
    printf("[+]Port: %hu\n", opts->port);
}

static void handle_ctrlC(int signum) {
    printf("Ctrl+C detected. Exiting...\n");

    // Close the socket
    if (close(server_socket) == -1) {
        perror("Error closing socket");
    }
    // Terminate the program
    exit(signum);
}

static int log_message(int newSocket, struct serverOptions *opts) {
    char postFix[] = ".txt";

    // set log file name
    char filename[strlen(opts->ip_in) + strlen(postFix) + 1];  // Adjust the size as needed
    strcpy(filename, opts->ip_in);
    strcat(filename, postFix);
    FILE *file;
    char message[SIZE];

    while (1) {
        ssize_t bytes_received = recv(newSocket, message, sizeof(message), 0);
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                printf("Client disconnected.\n");
            } else {
                perror("Error receiving data");
            }

            // Cleanup and close the socket
            close(client_socket);
            close(server_socket);
            fclose(file);
            exit(EXIT_SUCCESS);
        }

        if (strcmp(message, "start") == 0) {
            remove(filename);
            file = fopen(filename, "a");
            continue;
        }
        message[bytes_received] = '\0';
        printf("Received message from client :%s\n", message);

        // Write the log message to the log file
        fprintf(file, "%s", message);
        fflush(file);
    }
    fclose(file);
    return EXIT_SUCCESS;
}

static void options_init(struct serverOptions *opts) {
    memset(opts, 0, sizeof(struct serverOptions));
    opts->port  = DEFAULT_PORT;
}