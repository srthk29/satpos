#ifndef SATPOS_PARSE_H
#define SATPOS_PARSE_H

#include <string>
#include <vector>

struct ThreeLE {
	std::string name;
	std::string line1;
	std::string line2;
};

// Trim from the start (in place)
inline void ltrim(std::string &s);

// Trim from the end (in place)
inline void rtrim(std::string &s);

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

std::vector<ThreeLE> parse_3le_from_string(const std::string& all_tles);

#endif
