get-preact:
    curl -o static/htm.mjs https://esm.sh/v134/htm@3.1.1/esnext/htm.mjs
    curl -o static/preact.mjs https://esm.sh/stable/preact@10.19.2/esnext/preact.mjs

get-fcgi:
    curl -o extern/fcgi2.4.2.tar.gz -L https://github.com/FastCGI-Archives/fcgi2/archive/refs/tags/2.4.2.tar.gz
    tar xvf extern/fcgi2.4.2.tar.gz --directory=extern/
    rm extern/fcgi2.4.2.tar.gz

get-sqlite:
    curl -o extern/sqlite.zip https://www.sqlite.org/2024/sqlite-amalgamation-3450100.zip
    unzip extern/sqlite.zip -d extern
    rm extern/sqlite.zip

get-curl:
    curl -o extern/curl-8.6.0.tar.gz https://curl.se/download/curl-8.6.0.tar.gz
    tar xvf extern/curl-8.6.0.tar.gz --directory=extern/
    rm extern/curl-8.6.0.tar.gz

serve:
    lighttpd -D -f config/lighttpd.conf
