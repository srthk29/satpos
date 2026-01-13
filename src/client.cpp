#define CPPHTTPLIB_CLIENT_READ_TIMEOUT_USECOND 10

#include <iostream>

#include "httplib.h"
#include "SGP4.h"
#include "DateTime.h"
#include "Vector.h"
#include "CoordGeodetic.h"
#include "Eci.h"
#include "Tle.h"
#include "Util.h"
#include "parse.h"
#include "parse_tle.h"

#include "api/v1/sat.pb.h"
#include "client.h"

std::string get_tle(int catnr, std::string& err) {
	err.clear();

	static httplib::Client cli("https://celestrak.org");
	// httplib::Client cli("https://celestrak.org");
	cli.set_connection_timeout(5);
	cli.set_read_timeout(5);
	cli.set_keep_alive(true);

	const std::string path = "/NORAD/elements/gp.php?CATNR="+std::to_string(catnr)+"&FORMAT=TLE";

	std::cout << "PATH: " << path << '\n';

	httplib::Result res = cli.Get(path);
	if (!res) {
		err = httplib::to_string(res.error());
		return {};
	}

	if (res->status != 200) {
		err = "HTTP status: " + std::to_string(res->status);
		return {};
	}

	return res->body;
}

propogation_service::PropogationReply* parse_tle(const std::string& body) {
	propogation_service::PropogationReply* reply;

	// libsgp4::DateTime utc(2026, 1, 2, 0, 0, 0.0);
	for (const auto& tlestruct : parse_3le_direct(body)) {
		libsgp4::Tle tle(
			std::string{tlestruct.name},
			std::string{tlestruct.line1},
			std::string{tlestruct.line2}
		);

		propogation_service::TLE* ps_tle = reply->mutable_tle();
		ps_tle->set_name(tle.Name());
		ps_tle->set_line1(tle.Line1());
		ps_tle->set_line2(tle.Line2());

		std::cout << tlestruct.name << '\n';
		std::cout << tlestruct.line1 << '\n';
		std::cout << tlestruct.line2 << '\n';

		// std::cout << "Epoch = " << tle.Epoch() << '\n';
		// std::cout << "Epoch Ticks = " << tle.Epoch().Ticks() << '\n';
		std::cout << tle.ToString() << '\n';

		libsgp4::SGP4 sat(tle);

		libsgp4::DateTime now = libsgp4::DateTime::Now();
		// std::cout << "Now = " << now.ToString() << '\n';
		// std::cout << "Now Ticks = " << now.Ticks() << '\n';

		for (int tick = 0; tick < 90; ++tick) {
			// std::cout << "Now + 10mins = " << now.ToString() << '\n';
			// std::cout << "Now Ticks + 10mins = " << now.Ticks() << '\n';
			libsgp4::Eci eci = sat.FindPosition(now);
			// std::cout << "Velocity: " << eci.Velocity().ToString() << '\n';

			/*
				libsgp4::Vector pos = eci.Position();
				std::cout << "Position Magnitude: " << pos.Magnitude() << '\n';
				std::cout << "Position:" << pos.ToString() << '\n';
				*/

			libsgp4::CoordGeodetic geo = eci.ToGeodetic();
			std::cout << geo.ToString() << '\n';
			/*
				latitudes.push_back(libsgp4::Util::RadiansToDegrees(geo.latitude));
				longtitude.push_back(libsgp4::Util::RadiansToDegrees(geo.longitude));
				attitudes.push_back(geo.altitude);
				*/
			propogation_service::Propogation* prop = reply->add_propogations();
			propogation_service::LatLng* latlng = prop->mutable_lat_lng();
			latlng->set_latitude(libsgp4::Util::RadiansToDegrees(geo.latitude));
			latlng->set_longitude(libsgp4::Util::RadiansToDegrees(geo.longitude));
			propogation_service::Timestamp* timestamp = prop->mutable_timestamp();
			timestamp->set_seconds(now.Ticks());
			prop->set_altitude_meters(geo.altitude);

			now = now.AddMinutes(1);
		}
	}

	return reply;
}
