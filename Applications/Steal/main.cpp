#include <iostream>
#include <vector>
#include <fstream>

#include <lemon/core/url.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <getopt.h>

int main(int argc, char** argv){
    int help = false;
    int verbose = false;
    
    option opts[] = {
        {"help", no_argument, &help, true},
        {"verbose", no_argument, nullptr, 'v'},
        {"output", required_argument, nullptr, 'o'},
    };

    char* outFile = nullptr;
    
    int option;
    while((option = getopt_long(argc, argv, "v", opts, nullptr)) >= 0){
        switch(option){
            case 'v':
                verbose = true;
                break;
            case 'o':
                outFile = optarg;
                break;
            case '?':
                std::cout << "Usage: " << argv[0] << "[options] <url>\nSee " << argv[0] << "--help\n";
                return 1;
        }
    }

    if(argc - optind < 1){
        std::cout << "Usage: " << argv[0] << " [options] <url>\nSee " << argv[0] << " --help\n";
        return 1;
    }

    if(help){
        std::cout << "steal [options] <url>\n"
            << "Steal data from <url> (Default HTTP)\n\n"
            << "--help              Show Help\n"
            << "-v, --verbose       Show extra information\n";

        return 0;
    }

    Lemon::URL url(argv[optind]);

    if(!url.IsValid() || !url.Host().length()){
        std::cout << "steal: Invalid/malformed URL";
        return 2;
    }

    if(url.Protocol().length() && url.Protocol().compare("http")){
        std::cout << "steal: Unsupported protocol: '" << url.Protocol() << "'\n";
        return 2;
    }

    std::ostream* out = nullptr;

    if(outFile){
        auto file = new std::ofstream(outFile, std::ofstream::out | std::ofstream::binary);

        if(!file->is_open()){
            return 3;
        }

        out = reinterpret_cast<std::ostream*>(file);
    } else {
        out = &std::cout;
    }

redirect:
    uint32_t ip;
    if(inet_pton(AF_INET, url.Host().c_str(), &ip) > 0){

    } else {
        addrinfo hint;
        memset(&hint, 0, sizeof(addrinfo));
        hint = { .ai_family = AF_INET, .ai_socktype = SOCK_STREAM };

        addrinfo* result;
        if(getaddrinfo(url.Host().c_str(), nullptr, &hint, &result)){
            std::cout << "steal: Failed to resolve host '" << url.Host() << "': " << strerror(errno) << "\n";
            return 4;
        }

        ip = reinterpret_cast<sockaddr_in*>(result->ai_addr)->sin_addr.s_addr;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in address;
    address.sin_addr.s_addr = ip;
    address.sin_family = AF_INET;
    address.sin_port = htons(80);

    if(int e = connect(sock, reinterpret_cast<sockaddr*>(&address), sizeof(sockaddr_in)); e){
        perror("steal: Failed to connect");
        return 10;
    }

    char request[4096];
    int reqLength = snprintf(request, 4096, "GET /%s HTTP/1.1\r\n\
Host: %s\r\n\
User-Agent: steal/0.1\r\n\
Upgrade-Insecure-Requests: 0\r\n\
Accept: */*\r\n\
\r\n", url.Resource().c_str(), url.Host().c_str());

    if(verbose){
        printf("%s\n---\n", request);
    }

    if(ssize_t len = send(sock, request, reqLength, 0); len != reqLength){
        if(reqLength < 0){
            perror("steal: Failed to send data");
        } else {
            std::cout << "steal: Failed to send all " << reqLength << " bytes (only sent " << len << ").\n";
        }

        return 11;
    }

    bool recievedHeader = false;

    char receiveBuffer[4096];
    while(ssize_t len = recv(sock, receiveBuffer, 4096, 0)){
        if(len < 0){
            perror("steal: Failed stealing data");
            return 12;
        }

        if(!(recievedHeader || verbose)){
            std::string line;
            unsigned i = 0;
            for(; i < len; i++){
                char c = receiveBuffer[i];
                if(c == '\n'){
                    if(line.find("Location: ") == 0){
                        url = Lemon::URL(line.substr(10).c_str());

                        if(!url.IsValid() || !url.Host().length()){
                            std::cout << "steal: Invalid/malformed redirect URL '" << line.substr(10) << "'.\n";
                            return 2;
                        }

                        if(url.Protocol().length() && url.Protocol().compare("http")){
                            std::cout << "steal: Unsupported redirect protocol: '" << url.Protocol() << "'\n";
                            return 2;
                        }
                        goto redirect;
                    } else if(!line.length()){
                        recievedHeader = true; // Empty line, header finished
                        break;
                    }

                    line.clear();
                } else if(c == '\r'){
                    // Ignore carriage return
                } else {
                    line += c;
                }
            }

            if(recievedHeader){
                out->write(receiveBuffer + i, len - i);
            }
        } else {
            out->write(receiveBuffer, len);
        }

        if(len < 4096){
            break; // No more data to recieve
        }
    } 

    return 0;
}
