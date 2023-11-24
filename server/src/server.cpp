#include <string>
#include <unordered_map>
#include "fastcgi.h"

std::string handle_request(std::string fcgi_stdin, const std::unordered_map<std::string, std::string>& params) {
    std::string http_content = fcgi_stdin;
    std::string fcgi_stdout;
    fcgi_stdout += "HTTP/1.1 201 Created\r\n";
    fcgi_stdout += "Content-Length: ";
    fcgi_stdout += std::to_string(http_content.length());
    fcgi_stdout += "\r\n";
    fcgi_stdout += "\r\n";
    fcgi_stdout += http_content;

    return fcgi_stdout;
}

int main() {
    fcgi_start(&handle_request);
    return 0;
}
