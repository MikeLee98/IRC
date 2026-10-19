#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <bits/stdc++.h>

// #define MAX_PARAMS 15
// typedef struct Commands {
//     std::string commands[MAX_PARAMS];
// }   Commands;

// static Commands* parse_buff(std::string buffer){

//     int idx = -1;
//     if (!buffer.size()) return NULL;
//     Commands* cmd = new Commands;
//     idx = buffer.find(' ');
//     if (idx == std::string::npos) {
//         cmd->commands[0] = buffer;
//         return cmd;
//     }
//     cmd->commands[0] = buffer.substr(0, idx+1);
//     idx++;
//     // while (true){
//     // }
//     return cmd;
// }

int main(void){
    int soc = socket(AF_INET, SOCK_STREAM, 0); // TCP/IP
    if (soc < 0){
        std::cerr << "Socket: failed to create the socket!";
        exit(EXIT_FAILURE);
    }
    std::cout << "Socket: successfully created [#" << soc << "]" << std::endl;
    sockaddr_in srvAdd;
    std::memset(&srvAdd, 0, sizeof(srvAdd));

    srvAdd.sin_family = AF_INET;
    srvAdd.sin_port = htons(7777);
    srvAdd.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    setsockopt(soc, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (bind(soc, (struct sockaddr*)(&srvAdd), sizeof(srvAdd)) == -1){
        std::cerr << "Failed to bind the to the address" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "Server sucessfully binded to the address!" << std::endl;

    if (listen(soc, 5) == -1)
    {
        std::cerr << "Failed to listen to the socket." << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "Server is now listening to the socket at port 7777" << std::endl;
    char buff[2048];
    int csc = -1;
    int bread = 0;
    while (1){
        csc = accept(soc, NULL, NULL);
        if (csc == -1) {
            std::cerr << "Failed to accept connection" << std::endl;
            exit (EXIT_FAILURE);
        }
        std::memset(buff, '\0', sizeof(buff));
        while (1){
            bread = recv(csc, buff, sizeof(buff) - 1, 0);
            if (bread <= 0) break ;
            buff[bread] = 0;
            std::cout << "------ >> Buffer" << std::endl;
            std::cout << buff << std::endl;
            // Commands *cmd = parse_buff(buff);
            // std::cout << "Command sent: " << cmd->commands[0] << std::endl;
        }
    }
    return (EXIT_SUCCESS);
}