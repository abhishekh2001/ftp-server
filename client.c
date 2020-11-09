// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#include <sys/sendfile.h>
#define PORT 8000
#define BUFLEN 1024

struct sockaddr_in address;
int socket_fd = 0, valread;
struct sockaddr_in serv_addr;
char *hello = "Hello from client";
char buffer[BUFLEN] = {0};
size_t bufsize = 1024;

int handle_input(char *buffer)
{
    char ack[2049] = { 0 };
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

    if (argc == 0)
    {
        return 0;
    }
    else if (!strcmp(argv[0], "get"))
    {
        if (argc == 1)
        {
            fprintf(stderr, "Error get: usage{ get <filename...> }\n");
            return -1;
        }
        if (send(socket_fd, argv[0], strlen(argv[0]), 0) == -1)
        {
            perror("Error sending");
            return -1;
        }

        if (recv(socket_fd, ack, 1020, 0) == -1)
        {
            perror("Error receiving ack from server");
            return -1;
        }

        for (int i = 1; i < argc; i++)
        {
            printf("Requesting server for file %s\n", argv[i]);
            if (send(socket_fd, argv[i], strlen(argv[i]), 0) == -1)
            {
                perror("Error while sending filename");
                return -1;
            }


        }
    }

    for (int i = 0; i < argc; i++)
    {
        free(argv[i]);
    }
    free(argv);
    free(abc);
}

int main()
{
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        perror("Error creating socket");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1",
                  &serv_addr.sin_addr) <= 0)
    {
        perror("address");
        return -1;
    }

    printf("socket fd = %d\n", socket_fd);
    if (connect(socket_fd, (struct sockaddr *)&serv_addr,
                sizeof(serv_addr)) < 0)
    {
        perror("Error connecting");
        return -1;
    }

    while (1)
    {
        size_t linesiz = 0;
        char *linebuf = 0;
        ssize_t linelen = 0;

        printf("> ");
        getline(&linebuf, &linesiz, stdin);

        handle_input(linebuf);
    }

    return 0;
}