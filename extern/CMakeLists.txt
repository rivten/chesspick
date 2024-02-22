# FCGI
project(fcgi)
add_library(fcgi STATIC 
    fcgi2/libfcgi/fcgiapp.c
    fcgi2/libfcgi/fcgio.cpp
    fcgi2/libfcgi/fcgi_stdio.c
    $<$<PLATFORM_ID:Linux>:fcgi2/libfcgi/os_unix.c>
    $<$<PLATFORM_ID:Windows>:fcgi2/libfcgi/os_win32.c>)

target_include_directories(fcgi INTERFACE fcgi2/include)
