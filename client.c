// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#include <sys/sendfile.h>
#define PORT 8000
#define BUFLEN 1024

int main()
{
    struct sockaddr_in address;
    int socket_fd = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    char buffer[BUFLEN] = {0};
    size_t bufsize = 1024;

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

    printf("Here\n");

    while (1)
    {
        size_t linesiz=0;
        char* linebuf=0;
        ssize_t linelen=0;

        printf("> ");
        getline(&linebuf, &linesiz, stdin);

        if (send(socket_fd, linebuf, strlen(linebuf), 0) < 0)
        {
            perror("Error sending");
            return -1;
        }
    }
    
    return 0;
}