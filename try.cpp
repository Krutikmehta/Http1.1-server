
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

// using namespace std;

#define PORT 8989
#define BUFSIZE 4096
#define THREAD_POOL_SIZE 100
#define PAGE_SIZE 4096
#define STK_SIZE (10 * PAGE_SIZE)

pthread_t thread_pool[THREAD_POOL_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;
pthread_attr_t attr;;

std::deque<int*> deq;



class HTTPRequest
{
    public:
    std::string method;
    std::string uri;
    std::string http_version;
    std::unordered_map<std::string,std::string> headers;
    std::unordered_map<int,std::string> status_codes;
    std::string response;

    HTTPRequest(char* data){
        struct tm * gmtime (const time_t * timer);

        std::time_t now= std::time(0);
        std::tm* now_tm= std::gmtime(&now);
        char buf[200];
        std::strftime(buf, 42, "%a, %d %b %Y %T", now_tm);

        
        headers = {
            {"Server", "linuxServer"},
            {"Content-Type", "text/html"}
        };
        headers.insert({"Date",std::string(buf) +" GMT"});
        status_codes = {
            {200, "OK"},
            {304, "Not Modified"},
            {404, "Not Found"},
            {501, "Not Implemented"}   //   /////////////////////////////////////////////////////////
        };
        parse(data);
        handle_request();
    }

    void parse(char* data){
        std::string request = data;
        size_t start;
        size_t end = 0;

        std::string delim = "\r\n";
        std::vector<std::string> lines;
        std::string conn_type;
        while((start = request.find_first_not_of(delim, end)) != std::string::npos)
        {
            end = request.find(delim, start);
            std::string header = request.substr(start, end - start);
            
            if(header.find("Connection")!=std::string::npos)
                conn_type = header.substr(header.find(":") + 2);  
            
            else conn_type = "close";

            if(header.find("If-Modified-Since")!=std::string::npos){
                conn_type = header.substr(header.find(":") + 2);  
                this->headers.insert({"If-Modified-Since",conn_type});
            }
            lines.push_back(header);
        
        }

        std::string request_line = lines[0];
        end = 0;
        
        delim = " ";
        std::vector<std::string> words;

        while((start = request_line.find_first_not_of(delim, end)) != std::string::npos)
        {
            end = request_line.find(delim, start);
            words.push_back(request_line.substr(start, end - start));
        }

        if(words.size()>0) method = words[0];
        if(words.size()>1) uri = words[1];
        if(words.size()>2) http_version = words[2];
    }

    

    std::string response_line(int status_code){
        std::string reason = status_codes[status_code];
        std::string response_line = "HTTP/1.1 " + std::to_string(status_code) + " " + status_codes[status_code] + "\r\n";
        return response_line;
    }


    std::string response_headers(std::unordered_map<std::string,std::string> extra_headers){
        std::string headers_response = "";
        for(auto it=headers.begin();it!=headers.end();it++){
            headers_response += it->first + ": " + it->second + "\r\n"; 
        }
        if(!extra_headers.empty()){
            for(auto it=extra_headers.begin();it!=extra_headers.end();it++)
            headers_response += it->first + ": " + it->second + "\r\n";
        }
        
        return headers_response;
    }

    void handle_request(){
        std::string response;  

        if(this->headers.find("If-Modified-Since")!=this->headers.end()){
            tm timeptr, client,server;
            std::string if_modified_client = this->headers["If-Modified-Since"];
            strptime(if_modified_client.c_str(),"%a, %d %b %Y %T",&client);
            time_t client_t = mktime(&client);


            struct stat result;
            stat(this->uri.substr(1).c_str(), &result);
        
            char buf[100];
            time_t mod_time = result.st_mtime;
            
            gmtime_r(&mod_time, &timeptr);
            strftime(buf,sizeof(buf), "%a, %d %b %Y %T", &timeptr);
            strptime(buf,"%a, %d %b %Y %T",&server);
            time_t server_t = mktime(&server);

            if(server_t == client_t) {
                std::string response_lin = response_line(304);
                std::string header_lines = "Date: " + this->headers["Date"] + "\r\n"; 
                this->response = response_lin + header_lines;
                return ;
            }
        }

        if(this->method == "GET"){
            this->response = handle_GET();
        }
        else if(this->method == "HEAD"){
            this->response = handle_HEAD();
        }
        return ;
    }

    std::string handle_GET(){
        char buffer[BUFSIZE];
        const char * uri = this->uri.substr(1).c_str();
        FILE *fp = fopen(uri,"r");


        if(fp == NULL){
            std::string response_lin = response_line(404);
            std::string body = "404 Not Found\r\n";
            std::unordered_map<std::string,std::string> extra_headers;
            extra_headers["Content-Length"] = std::to_string(body.length()); 
            std::string header_lines = response_headers(extra_headers);
            std::string blank_line = "\r\n";
            fclose(fp);
            fp = NULL;
            
            return response_lin + header_lines + blank_line + body ;
        }

        std::string extension = this->uri.substr(this->uri.find('.')+1);
        std::string response_lin;
        std::string content_type;
        std::string content_lenght;
        std::unordered_map<std::string,std::string> extra_headers;
        std::string header_lines;
        std::string blank_line = "\r\n";
        std::string body;



        if(extension == "html"){
            size_t bytes_read;
            while((bytes_read = fread(buffer,1,BUFSIZE,fp))>0){
                body += buffer;
            }
            response_lin = response_line(200);
            content_type = "text/html";
            content_lenght = std::to_string(body.length());
            extra_headers["Content-Type"] = content_type;
            extra_headers["Content-Length"] = content_lenght;
            header_lines = response_headers(extra_headers);
            fclose(fp);
            fp = NULL;
        }
        else if(extension == "jpg" or extension == "png" or extension == "jpeg"){

            FILE* file_stream = fopen(uri, "rb");
            std::vector<char> buffer1;
            size_t file_size;

            if(file_stream != nullptr)
            {
                fseek(file_stream, 0, SEEK_END);
                long file_length = ftell(file_stream);
                rewind(file_stream);
                buffer1.resize(file_length);
                file_size = fread(&buffer1[0], 1, file_length, file_stream);
                for(int i = 0; i < file_size; i++)
                {
                    body += buffer1[i];
                }
            }

            response_lin = response_line(200);
            content_type = "images/jpeg";
            content_lenght = std::to_string(body.length());
            extra_headers["Content-Type"] = content_type;
            extra_headers["Content-Length"] = content_lenght;
            header_lines = response_headers(extra_headers);
            std::cout << content_lenght << "\n";
            std::cout << body.length() << "\n"; 
            fclose(file_stream);
            file_stream = NULL;

        }
        
        return response_lin + header_lines + blank_line + body;
    }
    std::string handle_HEAD(){
        char buffer[BUFSIZE]; 
        
        std::ifstream in(this->uri.substr(1), std::ifstream::ate | std::ifstream::binary);
        

        std::string response_lin = response_line(200);
        std::string content_type = "text/html";
        std::string content_lenght = std::to_string(in.tellg());
        std::unordered_map<std::string,std::string> extra_headers;
        extra_headers["Content-Type"] = content_type;
        extra_headers["Content-Length"] = content_lenght;
        std::string header_lines = response_headers(extra_headers);

        // std::cout << response_lin + header_lines;
        return response_lin + header_lines;
    }

};



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

    fd_set set;
    struct timeval timeout;
    int rv;
    int filedesc = client_socket;
    
    while(1){
        FD_ZERO(&set); /* clear the set */
        FD_SET(filedesc, &set); /* add our file descriptor to the set */

        char buffer[BUFSIZE];
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        
        size_t bytes_read;
        int msgsize = 0;
        
        rv = select(filedesc + 1, &set, NULL, NULL, &timeout);
        if(rv == -1)
            perror("select"); /* an error accured */
        else if(rv == 0){
            close(client_socket);
            break;
        }
        else{
            while((bytes_read = read(client_socket,buffer+msgsize,sizeof(buffer)-msgsize-1)) > 0){
                msgsize += bytes_read;
                break;
                // if(msgsize > BUFSIZE-1 || buffer[msgsize-1] == "\n") break;
            }

            buffer[msgsize-1] = 0;
        
            HTTPRequest* req = new HTTPRequest(buffer);
            std::string resp = req->response;
            std::cout << buffer << "\n";
            std::cout << resp << "\n";
            write(client_socket,resp.c_str(),resp.length());

            if(req->headers["Connection"] == "close") {
                close(client_socket);
                break;
            }
            else continue;
        }
    }
    return NULL;
}
int i=0;
void* thread_fun(void* arg){ 
    while(1){       
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
    void *stack;
    posix_memalign(&stack,PAGE_SIZE,STK_SIZE);
    int server_fd, new_socket; long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    size_t stacksize;
    pthread_attr_init(&attr);
    
    pthread_attr_setstack(&attr,&stack,STK_SIZE);
    pthread_attr_getstacksize(&attr, &stacksize);
    printf("%zd\n", stacksize);
    for(int i=0;i<THREAD_POOL_SIZE;i++){
        pthread_create(&thread_pool[i],NULL,thread_fun,NULL);
    }
    
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In socket");
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

        int *p_client = (int *)malloc(sizeof(int));
        *p_client = new_socket;

        pthread_mutex_lock(&mutex);
        enqueue(p_client);
        pthread_cond_signal(&cond_var);
        pthread_mutex_unlock(&mutex); 
        
        
    }
    return 0;
}