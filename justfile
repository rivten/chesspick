get-preact:
    curl -o static/htm.mjs https://esm.sh/v134/htm@3.1.1/esnext/htm.mjs
    curl -o static/preact.mjs https://esm.sh/stable/preact@10.19.2/esnext/preact.mjs

serve:
    lighttpd -D -f config/lighttpd.conf
