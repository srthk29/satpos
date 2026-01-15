#include "SGP4.h"
#include "DateTime.h"
#include "CoordGeodetic.h"
#include "Eci.h"
#include "Tle.h"
#include "Util.h"

#include "parse_tle.h"

#include <iostream>
#include "api/v1/sat.pb.h"

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include <string>
#include <sstream>
#include <iostream>

std::string get_tle(int catnr, std::string& err) {
	try {
		// What the previous example done there was simply 
		// to create a curlpp::Easy class, which is the basic
		// object in cURLpp, and then set the Url option.
		// curlpp::options classes are the primitives that allow to specify 
		// values to the requests. 
		curlpp::Easy myRequest;

		curlpp::options::Url myUrl(std::string("https://celestrak.org/NORAD/elements/gp.php?CATNR="+std::to_string(catnr)+"&FORMAT=TLE"));
		myRequest.setOpt(myUrl);

		// If we wanted to put the content of the URL within a string stream
		// (or any type of std::ostream, for that matter), like the first example, 
		// we would use the WriteStream option like this:
		std::ostringstream os;
		curlpp::options::WriteStream ws(&os);
		myRequest.setOpt(ws);

		myRequest.perform();

		// There is some shortcut within curlpp that allow you to write shorter code
		// like this:
		// os << myRequest;

		// That would do exactly what the previous code was doing.
		
		return os.str();
	}
	catch(curlpp::RuntimeError & e) {
		std::cout << e.what() << std::endl;
		err = e.what();
	}
	catch(curlpp::LogicError & e) {
		std::cout << e.what() << std::endl;
		err = e.what();
	}

	return err;
}

void parse_tle(const std::string& body, propogation_service::PropogationReply* reply) {
	std::cout << "F: parse_tle:L:63\n";
	// libsgp4::DateTime utc(2026, 1, 2, 0, 0, 0.0);
	for (const auto& tlestruct : parse_3le_direct(body)) {
		libsgp4::Tle tle(
			std::string{tlestruct.name},
			std::string{tlestruct.line1},
			std::string{tlestruct.line2}
		);
		std::cout << "F: parse_tle:L:71\n";

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
		std::cout << "F: parse_tle:L:91\n";

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
	std::cout << "F: parse_tle:L:123\n";
}

