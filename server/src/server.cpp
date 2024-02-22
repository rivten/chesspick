#include <iostream>
#include <cstdlib>
#define NO_FCGI_DEFINES
#include <fcgi_stdio.h>

int main() {
    while (FCGI_Accept() >= 0) {
        FCGI_printf("Content-Type: application/json\r\nStatus: 400\r\n\r\n{\"hello\":\"world\"}\n");
        std::cerr << "received\n";
    }
    return 0;
}
