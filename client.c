// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#define PORT 8000

int main()
{
    struct sockaddr_in address;
    int socket_fd = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    char buffer[1024] = {0};

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

    send(socket_fd, hello, strlen(hello), 0); // send the message.
    printf("Hello message sent\n");
    while (1)
    {
        valread = read(socket_fd, buffer, 1024); // receive message back from server, into the buffer
        printf("%s\n", buffer);
    }
    
    return 0;
}