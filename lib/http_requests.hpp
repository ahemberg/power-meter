//Sample response
// HTTP/1.1 204 No Content
// Content-Type: application/json
// Request-Id: 30789873-6d24-11ee-aa22-000000000000
// X-Influxdb-Build: OSS
// X-Influxdb-Version: 1.6.7~rc0
// X-Request-Id: 30789873-6d24-11ee-aa22-000000000000
// Date: Tue, 17 Oct 2023 19:34:29 GMT

#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>
#include <map>

#include "tls_common.hpp"

#define TLS_CLIENT_TIMEOUT_SECS  10

class HTTPResponse {
    public:
        HTTPResponse(std::string raw_response);
        uint16_t code;
        std::string reason;
        //Not implemented until needed for memory reasons
        //std::map<std::string, std::string> headers; //Will we ever use headers? Probably skip this in order to save memory. Can include in some other library
        //std::string raw_response
};

class HTTPRequest {
    public:
        static HTTPResponse post(std::string url, std::string data, std::map<std::string, std::string> headers);
        //Must verify if connected and catch that error. Otherwise we crash and burn
    private:
        static std::string extract_host(std::string url, bool include_port);
        static std::string extract_path(std::string url);
        static uint16_t extract_port(std::string url);
};