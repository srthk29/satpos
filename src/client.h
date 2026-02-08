#ifndef CLIENT_SRC_H
#define CLIENT_SRC_H
#include "api/v3/sat.pb.h"
#include <string>

std::string get_tle(int catnr);
void parse_tle(const std::string &body, orbit::GetPropagationResponse *reply);
#endif
