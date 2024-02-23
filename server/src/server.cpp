#include <iostream>
#include <cstdlib>
#include <cassert>
#define NO_FCGI_DEFINES
#include <fcgi_stdio.h>
#include <cstring>
#include <string>
#include <unordered_map>
#include <format>
#include <sstream>

extern char** environ;

#define BUFFER_SIZE 1024
char buffer[BUFFER_SIZE];

static std::unordered_map<std::string, std::string> get_headers() {
    std::unordered_map<std::string, std::string> headers;
    for (char** envp = environ; *envp != nullptr; ++envp) {
        std::string e = std::string(*envp);

        size_t pos = e.find("=");
        assert(pos != e.npos);
        std::string header_name = e.substr(0, pos);
        std::string header_value = e.substr(pos + 1, e.length());
        headers.insert({header_name, header_value});
    }
    return headers;
}

enum class HttpMethod {
    Get,
    Post,
    Put,
    Patch,
    Delete,
    Head,
};

struct Request {
    HttpMethod method;
    std::string content;
    std::unordered_map<std::string, std::string> headers;
    std::string query_string;
    std::string script_name;
    std::string content_type;
    std::string accept;
};

struct Response {
    size_t status;
    std::unordered_map<std::string, std::string> headers;
    std::string content;
};

static HttpMethod parse_http_method(const char* method_str) {
    if (strcmp(method_str, "GET") == 0) {
        return HttpMethod::Get;
    }
    if (strcmp(method_str, "POST") == 0) {
        return HttpMethod::Post;
    }
    if (strcmp(method_str, "PATCH") == 0) {
        return HttpMethod::Patch;
    }
    if (strcmp(method_str, "PUT") == 0) {
        return HttpMethod::Put;
    }
    if (strcmp(method_str, "DELETE") == 0) {
        return HttpMethod::Delete;
    }
    if (strcmp(method_str, "HEAD") == 0) {
        return HttpMethod::Head;
    }
    assert(false);
}

Response handle_request(Request request) {
    if (request.script_name == "/api/pick") {
        return Response {
            201,
            {{"Content-Type", "application/json"}},
            std::format("\"{}\"", request.content),
        };
    }

    if (request.script_name == "/api/login") {
        return Response {
            302,
            {{"Location", "https://lichess.org"}},
            {},
        };
    }

    return Response {
        404,
        {},
        {},
    };
}

int main() {
    while (FCGI_Accept() >= 0) {
        const char* query_string = std::getenv("QUERY_STRING");
        const char* request_method = std::getenv("REQUEST_METHOD");
        const char* script_name = std::getenv("SCRIPT_NAME");
        const char* accept = std::getenv("HTTP_ACCEPT");
        const char* content_type = std::getenv("CONTENT_TYPE");
        const char* content_length_str = std::getenv("CONTENT_LENGTH");

        if (content_length_str != nullptr) {
            const long content_length = std::strtol(content_length_str, nullptr, 10);

            if (content_length > 0) {
                memset(buffer, 0, BUFFER_SIZE);
                std::cerr << "received " << content_length << '\n';
                assert(content_length < BUFFER_SIZE);
                size_t bytes_read = FCGI_fread(buffer, 1, content_length, FCGI_stdin);
                assert((long)bytes_read == content_length);
                std::cerr << buffer << '\n';
            }
        }

        Request request {
            parse_http_method(request_method),
            content_length_str ? std::string(buffer) : "",
            get_headers(),
            query_string ? std::string(query_string) : "",
            script_name ? std::string(script_name) : "",
            content_type ? std::string(content_type) : "",
            accept ? std::string(accept) : "",
        };

        Response response = handle_request(std::move(request));

        std::ostringstream response_headers;

        for (auto& [header_name, header_value]: response.headers) {
            response_headers << header_name << ":" << header_value;
        }

        FCGI_printf("Status: %u\r\n%s\r\n\r\n%s\n", response.status, response_headers.str().c_str(), response.content.c_str());
    }
    return 0;
}
