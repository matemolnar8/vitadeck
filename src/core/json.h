#ifndef VD_JSON_H
#define VD_JSON_H

#include <stdbool.h>
#include <stddef.h>
#include "jimp/jimp.h"

// Consumes the next value, whatever its type, so a parser can ignore members it does not know.
bool json_skip_value(Jimp *jimp);
// Escapes `in` for use inside a JSON string. Never splits an escape; 6x the input length always fits.
void json_escape(char *out, size_t out_size, const char *in);

#endif /* VD_JSON_H */
