#include <string>
#include <vector>
#include <sstream>
#include "parse.h"

// Trim from the start (in place)
inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// Trim from the end (in place)
inline void rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
	}).base(), s.end());
}

/*
// Source - https://stackoverflow.com/a
// Posted by Nicol Bolas, modified by community. See post 'Timeline' for change history
// Retrieved 2026-01-02, License - CC BY-SA 4.0
static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) {return !std::isspace(c);}));
    return s;
}

// Source - https://stackoverflow.com/a
// Posted by Nicol Bolas, modified by community. See post 'Timeline' for change history
// Retrieved 2026-01-02, License - CC BY-SA 4.0
static inline std::string &ltrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int c) {return !std::isspace(c);}), s.end());
    return s;
}
*/

std::vector<ThreeLE> parse_3le_from_string(const std::string& all_tles) {
	std::vector<ThreeLE> result;

	std::istringstream iss(all_tles);
	std::string name, l1, l2;

	while (true) {
		// Read name (skip empty lines)
		while (std::getline(iss, name) && name.empty()) {}

		if (!iss) break;
		if (!std::getline(iss, l1)) break;
		if (!std::getline(iss, l2)) break;

		rtrim(l1);
		rtrim(l2);
		result.push_back({name, l1, l2});
	}

	return result;
}
