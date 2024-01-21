#include "../conversion.c"
#include "../error.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <sys/inotify.h>

struct clientOptions
{
    char *server_ip;
    in_port_t port;
};

static void options_init(struct clientOptions *opts);
static void parse_client_arguments(int argc, char *argv[], struct clientOptions *opts);
static void monitor_file(int clientSocket);
static void send_message(int clientSocket, char *message);

#define DEFAULT_PORT 5000
#define SIZE 1024
#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))

int main (int argc, char *argv[]) {
    struct clientOptions opts;

    options_init(&opts);
    parse_client_arguments(argc, argv, &opts);

    int clientSocket, ret;
    struct sockaddr_in serverAddr;

    FILE *fp;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        error_errno(__FILE__, __func__ , __LINE__, errno, 2);
    }
    printf("[+]Client socket is created.\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(opts.port);
    serverAddr.sin_addr.s_addr = inet_addr(opts.server_ip);

    ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (ret < 0) {
        error_errno(__FILE__, __func__ , __LINE__, errno, 2);
    }
    printf("[+]Connect to Server.\n");

    monitor_file(clientSocket);

    close(clientSocket);

    return EXIT_SUCCESS;
}

void monitor_file(int clientSocket) {
    int length, i = 0;
    int fd;
    int wd;
    char buffer[EVENT_BUF_LEN];

    // Initialize inotify
    fd = inotify_init();
    if (fd < 0) {
        perror("Error: fail on inotify_init");
        return;
    }

    // Add watch for /etc/shadow
    wd = inotify_add_watch(fd, "/etc", IN_MODIFY | IN_DELETE | IN_CREATE);
    if (wd == -1) {
        perror("Error: fail on inotify_add_watch");
        return;
    }

    printf("Monitoring /etc/shadow for changes...\n");

    while (1) {
        // Read events
        length = read(fd, buffer, EVENT_BUF_LEN);
        if (length < 0) {
            perror("Error: fail on read buffer");
        }

        // Process events
        while (i < length) {
            struct inotify_event* event = (struct inotify_event*)&buffer[i];
            if (event->len) {
                if (event->mask & IN_CREATE) {
                    printf("File %s created.\n", event->name);
                    send_message(clientSocket, "File created.\n");
                } else if (event->mask & IN_DELETE) {
                    printf("File %s deleted.\n", event->name);
                    send_message(clientSocket, "File deleted.\n");
                } else if (event->mask & IN_MODIFY) {
                    printf("File %s modified.\n", event->name);
                    send_message(clientSocket, "File modified.\n");
                }
            }
            i += EVENT_SIZE + event->len;
        }
        i = 0;
    }

    // Clean up
    inotify_rm_watch(fd, wd);
    close(fd);
}

static void send_message(int clientSocket, char *message){
    // Send data to the server
    ssize_t bytes_sent = send(clientSocket, message, strlen(message), 0);
    if (bytes_sent == -1) {
        perror("Error sending data");
        exit(EXIT_FAILURE);
    }
    printf("Message sent to server: %s\n", message);
}

static void options_init(struct clientOptions *opts)
{
    memset(opts, 0, sizeof(struct clientOptions));
    opts->server_ip = "127.0.0.1"; //default localhost
    opts->port  = DEFAULT_PORT;
}

static void parse_client_arguments(int argc, char *argv[], struct clientOptions *opts)
{
    int c;
    bool is_serverip_set = false;
    while ((c = getopt(argc, argv, "s:p:*:")) != -1) {
        switch(c) {
            case 's':
            {
                opts->server_ip = optarg;
                is_serverip_set = true;
                break;
            }
            case 'p':
            {
                opts->port = parse_port(optarg, 10);
                break;
            }
            case ':':
            {
                error_message(__FILE__, __func__ , __LINE__, "\"Option requires an operand\"", 5);
                break;
            }
            case '?':
            {
                error_message(__FILE__, __func__ , __LINE__, "Unknown", 6);
                break;
            }
        }
    }

    if (!is_serverip_set)
        error_message(__FILE__, __func__ , __LINE__, "\"Server IP must be included\"", 5);
}
