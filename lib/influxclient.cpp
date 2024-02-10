#include "influxclient.hpp"

#define TLS_CLIENT_TIMEOUT_SECS 10

bool post_to_influx(std::string server_address, uint16_t port, std::string database, std::string username, std::string password, std::string payload)
{
    std::ostringstream os;
    os << server_address << ":" << port << "/write?db=" << database << "&precision=s&u=" << username << "&p=" << password;

    std::map<std::string, std::string> headers = {
        {"User-Agent", "curl/8.1.2"},
        {"Accept", "*/*"},
        {"Content-Type", "application/x-www-form-urlencoded"}};

    HTTPResponse response = HTTPRequest::post(os.str(), payload, headers);

    if (response.code == 200 || response.code == 204)
    {
        return true;
    }
    else
    {
        return false;
    }
}