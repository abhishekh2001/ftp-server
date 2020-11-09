// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#define PORT 8000
#define BUFFSIZE 1024

struct sockaddr_in address;
int socket_fd = 0, valread;
struct sockaddr_in serv_addr;
char buffer[BUFFSIZE] = {0};
char ack[2049] = {0};
size_t bufsize = 1024;

int check_con()
{
    if (recv(socket_fd, ack, 1020, 0) == -1)
    {
        perror("Error receiving ack from server");
        return -1;
    }
    return 0;
}

int handle_input(char *inp)
{
    char *abc = (char *)malloc(sizeof(char) * strlen(inp));
    strcpy(abc, inp);
    char *tok;
    char *ptr = abc;

    char **argv = (char **)malloc(sizeof(char *) * strlen(abc));
    int argc = 0;

    while ((tok = strtok(ptr, " ")) != NULL)
    {
        argv[argc] = (char *)malloc(sizeof(char) * strlen(tok));
        if (tok[strlen(tok)-1] == '\n')
            tok[strlen(tok)-1] = '\0';
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
        if (check_con() == -1)
        {
            return -1;
        }

        // Send number of files
        sprintf(buffer, "%d", argc - 1);
        if (send(socket_fd, buffer, strlen(buffer), 0) == -1)
        {
            perror("Error sending");
            return -1;
        }
        if (check_con() == -1)
        {
            return -1;
        }

        for (int i = 1; i < argc; i++)
        {
            int fd = open(argv[i], O_CREAT | O_RDWR | O_TRUNC | O_APPEND, 0666);
            if (fd == -1)
            {
                perror("Error opening file");
                continue;
            }
            printf("Requesting server for file %s\n", argv[i]);
            if (send(socket_fd, argv[i], strlen(argv[i]), 0) == -1)
            {
                perror("Error while sending filename");
                return -1;
            }
            printf("Sent filename\n");
            if (check_con() == -1)
            {
                return -1;
            }
            printf("Filename ack received\n");

            int f_size;
            if (recv(socket_fd, &f_size, sizeof(int), 0) == -1)
            {
                perror("Error while receiving filesize");
                return -1;
            }
            printf("received filesize %d\n", f_size);

            memset(buffer, '\0', sizeof(char) * BUFFSIZE);
            strcpy(buffer, "DONE");
            if (send(socket_fd, buffer, strlen(buffer), 0) == -1)
            {
                perror("Error while sending filesize ack");
                return -1;
            }
            printf("Filesize ack sent\n");

            if (f_size == 0)
            {
                printf("File <%s> does not exist on server\n", argv[i]);
                continue;
            }

            printf("File <%s> on server is of size %d bytes\n", argv[i], f_size);

            int tot = 0;
            while (tot < f_size)
            {
                memset(buffer, '\0', sizeof(char) * BUFFSIZE);
                int b = recv(socket_fd, buffer, 10024, 0);
                tot += b;
                printf(": %s - \n", buffer);
                write(fd, buffer, strlen(buffer));

                memset(buffer, '\0', sizeof(char) * BUFFSIZE);
                strcpy(buffer, "DONE");
                if (send(socket_fd, buffer, strlen(buffer), 0) == -1)
                {
                    perror("Error while sending chunk ack");
                    return -1;
                }
            }
            close(fd);
            printf("\n-DONE-\n");
        }
    }
    free(abc);
}

int main()
{
    signal(SIGPIPE, SIG_IGN);

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
        char *linebuf = 0;
        ssize_t linelen = 0;

        printf("> ");
        getline(&linebuf, &linelen, stdin);

        // trim
        // printf("this [%c]\n", linebuf[strlen(linebuf)-1]);
        // if (linebuf[strlen(linebuf)-1] == '\n')
        // {
        //     printf("removing newlnie\n");
        //     linebuf[strlen(linebuf)-1] = '\0';
        // }

        handle_input(linebuf);

        free(linebuf);
    }

    return 0;
}