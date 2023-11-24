#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <cstddef>
#include <cassert>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <string>

#include "fastcgi.h"

enum class FCGIHeaderType {
    BEGIN_REQUEST = 1,
    END_REQUEST = 3,
    FCGI_PARAMS = 4,
    FCGI_STDIN = 5,
    FCGI_STDOUT = 6,
    FCGI_STDERR = 7,
};

struct FCGIHeader {
    int version;
    FCGIHeaderType type;
    int request_id;
    int content_length;
    int padding_length;
};

static constexpr const char* PORT = "8081";
static constexpr int BACKLOG = 10;
static constexpr int RECV_BUFFER_SIZE = 65535;

FCGIHeader parse_header(char buffer[], size_t buffer_index) {
    FCGIHeader result;
    result.version = static_cast<int>(static_cast<unsigned char>(buffer[buffer_index + 0]));
    std::cerr << result.version << '\n';
    assert(result.version == 1);
    result.type = static_cast<FCGIHeaderType>(static_cast<int>(static_cast<unsigned char>(buffer[buffer_index + 1])));
    result.request_id = static_cast<int>(static_cast<unsigned char>(buffer[buffer_index + 2])) * 256 + static_cast<int>(static_cast<unsigned  char>(buffer[buffer_index + 3]));
    result.content_length = static_cast<int>(static_cast<unsigned char>(buffer[buffer_index + 4])) * 256 + static_cast<int>(static_cast<unsigned char>(buffer[buffer_index + 5]));
    result.padding_length = static_cast<int>(static_cast<unsigned char>(buffer[buffer_index + 6]));
    return result;
}

struct FCGIRecord {
    int version;
    int type;
    int request_id;
    std::string content_data;
};


void fcgi_start(std::string (*handle_request)(std::string, const std::unordered_map<std::string, std::string>&)) {
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
            if (recv_count < 0) {
                continue;
            }
            std::cerr << "[VERBOSE] data received\n";
            std::cerr << recv_count << '\n';
            size_t buffer_index = 0;
            std::unordered_map<std::string, std::string> params;
            std::string fcgi_stdin;
            int request_id = 0;

#if 0
            std::ofstream raw_request ("raw.bin", std::ios::out | std::ios::binary);
            raw_request.write(recv_buffer, recv_count);
            raw_request.close();
#endif
            while (buffer_index < recv_count) {
                std::cerr << "[VERBOSE] buffer index: " << buffer_index << '\n';
                FCGIHeader header = parse_header(recv_buffer, buffer_index);
                if (request_id == 0) {
                    request_id = header.request_id;
                } else {
                    assert(request_id == header.request_id);
                }
                buffer_index += 8; // size of the fcgi header
                std::cerr << "[VERBOSE] header: \n"
                    << "[VERBOSE]      version: " << header.version << '\n'
                    << "[VERBOSE]      type: " << static_cast<int>(header.type) << '\n'
                    << "[VERBOSE]      request_id: " << header.request_id << '\n'
                    << "[VERBOSE]      content_length: " << header.content_length << '\n'
                    << "[VERBOSE]      padding_length: " << header.padding_length << '\n';
                switch (header.type) {
                    case FCGIHeaderType::BEGIN_REQUEST:
                        {
                            std::cerr << "[VERBOSE] fcgi begin request\n";
                            // doing nothing for now
                        } break;
                    case FCGIHeaderType::END_REQUEST:
                        {
                            std::cerr << "[VERBOSE] fcgi end request\n";
                            // doing nothing for now
                        } break;
                    case FCGIHeaderType::FCGI_PARAMS:
                        {
                            std::cerr << "[VERBOSE] fcgi params\n";
                            size_t param_buffer_index = 0;
                            while (param_buffer_index < header.content_length) {
                                std::cerr << "[VERBOSE] param buffer index = " << param_buffer_index << '\n';
                                int name_length = static_cast<int>(static_cast<unsigned char>(recv_buffer[buffer_index + param_buffer_index + 0]));
                                int value_length = static_cast<int>(static_cast<unsigned char>(recv_buffer[buffer_index + param_buffer_index + 1]));
                                assert(name_length >> 7 == 0);
                                assert(value_length >> 7 == 0);
                                std::string name(&recv_buffer[buffer_index + param_buffer_index + 2], name_length);
                                std::string value(&recv_buffer[buffer_index + param_buffer_index + name_length + 2], value_length);
                                std::cerr << "[VERBOSE] param name: " << name << " - param value: " << value << '\n';
                                params.emplace(name, value);
                                param_buffer_index += 2 + name_length + value_length;
                            }
                        } break;
                    case FCGIHeaderType::FCGI_STDIN:
                        {
                            std::cerr << "[VERBOSE] fcgi stdin\n";
                            fcgi_stdin += std::string(&recv_buffer[buffer_index], header.content_length);
                            std::cerr << "[VERBOSE] stdin: " << fcgi_stdin << '\n';
                        } break;
                    case FCGIHeaderType::FCGI_STDOUT:
                        {
                            std::cerr << "[VERBOSE] fcgi stdout\n";
                        } break;
                    case FCGIHeaderType::FCGI_STDERR:
                        {
                            std::cerr << "[VERBOSE] fcgi stderr\n";
                        } break;
                    default:
                        assert(false);
                }
                buffer_index += header.content_length + header.padding_length;
            }

            std::string fcgi_stdout = handle_request(std::move(fcgi_stdin), params);

            std::vector<char> response;
            response.push_back(static_cast<char>(1));
            response.push_back(static_cast<char>(FCGIHeaderType::FCGI_STDOUT));
            response.push_back(static_cast<char>(request_id / 256));
            response.push_back(static_cast<char>(request_id % 256));
            response.push_back(static_cast<char>(fcgi_stdout.size() / 256));
            response.push_back(static_cast<char>(fcgi_stdout.size() % 256));
            response.push_back(static_cast<char>(0));
            response.push_back(static_cast<char>(0));
            for (char c: fcgi_stdout) {
                response.push_back(c);
            }

            int bytes_sent = send(fd, response.data(), response.size(), 0);
            if (bytes_sent != response.size()) {
                std::cerr << "[ERROR] not enough bytes sent\n";
            } else {
                std::cerr << "[VERBOSE] " << bytes_sent << " bytes sent\n";
            }
            response.clear();

            response.push_back(static_cast<char>(1));
            response.push_back(static_cast<char>(FCGIHeaderType::FCGI_STDERR));
            response.push_back(static_cast<char>(request_id / 256));
            response.push_back(static_cast<char>(request_id % 256));
            response.push_back(static_cast<char>(0));
            response.push_back(static_cast<char>(0));
            response.push_back(static_cast<char>(0));
            response.push_back(static_cast<char>(0));

            bytes_sent = send(fd, response.data(), response.size(), 0);
            if (bytes_sent != response.size()) {
                std::cerr << "[ERROR] not enough bytes sent\n";
            } else {
                std::cerr << "[VERBOSE] " << bytes_sent << " bytes sent\n";
            }
            response.clear();

            response.push_back(static_cast<char>(1));
            response.push_back(static_cast<char>(FCGIHeaderType::END_REQUEST));
            response.push_back(static_cast<char>(request_id / 256));
            response.push_back(static_cast<char>(request_id % 256));
            response.push_back(static_cast<char>(0));
            response.push_back(static_cast<char>(8));
            response.push_back(static_cast<char>(0));
            response.push_back(static_cast<char>(0));

            response.push_back(static_cast<char>(0));
            response.push_back(static_cast<char>(0));
            response.push_back(static_cast<char>(0));
            response.push_back(static_cast<char>(0));
            response.push_back(static_cast<char>(0));
            response.push_back(static_cast<char>(0));
            response.push_back(static_cast<char>(0));
            response.push_back(static_cast<char>(0));

            //std::cerr << "[VERBOSE] response: ";
            //for (char byte: response) {
            //    std::cerr << std::hex << byte;
            //}
            //std::cerr << '\n';

            bytes_sent = send(fd, response.data(), response.size(), 0);
            if (bytes_sent != response.size()) {
                std::cerr << "[ERROR] not enough bytes sent\n";
            } else {
                std::cerr << "[VERBOSE] " << bytes_sent << " bytes sent\n";
            }

            memset(&recv_buffer, 0, RECV_BUFFER_SIZE);
        }
    }
}
