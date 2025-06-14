#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"

const char *html =
    "HTTP/1.0 200 OK\r\nContent-type: text/html\r\n\r\n"
    "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">"
    "<title>Controller</title><style>"
    ".controller{display:flex;flex-direction:column;align-items:center;margin-top:50px;}"
    ".row{display:flex;justify-content:center;margin:5px;}"
    "button{width:60px;height:60px;font-size:24px;margin:5px;}"
    "</style></head><body>"
    "<div class=\"controller\">"
    "<div class=\"row\"><button onclick=\"alert('Up')\">&#8593;</button></div>"
    "<div class=\"row\">"
    "<button onclick=\"alert('Left')\">&#8592;</button>"
    "<button onclick=\"alert('Down')\">&#8595;</button>"
    "<button onclick=\"alert('Right')\">&#8594;</button>"
    "</div></div></body></html>";

err_t serve_page(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (p) {
        tcp_write(tpcb, html, strlen(html), TCP_WRITE_FLAG_COPY);
        tcp_close(tpcb);
        pbuf_free(p);
    }
    return ERR_OK;
}

err_t accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, serve_page);
    return ERR_OK;
}

int main() {
    stdio_init_all();
    if (cyw43_arch_init()) return 1;
    cyw43_arch_enable_sta_mode();
    // Parse secret.xml for SSID and password
    char ssid[64] = {0};
    char password[64] = {0};
    FILE *f = fopen("secret.xml", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            char *start, *end;
            if ((start = strstr(line, "<ssid>")) && (end = strstr(line, "</ssid>"))) {
                start += 6;
                size_t len = end - start;
                if (len < sizeof(ssid)) {
                    strncpy(ssid, start, len);
                    ssid[len] = '\0';
                }
            }
            if ((start = strstr(line, "<password>")) && (end = strstr(line, "</password>"))) {
                start += 10;
                size_t len = end - start;
                if (len < sizeof(password)) {
                    strncpy(password, start, len);
                    password[len] = '\0';
                }
            }
        }
        fclose(f);
    }
    cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, 30000);

    struct tcp_pcb *pcb = tcp_new();
    tcp_bind(pcb, IP_ADDR_ANY, 80);
    pcb = tcp_listen_with_backlog(pcb, 1);
    tcp_accept(pcb, accept_callback);

    while (true) {
        cyw43_arch_poll();
        sleep_ms(10);
    }
}