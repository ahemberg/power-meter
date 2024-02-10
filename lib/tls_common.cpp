/*
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tls_common.hpp"

static err_t tls_client_close(void *arg) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;
    err_t err = ERR_OK;

    state->complete = true;
    if (state->pcb != NULL) {
        altcp_arg(state->pcb, NULL);
        altcp_poll(state->pcb, NULL, 0);
        altcp_recv(state->pcb, NULL);
        altcp_err(state->pcb, NULL);
        err = altcp_close(state->pcb);
        if (err != ERR_OK) {
            printf("close failed %d, calling abort\n", err);
            altcp_abort(state->pcb);
            err = ERR_ABRT;
        }
        state->pcb = NULL;
    }
    return err;
}

static err_t tls_client_connected(void *arg, struct altcp_pcb *pcb, err_t err) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;
    if (err != ERR_OK) {
        printf("connect failed %d\n", err);
        return tls_client_close(state);
    }

    printf("connected to server, sending request\n");
    printf("***\nsending to server:\n***\n\n%s\n", state->http_request);
    err = altcp_write(state->pcb, state->http_request, strlen(state->http_request), TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        printf("error writing data, err=%d", err);
        return tls_client_close(state);
    }

    return ERR_OK;
}

static err_t tls_client_poll(void *arg, struct altcp_pcb *pcb) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;
    printf("timed out\n");
    state->error = PICO_ERROR_TIMEOUT;
    return tls_client_close(arg);
}

static void tls_client_err(void *arg, err_t err) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;
    printf("tls_client_err %d\n", err);
    tls_client_close(state);
    state->error = PICO_ERROR_GENERIC;
}

static err_t tls_client_recv(void *arg, struct altcp_pcb *pcb, struct pbuf *p, err_t err) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;
    if (!p) {
        printf("connection closed\n");
        return tls_client_close(state);
    }

    if (p->tot_len > 0) {     
        uint32_t data_len = p->tot_len;
        //char buf[data_len + 1];
        uint8_t inc = 10;
        char buf2[inc+1];
        uint32_t cursor = 0;
        std::ostringstream os;
        while (cursor < data_len) {  
            if ((cursor + inc) <= data_len) {
                pbuf_copy_partial(p, buf2, inc, cursor);
                buf2[inc+1] = 0;
            } else {
                for (uint8_t n = 0; n < inc+1; n++) {
                    buf2[n] = 0;
                }
                pbuf_copy_partial(p, buf2, (data_len - cursor), cursor);
            }
            os << buf2;
            printf("%s", buf2);
            cursor += inc;
        } 

        state->server_response = os.str();
        altcp_recved(pcb, p->tot_len);
        tls_client_close(state);
    }
    pbuf_free(p);

    return ERR_OK;
}

static void tls_client_connect_to_server_ip(const ip_addr_t *ipaddr, const uint16_t port, TLS_CLIENT_T *state)
{
    err_t err;
    //u16_t port = 8086; //TODO Make this a parameter!

    printf("connecting to server IP %s port %d\n", ipaddr_ntoa(ipaddr), port);
    err = altcp_connect(state->pcb, ipaddr, port, tls_client_connected);
    if (err != ERR_OK)
    {
        fprintf(stderr, "error initiating connect, err=%d\n", err);
        tls_client_close(state);
    }
}

static void tls_client_dns_found(const char* hostname, const ip_addr_t *ipaddr, void *arg)
{
    if (ipaddr)
    {
        printf("DNS resolving complete\n");
        //TODO this hardcoded port should be removed. Requires prot to be added to TLS_CLIENT_T
        tls_client_connect_to_server_ip(ipaddr, 8086, (TLS_CLIENT_T *) arg);
    }
    else
    {
        printf("error resolving hostname %s\n", hostname);
        tls_client_close(arg);
    }
}


static bool tls_client_open(const char *hostname, const uint16_t port, void *arg) {
    err_t err;
    ip_addr_t server_ip;
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;

    state->pcb = altcp_tls_new(tls_config, IPADDR_TYPE_ANY);
    if (!state->pcb) {
        printf("failed to create pcb\n");
        return false;
    }

    altcp_arg(state->pcb, state);
    altcp_poll(state->pcb, tls_client_poll, state->timeout * 2);
    altcp_recv(state->pcb, tls_client_recv);
    altcp_err(state->pcb, tls_client_err);

    mbedtls_ssl_context *ssl;
    altcp_tls_context(state->pcb);

    /* Set SNI */
    //mbedtls_ssl_set_hostname(state->pcb, hostname);

    printf("resolving %s\n", hostname);

    // cyw43_arch_lwip_begin/end should be used around calls into lwIP to ensure correct locking.
    // You can omit them if you are in a callback from lwIP. Note that when using pico_cyw_arch_poll
    // these calls are a no-op and can be omitted, but it is a good practice to use them in
    // case you switch the cyw43_arch type later.
    cyw43_arch_lwip_begin();

    //int err = dns_gethostbyname(NTP_SERVER, &state->ntp_server_address, ntp_dns_found, state);
    //TODO remember to pass port into callback somehow!
    err = dns_gethostbyname(hostname, &server_ip, tls_client_dns_found, state);
    if (err == ERR_OK)
    {
        /* host is in DNS cache */
        tls_client_connect_to_server_ip(&server_ip, port, state);
    }
    else if (err != ERR_INPROGRESS)
    {
        printf("error initiating DNS resolving, err=%d\n", err);
        tls_client_close(state->pcb);
    }

    cyw43_arch_lwip_end();

    return err == ERR_OK || err == ERR_INPROGRESS;
}

// Perform initialisation
static TLS_CLIENT_T* tls_client_init(void) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)calloc(1, sizeof(TLS_CLIENT_T));
    if (!state) {
        printf("failed to allocate state\n");
        return NULL;
    }

    return state;
}

//TODO: This crashes if the server has influxdb off. Probably crashes if server is down too
std::string send_tls_request(std::string server, std::string request, uint16_t port, int timeout) {

    tls_config = altcp_tls_create_config_client(NULL, 0);
    assert(tls_config);

    TLS_CLIENT_T *state = tls_client_init();
    if (!state) {
        std::cout << "Returning NULL. Will we crash?" << std::endl;
        return NULL; //TODO better way of representing error 
    }
    state->http_request = request.c_str();
    state->timeout = timeout;
    if (!tls_client_open(server.c_str(), port, state)) {
        std::cout << "Returning NULL. Will we crash?" << std::endl;
        return NULL; //TODO better way of representing error
    }
    while(!state->complete) {
        cyw43_arch_poll();
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(1000));
    }
    int err = state->error;
    std::string server_response = state->server_response;
    free(state);
    altcp_tls_free_config(tls_config);
    return server_response;
}
