#include <sys/socket.h> // socket
#include <netinet/in.h> // sockaddr
#include <assert.h>
#include <sys/fcntl.h>  // fcntl
#include <sys/epoll.h> // epoll
#include <unistd.h> // close
#include <errno.h>  // errno
#include <stdio.h>

#include <cstring>  // bzero
#include <vector>  
#include <iostream>


// A simple Echo Server with epoll


int setNonBlocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void addfd(int epollfd, int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setNonBlocking(fd);
}


int main()
{
    // server ip and port
    const char* server_ip = "127.0.0.1";
    int server_port = 9019;
 
    // create and init server socket
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof server_addr);

    // server sock file descriptor
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(server_sock >= 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // bind socket to port
    // transform server_addr to struct sockaddr
    int ret = bind(server_sock, (struct sockaddr*)&server_addr, sizeof server_addr); 
    assert(ret != -1);
    
    // listen
    ret = listen(server_sock, 5);
    assert(ret != -1);
    printf("Listening in port[%d]\n", server_port);

    // create epollfd
    // different between epoll_create and epoll_create1
    int epollfd = ::epoll_create1(EPOLL_CLOEXEC);
    assert(epollfd != -1);

    const int MaxEventNumber = 1024;
    std::vector<epoll_event> events(MaxEventNumber);
    
    addfd(epollfd, server_sock);

    // client socket
    // int client_sock;
    struct sockaddr_in client_addr;
    int connect_sock;
    socklen_t client_addr_len = sizeof(client_addr);

    // buffer in application level
    const int Buffer_Size = 1024;
    char buffer[Buffer_Size];

    while (true)
    {
        int active_event = ::epoll_wait(epollfd, events.data(), MaxEventNumber, -1);
        if (active_event < 0)
        {
            // error
        }
        
        for (int i = 0; i < active_event; ++i)
        {
            int active_sockfd = events[i].data.fd;
            if (active_sockfd == server_sock)
            {
                // naming is easy to lead bugs
                connect_sock = accept4(server_sock, (struct sockaddr*)&client_addr, 
                    &client_addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
                printf("Connect from [%d] establied\n", client_addr.sin_port);
                assert(connect_sock != -1);
                addfd(epollfd, connect_sock);
            }
            else if (events[i].events & EPOLLIN)
            {
                // while(true) // while can be used for error break;
                
                    bzero(buffer, Buffer_Size);
                    ret = recv(active_sockfd, buffer, Buffer_Size - 1, 0);
                    if (ret < 0)
                    {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                        {
                            // error
                        }
                        ::close(connect_sock);
                    }
                    else if (ret == 0)
                    {
                        printf("Connect from [%d] discount\n", client_addr.sin_port);
                        ::close(active_sockfd);
                    }
                    else
                    {
                        printf("Message from [%d]: %s", client_addr.sin_port, buffer);
                        ::send(active_sockfd, buffer, ret, 0);
                    }
            }
            else
            {
                // something else
            }
        }
    }
    ::close(epollfd);
    ::close(server_sock);

}