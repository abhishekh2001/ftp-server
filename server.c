#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
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

void handler(int num)
{
    printf("Handling sigpipe error\n");
}

int main()
{
    signal(SIGPIPE, handler);

    int server_fd, new_socket, valread;
    int opt;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
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

    printf("Listening...\n");
    valread = read(new_socket, buffer, 1024); // read infromation received into the buffer
    printf("%s\n", buffer);

    

    for (int i = 0; i < 10; i++)
    {
        char hello[1000];
        sprintf(hello, "Hello %d", i);
        if (send(new_socket, hello, strlen(hello), 0) < 0) // use sendto() and recvfrom() for DGRAM
        {
            perror("Error sending");
            return -1;
        }
        printf("Hello message sent\n");
        sleep(2);
    }
}
