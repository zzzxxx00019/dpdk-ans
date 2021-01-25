#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sched.h>
#include <sys/times.h>
#include <assert.h>

#include <anssock_intf.h>
#include <ans_errno.h>

#define BACKLOG 512
#define MAX_EVENTS 128
#define MAX_MESSAGE_LEN 2048

#define default_port 8081

void error(char* msg);


int main(int argc, char *argv[])
{
	struct sockaddr_in server_addr, client_addr;
	socklen_t client_len = sizeof(client_addr);
    
    int core = 0;
    cpu_set_t cpus;
    
	CPU_ZERO(&cpus);
	CPU_SET((unsigned)core, &cpus);
	sched_setaffinity(0, sizeof(cpus), &cpus);
   
    int ret = anssock_init(NULL);
    assert(ret == 0 && "anssock init fail");
    
	char buffer[MAX_MESSAGE_LEN];
	memset(buffer, 0, sizeof(buffer));


	// setup socket
	int sock_listen_fd ;
    int opt_val = 1 ;
    
	if ((sock_listen_fd = anssock_socket(AF_INET, SOCK_STREAM, 0))<0) {
        printf("socket error \n");
        return -1;
    }

	memset((char *)&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(default_port);
	server_addr.sin_addr.s_addr = INADDR_ANY;

    if(anssock_setsockopt(sock_listen_fd, SOL_SOCKET, SO_REUSEPORT, &opt_val, sizeof(int)) < 0)
    {
        printf("set socket option failed \n");
    }
	
    
	// bind socket and listen for connections
	if (anssock_bind(sock_listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		error("Error binding socket..\n");
    }
	if (anssock_listen(sock_listen_fd, BACKLOG) < 0) {
        error("Error listening..\n");
    }
	printf("epoll echo server listening for connections on port: %d\n", default_port);

	struct epoll_event ev, events[MAX_EVENTS];
	int new_events, sock_conn_fd, epollfd;
	
	epollfd = anssock_epoll_create(MAX_EVENTS);
	if (epollfd < 0)
	{
		error("Error creating epoll..\n");
	}
	ev.events = EPOLLIN;
	ev.data.fd = sock_listen_fd;
    
	if (anssock_epoll_ctl(epollfd, EPOLL_CTL_ADD, sock_listen_fd, &ev) == -1)
	{
		printf("Error adding new listeding socket to epoll..\n");
        anssock_close(sock_listen_fd);
        anssock_close(epollfd);
        return -1;
	}
    
    
	while(1)
	{
		new_events = anssock_epoll_wait(epollfd, events, MAX_EVENTS, -1);
		
		if (new_events == -1)
		{
			printf("Error in epoll_wait..\n");
            anssock_close(sock_listen_fd);
            anssock_close(epollfd);
            return -1;
		}

		for (int i = 0; i < new_events; ++i)
		{
			if (events[i].data.fd == sock_listen_fd)
			{
				//sock_conn_fd = anssock_accept(sock_listen_fd, (struct sockaddr *)&client_addr, &client_len, SOCK_NONBLOCK);
				sock_conn_fd = anssock_accept(sock_listen_fd, NULL, NULL);
                if (sock_conn_fd == -1)
				{
					error("Error accepting new connection..\n");
				}

				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = sock_conn_fd;
				if (anssock_epoll_ctl(epollfd, EPOLL_CTL_ADD, sock_conn_fd, &ev) == -1)
				{
					error("Error adding new event to epoll..\n");
				}
			}
			else
			{
				int newsockfd = events[i].data.fd;
				int bytes_received = anssock_recvfrom(newsockfd, buffer, MAX_MESSAGE_LEN, 0, NULL, NULL);
				if (bytes_received <= 0)
				{
					anssock_epoll_ctl(epollfd, EPOLL_CTL_DEL, newsockfd, NULL);
					shutdown(newsockfd, SHUT_RDWR);
				}
				else
				{
					anssock_send(newsockfd, buffer, bytes_received, 0);
				}
			}
		}
	}
}



void error(char* msg)
{
	perror(msg);
	printf("erreur...\n");
	exit(1);
}
