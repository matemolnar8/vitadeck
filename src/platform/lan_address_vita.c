#include "platform/lan_address.h"

#include <psp2/net/netctl.h>
#include <stdio.h>
#include <string.h>

void lan_address_detect(char *out, size_t out_size)
{
    snprintf(out, out_size, "127.0.0.1");
    SceNetCtlInfo info;
    memset(&info, 0, sizeof(info));
    if (sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_IP_ADDRESS, &info) == 0 && info.ip_address[0] != '\0') {
        snprintf(out, out_size, "%s", info.ip_address);
    }
}
