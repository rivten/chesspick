project(extern)
file(COPY fcgi_config.h DESTINATION fcgi2-2.4.2/include)
add_library(fcgi SHARED 
    fcgi2-2.4.2/libfcgi/fcgiapp.c
    fcgi2-2.4.2/libfcgi/fcgio.cpp
    fcgi2-2.4.2/libfcgi/fcgi_stdio.c
    $<$<PLATFORM_ID:Linux>:fcgi2-2.4.2/libfcgi/os_unix.c>
    $<$<PLATFORM_ID:Windows>:fcgi2-2.4.2/libfcgi/os_win32.c>)

target_include_directories(fcgi PUBLIC fcgi2-2.4.2/include)
target_include_directories(fcgi PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/fcgi2-2.4.2/include)

add_library(sqlite SHARED sqlite-amalgamation-3450100/sqlite3.c)
target_include_directories(sqlite INTERFACE sqlite-amalgamation-3450100)

set(BUILD_CURL_EXE OFF)

set(CURL_DISABLE_ALTSVC ON)
set(CURL_DISABLE_SRP ON)
set(CURL_DISABLE_KERBEROS_AUTH ON)
set(CURL_DISABLE_AWS ON)
set(CURL_DISABLE_DICT ON)
set(CURL_DISABLE_GETOPTIONS ON)
set(CURL_DISABLE_GOPHER ON)
set(CURL_DISABLE_HEADERS_API ON)
set(CURL_DISABLE_IMAP ON)
set(CURL_DISABLE_LDAP ON)
set(CURL_DISABLE_LDAPS ON)
set(CURL_DISABLE_MIME ON)
set(CURL_DISABLE_MQTT ON)
set(CURL_DISABLE_BINDLOCAL ON)
set(CURL_DISABLE_NETRC ON)
set(CURL_DISABLE_NTLM ON)
set(CURL_DISABLE_PARSEDATE ON)
set(CURL_DISABLE_POP3 ON)
set(CURL_DISABLE_PROGRESS_METER ON)
set(CURL_DISABLE_PROXY ON)
set(CURL_DISABLE_RTSP ON)
set(CURL_DISABLE_SHUFFLE_DNS ON)
set(CURL_DISABLE_SMB ON)
set(CURL_DISABLE_SMTP ON)
set(CURL_DISABLE_SOCKETPAIR ON)
set(CURL_DISABLE_TELNET ON)
set(CURL_DISABLE_TFTP ON)
set(CURL_DISABLE_VERBOSE_STRINGS ON)
set(BUILD_LIBCURL_DOCS OFF)
set(ENABLE_CURL_MANUAL OFF)

add_subdirectory(curl-8.6.0)
