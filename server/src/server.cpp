#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>

static constexpr const char* PORT = "8081";
static constexpr int BACKLOG = 10;
static constexpr int RECV_BUFFER_SIZE = 1024;

int main() {
#if 0
    struct addrinfo hints;
    struct addrinfo *servinfo;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (int rv = getaddrinfo(nullptr, PORT, &hints, &servinfo); rv != 0) {
        std::cerr << "[ERROR] getaddrinfo: " << gai_strerror(rv) << '\n';
        return 1;
    }

    struct addrinfo* p = nullptr;

    int sockfd;
    for (p = servinfo; p != nullptr; p = p->ai_next) {
        if (sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol); sockfd == -1) {
            continue;
        }

        int yes = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            std::exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo);

    if (p == nullptr) {
        std::cerr << "[ERROR] server failed to bind\n";
        std::exit(1);
    }
    if (listen(sockfd, BACKLOG) == -1) {
        std::cerr << "[ERROR] server failed to listen\n";
    }
#else
    int sockfd = 0;
#endif
    
    std::cerr << "[INFO] server waiting for connections\n";

    while (true) {
        std::cerr << "[VERBOSE] server waiting for a connection\n";
        struct sockaddr_storage their_addr;
        socklen_t sin_size = sizeof(their_addr);
        int fd = accept(sockfd, reinterpret_cast<struct sockaddr*>(&their_addr), &sin_size);
        if (fd == -1) {
            std::cerr << "[ERROR] server failed to accept\n";
        }
        std::cerr << "[VERBOSE] server received a connection\n";

        char recv_buffer[RECV_BUFFER_SIZE];
        for (int recv_count; (recv_count = recv(fd, &recv_buffer, RECV_BUFFER_SIZE, 0)) != 0;) {
            std::cerr << "[VERBOSE] data received\n";
            std::cerr << recv_count << '\n';
            for (int i = 0; i < recv_count; ++i) {
                std::cerr << std::hex << static_cast<int>(recv_buffer[i]);
            }
            memset(&recv_buffer, 0, RECV_BUFFER_SIZE);
        }
    }

    return 0;
}
