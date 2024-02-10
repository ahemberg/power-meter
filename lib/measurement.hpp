
#ifndef MEASUREMENT_HPP 
#define MEASUREMENT_HPP

#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>

class Measurement {
    public:
        Measurement(uint32_t _timestamp, double _power) : timestamp(_timestamp), power(_power) {};
        uint32_t get_timestamp() const;
        double get_power() const;
        std::string to_line() const;
    private:
        const uint32_t timestamp;
        const double power;
};

#endif
