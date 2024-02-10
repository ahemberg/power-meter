#include "timeutils.hpp"


datetime_t time_to_datetime(time_t t) {
    struct tm *tm_time;

    tm_time = gmtime(&t);

    datetime_t time;
    time.year = tm_time->tm_year + 1900;
    time.month = tm_time->tm_mon + 1;
    time.dotw = tm_time->tm_wday;
    time.day = tm_time->tm_mday;
    time.hour = tm_time->tm_hour;
    time.min = tm_time->tm_min;
    time.sec = tm_time->tm_sec;
    return time;
}


time_t datetime_t_to_time_t(datetime_t t) {
    struct tm tm_time;

    tm_time.tm_year = t.year - 1900;
    tm_time.tm_mon = t.month -1;
    tm_time.tm_wday = t.dotw;
    tm_time.tm_mday = t.day;
    tm_time.tm_hour = t.hour;
    tm_time.tm_min = t.min;
    tm_time.tm_sec = t.sec;
    tm_time.tm_isdst = -1;

    return mktime(&tm_time);
}

time_t get_time() {
    // TODO: assumes that the rtc is correclty set. In the time-utils class this should be verifiable
    datetime_t t;
    //Check RTC initialised
    rtc_get_datetime(&t);

    //Make a time_t from year month day and so on
    //Convert this to unix timestamp using standard c. Don't reinvent the wheel

    return datetime_t_to_time_t(t);
}

/**
 * Blocking method that updates the internal RTC with current time from internet.
 * Requires that wifi is connected before called!
 * 
 * #TODO -> if wifi not connected, return false.
 */
bool update_rtc_from_ntp() {

    if (!rtc_running()) {
        rtc_init();
    }
    
    NTP_T *state = query_ntp();

    while(state->dns_request_sent) {
        //Waiting for response, poll 
        std::cout << "Waiting for NTP..." << std::endl;
        cyw43_arch_wait_for_work_until(at_the_end_of_time);
        cyw43_arch_poll();
        //sleep_ms(1000);
    }

    if (!state->request_successful) {
        std::cout << "Updating RTC failed!" << std::endl;
        return false;
    }

    // Clear state
    udp_remove(state->ntp_pcb);

    std::cout << "Updating RTC successful!" << std::endl;
    datetime_t t = time_to_datetime(state->epoch);   
    rtc_set_datetime(&t);
    sleep_us(64);
    return true;
}