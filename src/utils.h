#ifndef UTILS_SRC_H
#define UTILS_SRC_H

#include <DateTime.h>
#include <ctime>

time_t ToUnixTimestamp(const libsgp4::DateTime& dt);

#endif
