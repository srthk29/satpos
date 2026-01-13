#ifndef CLIENT_SRC_H
#define CLIENT_SRC_H
#include <string>
#include "api/v1/sat.pb.h"

std::string get_tle(int catnr, std::string& err);
propogation_service::PropogationReply* parse_tle(const std::string& body);
#endif
