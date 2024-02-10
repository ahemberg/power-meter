#ifndef TIMEUTILS_HPP
#define TIMEUTILS_HPP

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <time.h>

#include "hardware/rtc.h"
#include "ntp.hpp"

// Converts datetime_t to time_t
time_t datetime_t_to_time_t(datetime_t t);

// Gets the current time as time_t;
time_t get_time();

// Converts time_t to pico datetime_t
datetime_t time_to_datetime(time_t t);

// Blocks until internal RTC is updated or ntp call fails. Returns true if successful
bool update_rtc_from_ntp();

#endif