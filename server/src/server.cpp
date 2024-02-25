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
#include <vector>
#include <random>
#include <algorithm>

#include <sqlite3.h>
#include <sodium/crypto_hash_sha256.h>
#include <sodium/utils.h>

#include <curl/curl.h>
#include <cJSON.h>

#define CLIENT_ID "rivten.chesspick"
#define LICHESS_LOGIN_REDIRECT_URI "http://localhost:8080/api/lichess-callback"

extern char** environ;

#define BUFFER_SIZE 1024
char buffer[BUFFER_SIZE];

sqlite3* db_connection;

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
    // TODO: fail ?
    return HttpMethod::Get;
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
        std::ostringstream redirection;

        std::random_device rd;
        std::vector<unsigned char> bits(32);
        std::generate(std::begin(bits), std::end(bits), std::ref(rd));

        std::string verifier(sodium_base64_encoded_len(bits.size(), sodium_base64_VARIANT_URLSAFE_NO_PADDING), '\0');
        sodium_bin2base64(verifier.data(), verifier.size(), bits.data(), bits.size(), sodium_base64_VARIANT_URLSAFE_NO_PADDING);
        std::cerr << verifier << '\n';

        std::vector<unsigned char> challenge_before_base64(crypto_hash_sha256_BYTES);
        crypto_hash_sha256(challenge_before_base64.data(), (const unsigned char*)verifier.c_str(), verifier.size() - 1);

        std::string challenge(sodium_base64_encoded_len(challenge_before_base64.size(), sodium_base64_VARIANT_URLSAFE_NO_PADDING), '\0');
        sodium_bin2base64(challenge.data(), challenge.size(), (const unsigned char*)challenge_before_base64.data(), challenge_before_base64.size(), sodium_base64_VARIANT_URLSAFE_NO_PADDING);

        sqlite3_stmt* stmt;
        std::string sql {"INSERT INTO lichess_login_verifiers (verifier, epoch) VALUES (?, unixepoch());"};
        int prepare_result = sqlite3_prepare_v2(db_connection, sql.c_str(), sql.length(), &stmt, nullptr);
        assert(prepare_result == SQLITE_OK);

        int bind_result = sqlite3_bind_text(stmt, 1, verifier.c_str(), verifier.length(), SQLITE_STATIC);
        assert(bind_result == SQLITE_OK);
        int step_result = sqlite3_step(stmt);
        assert(step_result == SQLITE_DONE);

        int finalize_result = sqlite3_finalize(stmt);
        assert(finalize_result == SQLITE_OK);

        int state = sqlite3_last_insert_rowid(db_connection);

        redirection << "https://lichess.org/oauth?response_type=code&client_id=" 
            << CLIENT_ID << "&redirect_uri=" << LICHESS_LOGIN_REDIRECT_URI
            << "&state=" << state
            << "&scope=preference:read&code_challenge_method=S256&code_challenge=" << challenge;

        return Response {
            302,
            {{"Location", redirection.str()}},
        };
    }

    if (request.script_name == "/api/lichess-callback") {
        std::string query_string = request.headers.find("QUERY_STRING")->second;

        std::string code;
        std::string state;

        size_t p = -1;
        do {
            p++;
            size_t next_p = query_string.find('&', p);
            std::string s = query_string.substr(p, next_p - p);

            size_t eql = s.find('=');
            std::string param = s.substr(0, eql);
            if (param == "code") {
                code = s.substr(eql + 1);
            } else if (param == "state") {
                state = s.substr(eql + 1);
            }

            p = next_p;
        } while (p != std::string::npos);

        assert(code.size() != 0);
        assert(state.size() != 0);

        sqlite3_stmt* stmt;
        std::string sql {"SELECT verifier FROM lichess_login_verifiers WHERE id = ?;"};

        int prepare_result = sqlite3_prepare_v2(db_connection, sql.c_str(), sql.size(), &stmt, nullptr);
        assert(prepare_result == SQLITE_OK);

        int bind_result = sqlite3_bind_text(stmt, 1, state.c_str(), state.size(), SQLITE_STATIC);
        assert(bind_result == SQLITE_OK);

        int step_result = sqlite3_step(stmt);
        assert(step_result == SQLITE_ROW);
        std::string verifier {(const char*)sqlite3_column_text(stmt, 0)};
        std::cerr << verifier << '\n';

        int finalize_result = sqlite3_finalize(stmt);
        assert(finalize_result == SQLITE_OK);

        // TODO: delete the verifier row. it won't be used again

        std::string access_token;
        {
            std::ostringstream post_data_builder;
            post_data_builder
                << "grant_type=authorization_code&"
                << "redirect_uri=" << LICHESS_LOGIN_REDIRECT_URI << "&"
                << "client_id=" << CLIENT_ID << "&"
                << "code=" << code << "&"
                << "code_verifier=" << verifier;

            CURL* curl = curl_easy_init();
            assert(curl != nullptr);

            //struct curl_slist* headers = nullptr;
            //headers = curl_slist_append(headers, "Content-Type: application/json");

            curl_easy_setopt(curl, CURLOPT_URL, "https://lichess.org/api/token");
            // TODO: not sure why it doesn't work with just POSTFIELDS
            curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, post_data_builder.str().c_str());
            //curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
            std::string response;
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                +[](void* buffer, size_t size, size_t nmemb, void* user_data) -> size_t {
                    std::string* response = (std::string*) user_data;
                    response->append((char*)buffer, nmemb * size);
                    return nmemb * size;
                });
            CURLcode res = curl_easy_perform(curl);
            //std::cerr << curl_easy_strerror(res) << '\n';
            assert(res == CURLE_OK);
            curl_easy_cleanup(curl);

            cJSON* json = cJSON_ParseWithLength(response.c_str(), response.size());

            char* out = cJSON_Print(json);
            std::cerr << out << '\n';

            cJSON_free(out);

            cJSON* access_token_json = cJSON_GetObjectItemCaseSensitive(json, "access_token");
            access_token = std::string{access_token_json->valuestring};

            cJSON_Delete(json);
        }

        // get lichess user
        std::ostringstream header;
        header << "Authorization: Bearer " << access_token;
        CURL* curl = curl_easy_init();
        assert(curl != nullptr);
        curl_easy_setopt(curl, CURLOPT_URL, "https://lichess.org/api/account");
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, header.str().c_str());

        std::string response;
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
            +[](void* buffer, size_t size, size_t nmemb, void* user_data) -> size_t {
                std::string* response = (std::string*) user_data;
                response->append((char*)buffer, nmemb * size);
                return nmemb * size;
            });
        CURLcode res = curl_easy_perform(curl);
        //std::cerr << curl_easy_strerror(res) << '\n';
        assert(res == CURLE_OK);
        curl_easy_cleanup(curl);

        return Response {
            200,
            {{"Content-Type", "text/html"}},
            response,
        };
    }

    return Response {
        404,
        {},
        {},
    };
}

int main() {
    int open_result = sqlite3_open_v2("../../db.db", &db_connection, SQLITE_OPEN_READWRITE, nullptr);
    assert(open_result == SQLITE_OK);
    assert(db_connection != nullptr);

    curl_global_init(CURL_GLOBAL_ALL);

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
