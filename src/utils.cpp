#include <DateTime.h>
#include <ctime>

time_t ToUnixTimestamp(const libsgp4::DateTime& dt) {
    std::tm tm{};
    tm.tm_year = dt.Year() - 1900;  // years since 1900
    tm.tm_mon  = dt.Month() - 1;    // months since January [0,11]
    tm.tm_mday = dt.Day();
    tm.tm_hour = dt.Hour();
    tm.tm_min  = dt.Minute();
    tm.tm_sec  = static_cast<int>(dt.Second());
    tm.tm_isdst = 0;                // UTC

    return timegm(&tm);              // converts UTC → Unix epoch
}
