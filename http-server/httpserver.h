#pragma once

#include "constants.h"

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
            {"Server", "linuxServer"}
        };
        headers.insert({"Date",std::string(buf) +" GMT"});


        status_codes = {
            {200, "OK"},
            {304, "Not Modified"},
            {404, "Not Found"},
            {501, "Not Implemented"}  
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

        /*storing each header line into vector 'lines'*/
        while((start = request.find_first_not_of(delim, end)) != std::string::npos)
        {
            end = request.find(delim, start);
            std::string header = request.substr(start, end - start);
            lines.push_back(header);

            /*check the Connection header in the request*/
            if(header.find("Connection")!=std::string::npos)
                conn_type = header.substr(header.find(":") + 2);  
            else conn_type = "close";

            /*check the If-Modified-Since header in the request*/
            if(header.find("If-Modified-Since")!=std::string::npos){
                conn_type = header.substr(header.find(":") + 2);  
                this->headers.insert({"If-Modified-Since",conn_type});
            }
        
        }

        std::string request_line = lines[0];
        end = 0;
        delim = " ";

        std::vector<std::string> parsed_req_line;

        /*storing the request line into a vector 'parsed_req_line'*/
        while((start = request_line.find_first_not_of(delim, end)) != std::string::npos)
        {
            end = request_line.find(delim, start);
            parsed_req_line.push_back(request_line.substr(start, end - start));
        }

        if(parsed_req_line.size()>0) method = parsed_req_line[0];
        if(parsed_req_line.size()>1) uri = parsed_req_line[1];
        if(parsed_req_line.size()>2) http_version = parsed_req_line[2];
    }
  

    std::string status_line(int status_code){
        std::string reason = status_codes[status_code];
        std::string response_line = "HTTP/1.1 " + std::to_string(status_code) + " " + status_codes[status_code] + "\r\n";
        return response_line;
    }


    std::string response_headers(std::unordered_map<std::string,std::string> extra_headers){
        std::string headers_response = "";
        
        for(auto it=headers.begin();it!=headers.end();it++){
            headers_response += it->first + ": " + it->second + "\r\n"; 
        }

        /*Addtional header*/
        if(!extra_headers.empty()){
            for(auto it=extra_headers.begin();it!=extra_headers.end();it++)
            headers_response += it->first + ": " + it->second + "\r\n";
        }
        
        return headers_response;
    }

    void handle_request(){
        std::string response; 
        
        /*Handle request according to method*/
        if(this->method == "GET"){

            /*check for the If-Modified-Since header in request*/
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

                /*If the requested has been modified then send the file otherwise send Not Modified status line(304)*/
                if(server_t > client_t) {
                    this->response = handle_GET();
                }
                else {
                    std::string response_line= status_line(304);
                    std::string header_lines = "Date: " + this->headers["Date"] + "\r\n" + "Content-Length: 0"; 
                    this->response = response_line+ header_lines;
                }
            }
            else this->response = handle_GET();
        }
        else if(this->method == "HEAD"){
            this->response = handle_HEAD();
        }
        else {
            this->response = status_line(501);
        }
        return;
    }

    std::string handle_GET(){
        char buffer[BUFSIZE];
        const char * uri = this->uri.substr(1).c_str();
        FILE *fp = fopen(uri,"r");


        std::string extension = this->uri.substr(this->uri.find('.')+1);
        std::string response_line;
        std::string content_type;
        std::string content_lenght;
        std::unordered_map<std::string,std::string> extra_headers;
        std::string header_lines;
        std::string blank_line = "\r\n";
        std::string body;

        /*if file does not exist send '404 Not Found' status line*/
        if(fp == NULL){
            response_line = status_line(404);
            body = "404 Not Found\r\n";
            content_type = "text/html";
            extra_headers["Content-Length"] = std::to_string(body.length()); 
            extra_headers["Content-Type"] = content_type;
            header_lines = response_headers(extra_headers);
            fp = NULL;
            return response_line+ header_lines + blank_line + body ;
        }

        /*handle GET request according to file type*/

        if(extension == "html"){
            size_t bytes_read;
            while((bytes_read = fread(buffer,1,BUFSIZE,fp))>0){
                body += buffer;
            }
            response_line= status_line(200);
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
            
            fseek(file_stream, 0, SEEK_END);
            long file_length = ftell(file_stream);
            rewind(file_stream);
            buffer1.resize(file_length);
            file_size = fread(&buffer1[0], 1, file_length, file_stream);
            for(int i = 0; i < file_size; i++)
            {
                body += buffer1[i];
            }
            response_line= status_line(200);
            content_type = (extension == "png" ? "image/png" : "image/jpeg");
            content_lenght = std::to_string(body.length());
            extra_headers["Content-Type"] = content_type;
            extra_headers["Content-Length"] = content_lenght;
            header_lines = response_headers(extra_headers);
            fclose(file_stream);
            file_stream = NULL;
            

        }
        
        return response_line+ header_lines + blank_line + body;
    }


    std::string handle_HEAD(){
        /*Response without the body*/
        
        char buffer[BUFSIZE]; 
        
        std::ifstream file(this->uri.substr(1), std::ifstream::ate | std::ifstream::binary);
        std::string response_line= status_line(200);
        std::string content_type = "text/html";
        std::string content_lenght = std::to_string(file.tellg());
        std::unordered_map<std::string,std::string> extra_headers;
        extra_headers["Content-Type"] = content_type;
        extra_headers["Content-Length"] = content_lenght;
        std::string header_lines = response_headers(extra_headers);

        return response_line+ header_lines;
    }

};

