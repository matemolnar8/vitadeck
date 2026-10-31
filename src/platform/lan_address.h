#ifndef VD_LAN_ADDRESS_H
#define VD_LAN_ADDRESS_H

#include <stddef.h>

// Writes this device's IPv4 address on the LAN, or 127.0.0.1 when it has none.
void lan_address_detect(char *out, size_t out_size);

#endif /* VD_LAN_ADDRESS_H */
