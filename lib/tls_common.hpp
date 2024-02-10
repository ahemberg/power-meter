#ifndef TLS_COMMON_H
#define TLS_COMMON

#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>
#include <time.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/dns.h"

typedef struct TLS_CLIENT_T_ {
    struct altcp_pcb *pcb;
    bool complete;
    int error;
    const char *http_request;
    int timeout;
    std::string server_response;
} TLS_CLIENT_T;

static struct altcp_tls_config *tls_config = NULL;

static err_t tls_client_close(void *arg);

static err_t tls_client_connected(void *arg, struct altcp_pcb *pcb, err_t err);

static err_t tls_client_poll(void *arg, struct altcp_pcb *pcb);

static void tls_client_err(void *arg, err_t err);

static err_t tls_client_recv(void *arg, struct altcp_pcb *pcb, struct pbuf *p, err_t err);

static void tls_client_connect_to_server_ip(const ip_addr_t *ipaddr, uint16_t port, TLS_CLIENT_T *state);

static void tls_client_dns_found(const char* hostname, const ip_addr_t *ipaddr, void *arg);

static bool tls_client_open(const char *hostname, const uint16_t port, void *arg);

// Perform initialisation
static TLS_CLIENT_T* tls_client_init(void);

std::string send_tls_request(std::string server, std::string request, uint16_t port, int timeout);

#endif