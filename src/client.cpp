#define CPPHTTPLIB_CLIENT_READ_TIMEOUT_USECOND 30

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

#include <iostream>
#include <ostream>
#include "api/v1/sat.pb.h"

#include "server.h"

int main() {
	run_server();
	return 0;
}

int main2() {
	// HTTPS
	httplib::Client cli("https://celestrak.org");


	propogation_service::PropogationRequest req;
	req.set_noradcategory(25544);

	std::cout << req.DebugString() << '\n';


	propogation_service::PropogationReply resp;
	propogation_service::TLE* tle = resp.mutable_tle();
	tle->set_name("ISS");
	tle->set_line1("LINE1");
	tle->set_line2("LINE2");
	propogation_service::Propogation* prop = resp.add_propogations();
	propogation_service::LatLng* latlng = prop->mutable_lat_lng();
	latlng->set_latitude(-42.90);
	latlng->set_longitude(150.90);
	propogation_service::Timestamp* timestamp = prop->mutable_timestamp();
	timestamp->set_seconds(177774374);
	prop->set_altitude_meters(454000);
	
	std::cout << resp.DebugString() << '\n';

	// if (auto res = cli.Get("/NORAD/elements/gp.php?GROUP=geo&FORMAT=tle")) {
	if (auto res = cli.Get("/NORAD/elements/gp.php?CATNR=25544&FORMAT=TLE")) {
		// libsgp4::DateTime utc(2026, 1, 2, 0, 0, 0.0);
		for (const auto& tlestruct : parse_3le_direct(res->body)) {
			libsgp4::Tle tle(
				std::string{tlestruct.name},
				std::string{tlestruct.line1},
				std::string{tlestruct.line2}
			);
	
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

				now = now.AddMinutes(1);
			}
		}
	} else {
		if (!res) {
			// Check the error type
			const auto err = res.error();

			switch (err) {
				case httplib::Error::SSLConnection:
					std::cout << "SSL connection failed, SSL error: "
						<< res.ssl_error() << std::endl;
					break;

				case httplib::Error::SSLLoadingCerts:
					std::cout << "SSL cert loading failed, OpenSSL error: "
						<< std::hex << res.ssl_openssl_error() << std::endl;
					break;

				case httplib::Error::SSLServerVerification:
					std::cout << "SSL verification failed, X509 error: "
						<< res.ssl_openssl_error() << std::endl;
					break;

				case httplib::Error::SSLServerHostnameVerification:
					std::cout << "SSL hostname verification failed, X509 error: "
						<< res.ssl_openssl_error() << std::endl;
					break;

				default:
					std::cout << "HTTP error: " << httplib::to_string(err) << std::endl;
			}
		}
	}

	/*
	// HTTP
	httplib::Client cli("http://yhirose.github.io");

	if (auto res = cli.Get("/hi")) {
		std::cout << res->status << '\n';
		std::cout << res->body << "\n";
	}
	*/

	return 0;
}

