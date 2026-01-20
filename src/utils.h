#ifndef UTILS_SRC_H
#define UTILS_SRC_H

#include <DateTime.h>
#include <ctime>
namespace utils {
time_t to_unix_timestamp(const libsgp4::DateTime& dt);
}
#endif
