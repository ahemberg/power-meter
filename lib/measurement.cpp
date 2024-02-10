#include "measurement.hpp"

uint32_t Measurement::get_timestamp() const {
    return timestamp;
}

double Measurement::get_power() const {
    return power;
}

std::string Measurement::to_line() const {

    std::ostringstream os;
    os << "value" << "=" << power << " " << timestamp;
    return os.str();
}