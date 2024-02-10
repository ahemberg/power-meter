#include "http_requests.hpp"


HTTPResponse HTTPRequest::post(std::string url, std::string data, std::map<std::string, std::string> headers) {
    //TODO: specify port? Hanlde case where it exists or not!!

    std::ostringstream os;

    os << "POST " << extract_path(url) << " HTTP/1.1\r\n";
    os << "Host: " << extract_host(url, true) << "\r\n";

    for (auto const& [key, val] : headers) {
        os << key << ": " << val << "\r\n";
    }

    os << "Content-Length: "<< data.size() <<"\r\n";
    os << "\r\n";
    os << data;
    os << "\r\n";

    std::string raw_response = send_tls_request(extract_host(url, false), os.str(), extract_port(url), TLS_CLIENT_TIMEOUT_SECS);
    HTTPResponse response = HTTPResponse(raw_response);
    return response;
}

std::string HTTPRequest::extract_host(std::string url, bool include_port) {

    uint8_t pos;

    if (include_port) {
        pos = url.find_first_of("/");
    } else {
        pos = url.find_first_of(":");
        if (pos == url.length() -1) {
            //No port in url
            pos = url.find_first_of("/");
        }
    }

    return url.substr(0, pos);
}

std::string HTTPRequest::extract_path(std::string url) {
    uint8_t pos = url.find_first_of("/");
    return url.substr(pos);
}

uint16_t HTTPRequest::extract_port(std::string url) {
    uint8_t startpos = url.find_first_of(":");
    uint8_t endpos = url.find_first_of("/");

    if (startpos >= endpos) {
        //No port availble
        return 0;
    }
    //TODO this crashes if there is no port.
    return (uint16_t)stoi(url.substr(startpos+1, endpos-1));
}

HTTPResponse::HTTPResponse(std::string raw_response) {

    if (raw_response.rfind("HTTP/1.1 ", 0) != 0) {
        //This is probably not a http response
        code = 0;
        reason = "Invalid HTTP response";
        return;
    }

    //Only care about first line for now. If more detail about the response, like headers etc,
    //Just loop through the lines:
    std::istringstream f(raw_response);
    std::string first_line;
    std::getline(f, first_line);
    
    //Line should look like this: HTTP/1.1 204 No Content.
    //This way of parsing is error prone to say the least.
    code = stoi(first_line.substr(8,11));
    reason = first_line.substr(12);
}