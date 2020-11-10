#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#define PORT 8000
#define BUFFSIZE 1024
#define CHUNK_SIZE 1023 // TODO: UPDATE
#define DONE "done"

/*
https://pubs.opengroup.org/onlinepubs/009695399/functions/xsh_chap02_10.html
> socket created with type (SOCK_STREAM, SOCK_DGRAM...)
which allows selection of appropriate comm protocol
> SOCK_STREAM: TCP
A SIGPIPE signal is raised if a thread sends on 
a broken stream (one that is no longer connected).
*/

void handler(int signo)
{
    printf("Received SIGPIPE\n");
    exit(EXIT_SUCCESS);
}

int server_fd, new_socket, valread;
int opt;
struct sockaddr_in address;
int addrlen = sizeof(address);
char buffer[BUFFSIZE] = {0};

int handle_client_input(char *cmd)
{
    printf("inside handle, trying %s\n", cmd);
    if (!strcmp(cmd, "get"))
    {
        int fd, filenums, b;
        long long int f_size;
        char filename[10000];
        struct stat st;

        memset(buffer, '\0', BUFFSIZE);
        // Get number of file requests
        if (recv(new_socket, buffer, BUFFSIZE, 0) < 0)
        {
            perror("Error recieving from client");
            return -1;
        }
        printf("filenum %s\n", buffer);
        filenums = atoi(buffer);

        memset(buffer, '\0', BUFFSIZE);
        strcpy(buffer, "DONE");
        if (send(new_socket, buffer, BUFFSIZE, 0) == -1)
        {
            perror("Error sending to socket");
            return -1;
        }

        printf("filenum = %d\n", filenums);

        // Send each file
        for (int i = 0; i < filenums; i++)
        {
            memset(buffer, '\0', sizeof(char) * BUFFSIZE);
            if (recv(new_socket, buffer, BUFFSIZE, 0) == -1)
            {
                perror("Error recieving from client");
                return -1;
            }
            strcpy(filename, buffer);

            memset(buffer, '\0', sizeof(char) * BUFFSIZE);
            strcpy(buffer, "DONE");
            if (send(new_socket, buffer, BUFFSIZE, 0) == -1)
            {
                perror("Error sending to socket");
                return -1;
            }

            printf("SENDING FILE %s\n", filename);
            fd = open(filename, O_RDONLY);
            if (fd < 1) //  Error opening file
            {
                printf("Sending f_size\n");
                f_size = -1;
                if (send(new_socket, &f_size, sizeof(f_size), 0) == -1)
                {
                    perror("Error sending filesize");
                    return -1;
                }

                memset(buffer, '\0', BUFFSIZE);
                printf("Sent f_size\n");
                if (recv(new_socket, buffer, BUFFSIZE, 0) == -1)
                {
                    perror("Error while recieving filesize ack");
                    return -1;
                }
                fprintf(stderr, "Cannot open file %s\n", filename);
                continue;
            }
            stat(filename, &st);
            f_size = st.st_size;
            printf("File <%s> is of size %lld\n", filename, f_size);

            printf("Sending f_size\n");
            if (send(new_socket, &f_size, sizeof(f_size), 0) == -1)
            {
                perror("Error sending filesize");
                return -1;
            }
            printf("Sent f_size\n");
            if (recv(new_socket, buffer, BUFFSIZE, 0) == -1)
            {
                perror("Error while recieving filesize ack");
                return -1;
            }

            printf("Received f_size ack\n");

            if (f_size == -1)
            {
                continue;
            }

            char linebuf[CHUNK_SIZE + 100] = {0};
            memset(linebuf, '\0', sizeof(char) * (CHUNK_SIZE + 10));
            while ((b = read(fd, linebuf, CHUNK_SIZE)) > 0)
            {
                // printf("Sending size %d\n", b);
                if (send(new_socket, linebuf, b, 0) == -1)
                {
                    perror("Error sending chunk");
                    return -1;
                }

                memset(linebuf, '\0', sizeof(char) * CHUNK_SIZE);

                memset(buffer, '\0', sizeof(char) * BUFFSIZE);
                if (recv(new_socket, buffer, BUFFSIZE, 0) == -1)
                {
                    perror("Error while receiving chunk ack");
                    return -1;
                }
                // printf("Received chunk ack\n");
            }
            printf("Sent %s\n", filename);

            close(fd);
        }

        return 0;
    }
    else if (!strcmp(cmd, "exit"))
    {
        printf("Client has closed connection. Server quitting...\n");
        return -2;
    }
    return 0;
}

int main()
{
    signal(SIGPIPE, SIG_IGN);

    char *hello = "Hello from server";

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("Creating socket");
        return -1;
    }

    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt)))
    {
        perror("Setting socket options");
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0)
    {
        perror("Binding");
        return -1;
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("Error listening");
        return -1;
    }

    if ((new_socket = accept(server_fd,
                             (struct sockaddr *)&address,
                             (socklen_t *)&addrlen)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        char inp[BUFFSIZE + 100];
        if (recv(new_socket, buffer, BUFFSIZE, 0) < 0)
        {
            perror("Error recieving from client");
            return -1;
        }
        printf("Recieved %s from client\n", buffer);
        strcpy(inp, buffer);

        memset(buffer, 0, BUFFSIZE);
        strcpy(buffer, "DONE");
        if (send(new_socket, buffer, BUFFSIZE, 0) == -1)
        {
            perror("Error sending to socket");
            return -1;
        }
        if (handle_client_input(inp) < 0)
        {
            return 0;
        }
    }
}
