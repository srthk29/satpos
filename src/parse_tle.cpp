#include "parse_tle.h"
#include <cstring> // Required for memchr
#include <string_view>
#include <vector>

std::vector<std::string_view> split_lines_memchr(std::string_view str) {
    std::vector<std::string_view> lines;

    // For 50-100MB, pre-reserving prevents the vector from
    // reallocating/copying its own pointer array multiple times.
    // lines.reserve(100000);

    const char *start = str.data();
    const char *end = start + str.size();
    const char *next_nl;

    while (start < end && (next_nl = static_cast<const char *>(std::memchr(
                               start, '\n', std::size_t(end - start))))) {
        // Create a view from the current start to the found newline
        lines.emplace_back(start, next_nl - start);

        // Move start to the character immediately after the newline
        start = next_nl + 1;
    }

    // Handle the remaining text if the string doesn't end with a newline
    if (start < end) {
        lines.emplace_back(start, end - start);
    }

    return lines;
}

std::vector<std::string_view> split_lines_optimized(std::string_view str) {
    std::vector<std::string_view> lines;

    // Optional: Pre-allocate memory if you have an estimate of line count
    // lines.reserve(100000);

    size_t start = 0;
    size_t end = str.find('\n');

    while (end != std::string_view::npos) {
        lines.emplace_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find('\n', start);
    }

    // Add the last segment if it doesn't end with a newline
    if (start < str.size()) {
        lines.emplace_back(str.substr(start));
    }

    return lines;
}

std::vector<parser::TLE> parse_3tle(std::string_view str) {
    std::vector<std::string_view> lines = split_lines_memchr(str);
    std::vector<parser::TLE> tles;
    tles.reserve(lines.size() / 3);

    for (size_t i = 0; i < lines.size(); i += 3) {
        tles.push_back({lines[i], lines[i + 1], lines[i + 2]});
        // tles.emplace_back(lines[0], lines[1], lines[2]);
    }

    return tles;
}

std::vector<parser::TLE> parser::parse_3le_direct(std::string_view str) {
    std::vector<parser::TLE> result;
    // result.reserve(str.size() / 210); // Heuristic: ~210 chars per TLE

    const char *curr = str.data();
    const char *end = curr + str.size();

    while (curr < end) {
        const char *n1 = static_cast<const char *>(
            std::memchr(curr, '\n', static_cast<size_t>(end - curr)));
        if (!n1)
            break;
        std::string_view name(curr, static_cast<size_t>(n1 - curr));

        const char *l1_start = n1 + 1;
        const char *n2 = static_cast<const char *>(
            std::memchr(l1_start, '\n', static_cast<size_t>(end - l1_start)));
        if (!n2)
            break;
        std::string_view line1(l1_start, static_cast<size_t>(n2 - l1_start));

        const char *l2_start = n2 + 1;
        const char *n3 = static_cast<const char *>(
            std::memchr(l2_start, '\n', static_cast<size_t>(end - l2_start)));

        // Handle last line which might not have a trailing newline
        std::string_view line2;
        if (n3) {
            line2 = {l2_start, static_cast<size_t>(n3 - l2_start)};
            curr = n3 + 1;
        } else {
            line2 = {l2_start, static_cast<size_t>(end - l2_start)};
            curr = end;
        }

        // Optional: Handle \r for cross-platform compatibility
        auto trim_r = [](std::string_view s) {
            if (!s.empty() && s.back() == '\r')
                s.remove_suffix(1);
            return s;
        };

        result.push_back({trim_r(name), trim_r(line1), trim_r(line2)});
    }
    return result;
}
