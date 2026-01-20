#ifndef CLIENT_SRC_H
#define CLIENT_SRC_H
#include <string>
#include "api/v2/sat.pb.h"

std::string get_tle(int catnr, std::string& err);
void parse_tle(const std::string& body, satproto::PropogationReply* reply);
#endif
