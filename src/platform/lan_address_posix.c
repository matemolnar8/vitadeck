#include "platform/lan_address.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

void lan_address_detect(char *out, size_t out_size)
{
    snprintf(out, out_size, "127.0.0.1");
    struct ifaddrs *ifaddr = NULL;
    if (getifaddrs(&ifaddr) != 0) return;
    for (struct ifaddrs *ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET) continue;
        struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
        const char *ip = inet_ntoa(addr->sin_addr);
        if (!ip || strncmp(ip, "127.", 4) == 0) continue;
        snprintf(out, out_size, "%s", ip);
        break;
    }
    freeifaddrs(ifaddr);
}
