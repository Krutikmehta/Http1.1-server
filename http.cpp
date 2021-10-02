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


#define PORT 8102
int main(int argc, char const *argv[])
{
    int server_fd, new_socket; long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
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
    int i=0;

    while(1)
    {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("In accept");
            exit(EXIT_FAILURE);
        }

        int fd = open("index.html",O_RDONLY);
        char buffer[100000] = {0};
        int bytes_read = read(fd,buffer,sizeof(buffer));
        if (bytes_read == 0) // We're done reading from the file
        break;
    
        if (bytes_read < 0) {
            // handle errors
        }
        char *p = buffer;
        while (bytes_read > 0) {
            int bytes_written = write(new_socket, p, bytes_read);
            if (bytes_written <= 0) {
            // handle errors
            }
            bytes_read -= bytes_written;
            p += bytes_written;
        }
        close(fd);


        // std::ifstream myfile;
        // std::string myline;
        // myfile.open("index.html");
        // if ( myfile.is_open() ) {
        //     while ( myfile.good() ) { // equivalent to myfile.good()
        //         std::getline (myfile, myline);
        //         std::cout << ++i;
        //         hello += myline + "\n";
        //         // std::cout << myline << '\n';
        //     }
        // }
        // else {
        // std::cout << "Couldn't open file\n";
        // }
         
        // FILE *fd = fopen("index.html", "rb");
        // size_t rret, wret;
        // int bytes_read;
        // while (!feof(fd)) {
        //     if ((bytes_read = fread(&buffer, 1, BUFFER_SIZE, fd)) > 0)
        //         send(new_socket, buffer, bytes_read, 0);
        //     else
        //         break;
        // }
        // fclose(fd);
        // char buffer[30000] = {0};
        valread = read( new_socket , buffer, 30000);
        printf("%s\n",buffer );
        std::cout << hello ;
        
        // write(new_socket , hello.c_str() , hello.length());
        printf("------------------Hello message sent-------------------\n");
        close(new_socket);
    }
    return 0;
}

