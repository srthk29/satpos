#define CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_CLIENT_READ_TIMEOUT_USECOND 30

#include "httplib.h"
#include "simdjson.h"
#include "SGP4.h"
#include "DateTime.h"
#include "Vector.h"
#include "CoordGeodetic.h"
#include "Eci.h"
#include "Tle.h"
#include "parse.h"
#include "orbital_elements.h"

#include <iostream>
#include <ostream>
#include <iomanip>

int main() {
	// HTTPS
	httplib::Client cli("https://celestrak.org");

	// if (auto res = cli.Get("/NORAD/elements/gp.php?GROUP=geo&FORMAT=tle")) {
	if (auto res = cli.Get("/NORAD/elements/gp.php?CATNR=25544&FORMAT=TLE")) {
		//std::cout << res->status << '\n';
		//std::cout << res->body << '\n';
		/*
		for (const auto& oe : parseOrbitalElements(res->body)) {
			std::cout << oe << '\n';
		}
		*/
		// libsgp4::DateTime utc(2026, 1, 2, 0, 0, 0.0);
		libsgp4::DateTime now = libsgp4::DateTime::Now();
		for (const auto& tle : parse_3le_from_string(res->body)) {
			// std::cout << tle.name << '\n' << tle.line1 << '\n' << tle.line2 << '\n' << '\n';
			// std::cout << "len - " << "line_one" << tle.line1.size() << '\t' << "line_two" << tle.line2.size() << '\n';
			// std::cout << tle.line1[tle.line1.size()-1] << '\n';
			// std::cout << tle.line2[tle.line2.size()-1] << '\n';
		
			libsgp4::Tle TLE(tle.name, tle.line1, tle.line2);
			std::cout << "Epoch = " << TLE.Epoch() << '\n';
			std::cout << TLE.ToString() << '\n';
			
			libsgp4::SGP4 sat(TLE);
			libsgp4::Eci eci = sat.FindPosition(now);

			std::cout << eci.Velocity().ToString() << '\n';
			
			libsgp4::Vector pos = eci.Position();
			libsgp4::CoordGeodetic geo = eci.ToGeodetic();

			std::cout << pos.Magnitude() << '\n';
			std::cout << pos.ToString() << '\n';
			/*
			std::cout << "ECI (km): "
				<< pos.x << " "
				<< pos.y << " "
				<< pos.z << "\n";
			*/

			std::cout << geo.ToString() << '\n';
			/*
			std::cout << "Lat: " << geo.latitude * RAD2DEG
				<< " Lon: " << geo.longitude * RAD2DEG
				<< " Alt(km): " << geo.altitude << "\n";
			*/
		}

		std::cout << now.ToString() << '\n'; 
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

}

