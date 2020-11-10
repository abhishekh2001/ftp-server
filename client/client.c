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

#define KNRM "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"

struct sockaddr_in address;
int socket_fd = 0, valread;
struct sockaddr_in serv_addr;
char buffer[BUFFSIZE] = {0};
char ack[BUFFSIZE] = {0};
size_t bufsize = 1024;

int check_con()
{
    if (recv(socket_fd, ack, BUFFSIZE, 0) == -1)
    {
        perror("Error receiving ack from server");
        return -1;
    }
    // printf("Ack received %s\n", ack);
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

    while ((tok = strtok(ptr, " \t\n")) != NULL)
    {
        argv[argc] = (char *)malloc(sizeof(char) * strlen(tok));
        if (tok[strlen(tok) - 1] == '\n')
        {
            // printf("deling newline\n");
            tok[strlen(tok) - 1] = '\0';
        }
        // printf("tokenizing: %s\n", tok);
        strcpy(argv[argc], tok);
        argc++;
        ptr = NULL;
    }

    // for (int i = 0; i < argc; i++)
    //     printf(": %s\n", argv[i]);

    if (argc == 0)
    {
        return 0;
    }
    else if (!strcmp(argv[0], "get"))
    {
        if (argc == 1)
        {
            fprintf(stderr, KRED "Error get: usage{ get <filename...> }\n" KNRM);
            return 1;
        }
        memset(buffer, 0, BUFFSIZE);
        strcpy(buffer, argv[0]);
        if (send(socket_fd, buffer, BUFFSIZE, 0) == -1)
        {
            perror("Error sending");
            return -1;
        }
        if (check_con() == -1)
        {
            return -1;
        }

        // Send number of files
        memset(buffer, 0, BUFFSIZE);
        sprintf(buffer, "%d", argc - 1);
        if (send(socket_fd, buffer, BUFFSIZE, 0) == -1)
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
            // printf("Requesting server for file %s\n", argv[i]);
            memset(buffer, 0, BUFFSIZE);
            strcpy(buffer, argv[i]);
            if (send(socket_fd, buffer, BUFFSIZE, 0) == -1)
            {
                perror("Error while sending filename");
                return -1;
            }
            // printf("Sent filename\n");
            if (check_con() == -1)
            {
                return -1;
            }
            // printf("Filename ack received\n");

            long long int f_size;
            if (recv(socket_fd, &f_size, sizeof(long long int), 0) == -1)
            {
                perror("Error while receiving filesize");
                return -1;
            }
            // printf("received filesize %lld\n", f_size);

            memset(buffer, '\0', sizeof(char) * BUFFSIZE);
            strcpy(buffer, "DONE");
            if (send(socket_fd, buffer, BUFFSIZE, 0) == -1)
            {
                perror("Error while sending filesize ack");
                return -1;
            }
            // printf("Filesize ack sent\n");

            if (f_size == -1)
            {
                printf(KRED "File <%s> does not exist on server.\n" KNRM, argv[i]);
                continue;
            }

            printf("File <%s> on server is of size %lld bytes\n", argv[i], f_size);

            int fd = open(argv[i], O_CREAT | O_RDWR | O_TRUNC | O_APPEND, 0666);
            if (fd == -1)
            {
                perror("Error opening file");
                continue;
            }
            if (f_size == 0)
            {
                printf(KGRN "Received: 100.0 percentage\n" KNRM);
                printf(KGRN "Recieved %s\n" KNRM, argv[i]);

                continue;
            }

            long long int tot = 0;
            while (tot < f_size)
            {
                memset(buffer, '\0', sizeof(char) * BUFFSIZE);
                int b = recv(socket_fd, buffer, 1023, 0);
                tot += b;
                // printf(": %s - \n", buffer);
                write(fd, buffer, strlen(buffer));

                printf(KGRN "\rReceived: %f percentage" KNRM, ((float)tot / f_size) * 100);
                fflush(stdout);

                memset(buffer, '\0', sizeof(char) * BUFFSIZE);
                strcpy(buffer, "DONE");
                if (send(socket_fd, buffer, BUFFSIZE, 0) == -1)
                {
                    perror("Error while sending chunk ack");
                    return -1;
                }
            }
            close(fd);
            printf(KGRN "\nRecieved %s\n" KNRM, argv[i]);
        }
    }
    else if (!strcmp(argv[0], "exit"))
    {
        memset(buffer, 0, BUFFSIZE);
        strcpy(buffer, "exit");
        if (send(socket_fd, buffer, BUFFSIZE, 0) == -1)
        {
            perror("while send exit signal");
            return -1;
        }
        printf(KRED "Exitting client...\n" KRED);
        for (int i = 0; i < argc; i++)
        {
            free(argv[i]);
        }
        free(argv);
        free(abc);
        return -2;
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
        char *linebuf = NULL;
        size_t linelen = 0;

        printf("client> ");
        ssize_t chars = getline(&linebuf, &linelen, stdin);
        // puts(linebuf);

        // trim
        // printf("this [%c]\n", linebuf[strlen(linebuf)-1]);
        // if (linebuf[strlen(linebuf)-1] == '\n')
        // {
        //     printf("removing newlnie\n");
        //     linebuf[strlen(linebuf)-1] = '\0';
        // }

        if (linebuf[chars - 1] == '\n')
            linebuf[chars - 1] = '\0';

        if (handle_input(linebuf) < 0)
        {
            free(linebuf);
            return 0;
        }

        free(linebuf);
    }

    return 0;
}