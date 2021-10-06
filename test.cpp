#include <fstream>
#include <bits/stdc++.h>
#include <time.h>
#include <filesystem>
#include <chrono>
#include <sys/stat.h>
int main() {

        std::string filename = "ind.html";
        struct stat result;
        stat(filename.c_str(), &result);
    
        char buf[100];
        time_t mod_time = result.st_mtime;
        std::cout << std::to_string(mod_time) << "\n";
    
        
        tm timeptr, client,server;
        gmtime_r(&mod_time, &timeptr);
        strftime(buf,sizeof(buf), "%a, %d %b %Y %T", &timeptr);
        strptime(buf,"%a, %d %b %Y %T",&server);
        std::cout << buf << "\n";


        time_t server_t = mktime(&server);
        
        std::string buf1 = "Fri, 31 Dec 1999 23:59:59";
        strptime(buf1.c_str(),"%a, %d %b %Y %T",&client);
        time_t client_t = mktime(&client);


        std::cout <<buf1 << "\n";
        std::cout << client_t <<"\n";
        std::cout << server_t <<"\n";
}

