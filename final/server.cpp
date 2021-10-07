
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <fstream>
#include <bits/stdc++.h>
#include <iostream>
#include <sys/types.h>
#include <fcntl.h> 
#include <pthread.h>
#include <time.h>
#include <filesystem>
#include <sys/stat.h>
#include <arpa/inet.h>


#include "queue.h"
#include "httpserver.h"
#include "constants.h"
// using namespace std;



pthread_t thread_pool[THREAD_POOL_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;
pthread_attr_t attr;;

std::deque<int*> deq;

/*connections are handled by this function. It creates an obj of class HTTPRequest*/
void* handle_connection(void* p_client_socket){
    int client_socket = *((int*)p_client_socket);

    fd_set set;
    struct timeval timeout,tv1;
    timeout.tv_sec = CONN_TIMEOUT;
    timeout.tv_usec = 0;
    int rv;
    int filedesc = client_socket;
    FD_ZERO(&set); /* clear the set */
    FD_SET(filedesc, &set); /* add our file descriptor to the set */

    
    while(1){
        memcpy(&tv1, &timeout, sizeof(timeout));
        char buffer[BUFSIZE];
        
        size_t bytes_read;
        int msgsize = 0;
        
        /*check if any file descriptor/socket is ready active*/
        rv = select(filedesc + 1, &set, NULL, NULL, &tv1);
        if(rv == -1){        
            perror("Following error occured while waiting for client request\n");
        }
        else if(rv == 0){
            /*timeout occured*/
            struct sockaddr_in addr;
            socklen_t addr_size = sizeof(struct sockaddr_in);
            int res = getpeername(client_socket, (struct sockaddr *)&addr, &addr_size);
            char *clientip = new char[15];
            strcpy(clientip, inet_ntoa(addr.sin_addr));
            std::cout << "connection timeout with " <<std::string(clientip,15) << "\n";

            if(close(client_socket) == -1){
                perror("Following error occured while closing client socket\n");
            }
            
            return NULL;
        }
        else{
            /*a client connection received*/
            while((bytes_read = read(client_socket,buffer+msgsize,sizeof(buffer)-msgsize-1)) > 0){
                msgsize += bytes_read;
                break;
                if(msgsize > BUFSIZE-1 || std::to_string(buffer[msgsize-1]) == "\n") break;
            }

            buffer[msgsize-1] = 0;
            
            HTTPRequest* req = new HTTPRequest(buffer);
            std::string resp = req->response;
            write(client_socket,resp.c_str(),resp.length());
            
            /*if the Conncection type header is 'close' the the connecti0n is closed, else we wait for 
            the client to send another request until a timeout occurs*/
            if(req->headers["Connection"] == "close") {

                struct sockaddr_in addr;
                socklen_t addr_size = sizeof(struct sockaddr_in);
                int res = getpeername(client_socket, (struct sockaddr *)&addr, &addr_size);
                char *clientip = new char[15];
                strcpy(clientip, inet_ntoa(addr.sin_addr));
                std::cout << "closing connection with " <<std::string(clientip,15)<<"\n";
                if(close(client_socket) == -1){
                    perror("Following error occured while closing client socket\n");
                }
                return NULL;
            }
            else    
                continue;
        }
        }
    return NULL;
}

void* thread_fun(void* arg){ 
    while(1){       
            int *p_client;

            /*lock the queue before accessing the socket_fd*/
            pthread_mutex_lock(&mutex);

            if((p_client = dequeue()) == NULL){
                /*if queue is empty, wait until new socket_fd is added to queue. we donot keep looping to check for new socket_fd*/
                pthread_cond_wait(&cond_var,&mutex);
                p_client = dequeue();
            }
            /*unlock after removing socket_fd from the queue*/
            pthread_mutex_unlock(&mutex);

            /*server the client*/
            if(p_client != NULL){
                handle_connection(p_client);
            }
 
    }
}

int main(int argc, char const *argv[])
{

    int server_fd, new_socket; long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    size_t stacksize;
    pthread_attr_init(&attr);


    /*creating threads*/
    for(int i=0;i<THREAD_POOL_SIZE;i++){
        pthread_create(&thread_pool[i],NULL,thread_fun,NULL);
    }
    
    
    /*Creating socket file descriptor */
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Following error occured while creating server socket\n");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    
    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    int optval = 1;
    setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,&optval, sizeof(int));
    setsockopt(server_fd,SOL_SOCKET,SO_REUSEPORT,&optval, sizeof(int));

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("Following error occured while binding server\n");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0)
    {
        perror("Following error occured while the server was listening for connection\n");
        exit(EXIT_FAILURE);
    }
    

    while(1)
    {
        std::cout <<("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("Following error occured while accepting client\n");
            exit(EXIT_FAILURE);
        }
        else {
            printf("IP address of client: %s\n", inet_ntoa(address.sin_addr));
            printf("port no. of client: %d\n", (int) ntohs(address.sin_port));
        }

        int *p_client = (int *)malloc(sizeof(int));
        *p_client = new_socket;

        /*lock the queue to make it thread-safe and add the client, then unlock it*/
        pthread_mutex_lock(&mutex);
        enqueue(p_client);
        pthread_cond_signal(&cond_var);
        pthread_mutex_unlock(&mutex); 
        
    }
    return 0;
}