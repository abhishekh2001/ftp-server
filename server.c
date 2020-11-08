#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
// For sendfile()
#include <sys/sendfile.h>
#define PORT 8000

#define CHUNK_SIZE 4096

/*
https://pubs.opengroup.org/onlinepubs/009695399/functions/xsh_chap02_10.html
> socket created with type (SOCK_STREAM, SOCK_DGRAM...)
which allows selection of appropriate comm protocol
> SOCK_STREAM: TCP
A SIGPIPE signal is raised if a thread sends on 
a broken stream (one that is no longer connected).
*/

int server_fd, new_socket, valread;
int opt;
struct sockaddr_in address;
int addrlen = sizeof(address);

int handle_client_input(char *buffer, int buff_len)
{
    printf("handling");

    char *abc = (char *)malloc(sizeof(char) * strlen(buffer));
    strcpy(abc, buffer);
    char *tok;
    char *ptr = abc;

    char **argv = (char **)malloc(sizeof(char *) * strlen(abc));
    int argc = 0;

    while ((tok = strtok(ptr, " ")) != NULL)
    {
        argv[argc] = (char *)malloc(sizeof(char) * strlen(tok));
        strcpy(argv[argc], tok);
        argc++;
        ptr = NULL;
    }

    for (int i = 0; i < argc; i++)
        printf(": %s\n", argv[i]);

    if (!strcmp(argv[0], "get"))
    {
        printf("SERVER MUST RETURN SEND FILES\n");
        return 0;
    }
    return -1;

    free(abc);
    for (int i = 0; i < argc; i++)
        free(argv[i]);
    free(argv);
}

int main()
{
    char buffer[1024] = {0};
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
        if (recv(new_socket, buffer, 1020, 0) < 0)
        {
            perror("Error recieving from client");
            return -1;
        }

        printf("Recieved %s from client\n", buffer);
        handle_client_input(buffer, strlen(buffer));
    }
}
