#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#define PORT 8082

int main(int argc, char const *argv[])
{
    int sock = 0; long valread;
    struct sockaddr_in serv_addr;
    // char *hello = "Hello from client";
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    
    memset(&serv_addr, '0', sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    // send(sock , hello , strlen(hello) , 0 );
    printf("Hello message sent\n");
    valread = read( sock , buffer, 1024);
    printf("%s\n",buffer );
    return 0;
}


// void handle_connection(int new_socket){
//     // int client_socket = *((int*)p_client);
//     // free(p_client);

//     int fd = open("index.html",O_RDONLY);
//     char buffer[100000] = {0};
//     int bytes_read = read(fd,buffer,sizeof(buffer));
//     if (bytes_read == 0) // We're done reading from the file
//     // break;

//     if (bytes_read < 0) {
//         // handle errors
//     }
//     char *p = buffer;
//     while (bytes_read > 0) {
//         int bytes_written = write(new_socket, p, bytes_read);
//         if (bytes_written <= 0) {
//         // handle errors
//         }
//         bytes_read -= bytes_written;
//         p += bytes_written;
//     }
//     close(fd);
//     // valread = read( new_socket , buffer, 30000);
//     // printf("%s\n",buffer );
//     // std::cout << hello ;
    
//     // write(new_socket , hello.c_str() , hello.length());
//     printf("------------------Hello message sent-------------------\n");
//     close(new_socket);
//     return;

// }

// pthread_create(&t,NULL,handle_connection,p_client);
        // handle_connection(new_socket);
        // int fd = open("index.html",O_RDONLY);
        // char buffer[100000] = {0};
        // int bytes_read = read(fd,buffer,sizeof(buffer));
        // if (bytes_read == 0){
        //     std::cout << "br";
        //     break;
        // } // We're done reading from the file
        

    
        // if (bytes_read < 0) {
        //     // handle errors
        // }
        // char *p = buffer;
        // while (bytes_read > 0) {
        //     int bytes_written = write(new_socket, p, bytes_read);
        //     if (bytes_written <= 0) {
        //     // handle errors
        // //     }
        // //     bytes_read -= bytes_written;
        // //     p += bytes_written;
        // // }
        // // close(fd);
        // // valread = read( new_socket , buffer, 30000);
        // // // printf("%s\n",buffer );
        // // // std::cout << hello ;
        
        // // // write(new_socket , hello.c_str() , hello.length());
        // // printf("------------------Hello message sent-------------------\n");
        // // close(new_socket);