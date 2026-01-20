#ifndef SATPOS_PARSE_TLE_H
#define SATPOS_PARSE_TLE_H
#include <string_view>
#include <vector>
namespace parser{
struct TLE {
	std::string_view name;
	std::string_view line1;
	std::string_view line2;
};

std::vector<TLE> parse_3le_direct(std::string_view str);
}
#endif
