#ifndef INFLUXCLIENT_HPP
#define INFLUXCLIENT_HPP

#include <stdio.h>
#include <iostream>
#include <sstream>

#include "pico/stdlib.h"
#include "http_requests.hpp"

//TODO, should handle failed requests
bool post_to_influx(std::string server_address, uint16_t port, std::string database, std::string username, std::string password, std::string payload);

#endif