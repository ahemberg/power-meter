#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <deque>
#include <vector>
#include <algorithm>
#include <string>
#include <iostream>
#include <sstream>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/rtc.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#include "secrets.hpp"
#include "lib/timeutils.hpp"
#include "lib/measurement.hpp"
#include "lib/influxclient.hpp"


char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASS;

std::deque<Measurement> measurements;

///DATE AND WIFI CODE, BREAK OUT nicely

uint32_t get_unix_timestamp()
{
    return (uint32_t)get_time();
}

bool connect_wifi(char *ssid, char *pass)
{
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_UK))
    {
        printf("failed initialize\n");
        cyw43_arch_deinit();
        return false;
    }
    printf("initialized\n");

    cyw43_arch_enable_sta_mode();

    if (cyw43_arch_wifi_connect_timeout_ms(ssid, pass, CYW43_AUTH_WPA2_AES_PSK, 30000))
    {
        printf("failed to connect\n");
        cyw43_arch_deinit();
        return false;
    }
    printf("connected\n");
    return true;
}

void disconnect_wifi()
{
    void cyw43_arch_disable_sta_mode();
    cyw43_arch_deinit();
}

void block_until_wifi_connected(char *ssid, char *pass, uint32_t timeout)
{
    uint32_t delay_backoff = 1000;
    uint32_t time_waited = 0;
    while (!connect_wifi(ssid, pass))
    {
        std::cout << "Failed toconnect to Wifi. Will retry in " << delay_backoff / 1000 << " seconds" << std::endl;
        sleep_ms(delay_backoff); // todo: should be a deep sleep after a couple of retries.
        if (timeout != 0 && time_waited >= timeout)
        {
            return;
        }
        delay_backoff += 10000;
    }
}

// TODO will block forever if wifi not connected when called.
void block_until_rtc_updated(uint32_t timeout)
{
    uint32_t delay_backoff = 1;
    uint32_t time_waited = 0;
    while (!update_rtc_from_ntp())
    {
        std::cout << "Failed to update RTC. Will retry in " << delay_backoff / 1000 << " seconds" << std::endl;
        sleep_ms(delay_backoff); // Todo: this must be a deep sleep after a couple of tries
        if (timeout != 0 && time_waited >= timeout)
        {
            return;
        }
        delay_backoff += 10000;
    }
}


///Measurement code

Measurement calculate_power(absolute_time_t& start) {
    absolute_time_t stop = get_absolute_time();
    uint32_t timestamp = get_unix_timestamp();
    int64_t delta = absolute_time_diff_us (start, stop);
    double imps = 1000000.0 / ((double) delta);
    return Measurement(timestamp, imps * 3600);
}

//Send code

std::vector<Measurement> pop_n(std::deque<Measurement> &queue, uint16_t n) {
    std::vector<Measurement> measurements;

    uint16_t n_pop = queue.size() > n ? n : queue.size();

    measurements.reserve(n_pop);

    for (uint16_t i = 0; i< n_pop; i++) {
        measurements.push_back(queue.back());
        queue.pop_back();
    }
    return measurements;
}

std::string measurements_to_payload(std::vector<Measurement> &_measurements)
{
    std::ostringstream os;
    for (const auto &measurement : _measurements)
    {
        os << "power" << ","
           << "host=" << "power_meter" << " " << measurement.to_line() << "\n";
    }
    return os.str();
}




int main() {

    stdio_init_all();
    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);

    bool measuring = false;
    double power;
    absolute_time_t start;

    // Connect WIFI
    block_until_wifi_connected(ssid, pass, 0);

    // Update rtc with current time
    rtc_init();
    block_until_rtc_updated(0);
    sleep_ms(100);

    uint32_t timestamp_last_sent = get_unix_timestamp();


    while (1)
    {
        const float conversion_factor = 3.3f / (1 << 12);
        uint16_t result = adc_read();

        //This should be an interrupt, otherwise sending will lead to lower resolution
        //For now that is acceptable though, The value of .25 volts is highly dependent on
        //the phototransistor and the light level of the power meter led. Not resilient at all.
        if (result * conversion_factor > 0.25) {
            if (measuring) {            
                measurements.push_back(calculate_power(start));
            } else {
                measuring = true;
            }
            start = get_absolute_time();
            sleep_ms(50);
        }

        uint32_t current_time = get_unix_timestamp();

        if (measurements.size() > 100 || (current_time - timestamp_last_sent) > 300) {
            measuring = false;
            std::vector<Measurement> measurements_to_send = pop_n(measurements, 20);
            std::string payload = measurements_to_payload(measurements_to_send);
            std::cout << "Sending measurements" << std::endl;
            if (post_to_influx(INFLUX_SERVER, INFLUX_PORT, INFLUX_DATABASE, INFLUX_USER, INFLUX_PASSWORD, payload)) {
                std::cout << "Sending data success!" << std::endl;
            } else {
                // TODO Two failures in a row crashes the program. But only if dns resolves.
                std::cout << "Failed to send data!" << std::endl;
            }

            timestamp_last_sent = get_unix_timestamp();
        }
    }
}