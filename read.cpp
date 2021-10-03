
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <fstream>
#include <bits/stdc++.h>
#include <iostream>
#include<sys/types.h>
#include<sys/stat.h>
#include <fcntl.h> 
#include <pthread.h>
  

#define PORT 8989
#define BUFSIZE 4096
#define THREAD_POOL_SIZE 20

pthread_t thread_pool[THREAD_POOL_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;

std::deque<int*> deq;

struct hellas{
    hellas* next;
    int *client_socket;
};

typedef struct hellas node_t;

node_t* head = NULL;
node_t* tail = NULL;


void enqueue(int *client_socket){
    node_t *newnode = new node_t();
    newnode->client_socket = client_socket;
    newnode->next = NULL;
    if(tail == NULL){
        head = newnode;
    }else{
        tail->next = newnode;
    }
    tail = newnode;
}

int* dequeue(){
    if(head == NULL) return NULL;
    else{
        int* result = head->client_socket;
        node_t *temp = head;
        head = head->next;
        if(head == NULL) tail = NULL;
        free(temp);
        return result;
    }
}

void* handle_connection(void* p_client_socket){
    int client_socket = *((int*)p_client_socket);
    // std::cout << client_socket << "\n";
    // std::cout << "here" << "\n";
    char buffer[BUFSIZE];

    size_t bytes_read;
    int msgsize = 0;
    // char actual_path
    while((bytes_read = read(client_socket,buffer+msgsize,sizeof(buffer)-msgsize-1)) > 0){
        msgsize += bytes_read;
        break;
        // if(msgsize > BUFSIZE-1 || buffer[msgsize-1] == "\n") break;
    }
    buffer[msgsize-1] = 0;
    // std::cout<< "open" << "\n";
    FILE *fp = fopen("index.html","r");
    if(fp == NULL){
        // std::cout << "error";
        close(client_socket);
        return NULL;
    }

    while((bytes_read = fread(buffer,1,BUFSIZE,fp))>0){
        write(client_socket,buffer,bytes_read);
    }

    close(client_socket);
    fclose(fp);
    std::cout << "close conn";
    return NULL;
}
int i=0;
void* thread_fun(void* arg){ 
    while(1){
            std::cout << ++i;
            int *p_client;
            pthread_mutex_lock(&mutex);
            if((p_client = dequeue()) == NULL){
                pthread_cond_wait(&cond_var,&mutex);
                p_client = dequeue();
            }
            pthread_mutex_unlock(&mutex);
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
    for(int i=0;i<THREAD_POOL_SIZE;i++){
        pthread_create(&thread_pool[i],NULL,thread_fun,NULL);
    }
    
    std::string hello = "HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length:1000\n\n";
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In socket");
        exit(EXIT_FAILURE);
    }
    // std::cout << hello << " ";

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    
    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    int optval = 1;
    setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,&optval, sizeof(int));
    setsockopt(server_fd,SOL_SOCKET,SO_REUSEPORT,&optval, sizeof(int));

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
    

    while(1)
    {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("In accept");
            exit(EXIT_FAILURE);
        }

        // std:: cout<<new_socket << "\n";
        int *p_client = (int *)malloc(sizeof(int));
        *p_client = new_socket;

        pthread_mutex_lock(&mutex);
        enqueue(p_client);
        pthread_cond_signal(&cond_var);
        pthread_mutex_unlock(&mutex); 
        
        
    }
    return 0;
}

