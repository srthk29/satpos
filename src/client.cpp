#include <stdexcept>
#include <string>
#include <sstream>
#include <iostream>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include "SGP4.h"
#include "DateTime.h"
#include "CoordGeodetic.h"
#include "Eci.h"
#include "Tle.h"
#include "Util.h"

#include "parse_tle.h"
#include "api/v2/sat.pb.h"
#include "utils.h"

std::string get_tle(int catnr) {
	try {
		// What the previous example done there was simply 
		// to create a curlpp::Easy class, which is the basic
		// object in cURLpp, and then set the Url option.
		// curlpp::options classes are the primitives that allow to specify 
		// values to the requests. 
		curlpp::Easy myRequest;

		const std::string url = "https://celestrak.org/NORAD/elements/gp.php?CATNR="+std::to_string(catnr)+"&FORMAT=TLE";

		curlpp::options::Url myUrl(url);
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
	catch(const curlpp::RuntimeError& e) {
		throw std::runtime_error(
			std::string("curlpp runtime error: ") + e.what());
	}
	catch(const curlpp::LogicError& e) {
		throw std::runtime_error(
			std::string("curlpp logic error: ") + e.what());
	}
}

void parse_tle(const std::string& body, satproto::PropogationReply* reply) {
	// libsgp4::DateTime utc(2026, 1, 2, 0, 0, 0.0);
	for (const auto& tlestruct : parser::parse_3le_direct(body)) {
		libsgp4::Tle tle(
			std::string{tlestruct.name},
			std::string{tlestruct.line1},
			std::string{tlestruct.line2}
		);

		satproto::Tle* ps_tle = reply->mutable_tle();
		ps_tle->set_name(tle.Name());
		ps_tle->set_line1(tle.Line1());
		ps_tle->set_line2(tle.Line2());
	
		// std::cout << "Epoch = " << tle.Epoch() << '\n';
		// std::cout << "Epoch Ticks = " << tle.Epoch().Ticks() << '\n';
		std::cout << tle.ToString() << '\n';

		// radians per minutes
		// the mean motion (revolutions per day)
		std::cout << "Mean Motion: " << tle.MeanMotion() << '\n';
		std::cout << "Orbital Period(in minutes): " << 1440/tle.MeanMotion() << '\n';

		libsgp4::SGP4 sat(tle);
	
		libsgp4::DateTime epoch = tle.Epoch();
		time_t epoch_unix = utils::to_unix_timestamp(epoch);

		libsgp4::CoordGeodetic geo = sat.FindPosition(epoch).ToGeodetic();
		satproto::Propogation* prop = ps_tle->mutable_propogation();
		prop->set_latitude(libsgp4::Util::RadiansToDegrees(geo.latitude));
		prop->set_longitude(libsgp4::Util::RadiansToDegrees(geo.longitude));
		prop->set_timestamp(epoch_unix);
		prop->set_altitude(geo.altitude);
		
		libsgp4::DateTime now = libsgp4::DateTime::Now(true);
		time_t now_unix = utils::to_unix_timestamp(now);
		// std::cout << "Now = " << now.ToString() << '\n';
		// std::cout << "Now Ticks = " << now.Ticks() << '\n';
		ps_tle->set_age(now_unix-epoch_unix);

		for (int tick = -20; tick < 90; ++tick) {
			libsgp4::DateTime nowtick = now.AddMinutes(tick);
			std::cout << nowtick.ToString() << '\n';
			// std::cout << "Now + 10mins = " << now.ToString() << '\n';
			// std::cout << "Now Ticks + 10mins = " << now.Ticks() << '\n';
			libsgp4::Eci eci = sat.FindPosition(nowtick);
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

			satproto::Propogation* prop = reply->add_propogations();
			prop->set_latitude(libsgp4::Util::RadiansToDegrees(geo.latitude));
			prop->set_longitude(libsgp4::Util::RadiansToDegrees(geo.longitude));
			prop->set_timestamp(now_unix+tick*60);
			prop->set_altitude(geo.altitude);
		}
	}
}

