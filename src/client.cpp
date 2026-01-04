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

#include <iostream>
#include <ostream>
#include <iomanip>

struct OrbitalElements {
	// Identification
	std::string object_name;
	std::string object_id;
	std::string epoch;              // ISO-8601 timestamp

	// Orbital parameters
	double mean_motion;
	double eccentricity;
	double inclination;
	double ra_of_asc_node;
	double arg_of_pericenter;
	double mean_anomaly;

	// Metadata
	int ephemeris_type;
	std::string classification_type;
	int norad_cat_id;
	int element_set_no;
	int rev_at_epoch;

	// Drag / motion derivatives
	double bstar;
	double mean_motion_dot;
	double mean_motion_ddot;

	static OrbitalElements from_simdjson(simdjson::ondemand::object obj) {
		OrbitalElements e;

		e.object_name = std::string(obj["OBJECT_NAME"].get_string().value());
		e.object_id   = std::string(obj["OBJECT_ID"].get_string().value());

		// Epoch parsing
		//auto epoch_sv = obj["EPOCH"].get_string().value();
		// e.epoch = parse_iso8601_utc(epoch_sv);
		e.epoch = obj["EPOCH"].get_string().value();

		e.mean_motion        = double(obj["MEAN_MOTION"]);
		e.eccentricity       = double(obj["ECCENTRICITY"]);
		e.inclination        = double(obj["INCLINATION"]);
		e.ra_of_asc_node     = double(obj["RA_OF_ASC_NODE"]);
		e.arg_of_pericenter  = double(obj["ARG_OF_PERICENTER"]);
		e.mean_anomaly       = double(obj["MEAN_ANOMALY"]);

		e.ephemeris_type     = int64_t(obj["EPHEMERIS_TYPE"]);
		e.classification_type =
			std::string(obj["CLASSIFICATION_TYPE"].get_string().value());
		e.norad_cat_id       = int64_t(obj["NORAD_CAT_ID"]);
		e.element_set_no     = int64_t(obj["ELEMENT_SET_NO"]);
		e.rev_at_epoch       = int64_t(obj["REV_AT_EPOCH"]);

		e.bstar              = double(obj["BSTAR"]);
		e.mean_motion_dot    = double(obj["MEAN_MOTION_DOT"]);
		e.mean_motion_ddot   = double(obj["MEAN_MOTION_DDOT"]);

		return e;
	}
};

std::vector<OrbitalElements> parseOrbitalElements(const std::string& json) {
	simdjson::ondemand::parser parser;
	simdjson::padded_string padded(json);
	auto doc = parser.iterate(padded);

	std::vector<OrbitalElements> elements;

	for (auto obj : doc.get_array()) {
		elements.emplace_back(OrbitalElements::from_simdjson(obj));
	}
	return elements;
}

std::ostream& operator<<(std::ostream& os, const OrbitalElements& oe) {
	os << "OrbitalElements {\n";

	os << "  Identification:\n";
	os << "    Object Name        : " << oe.object_name << '\n';
	os << "    Object ID          : " << oe.object_id << '\n';
	os << "    Epoch              : " << oe.epoch << '\n';

	os << "  Orbital Parameters:\n";
	os << std::fixed << std::setprecision(8);
	os << "    Mean Motion        : " << oe.mean_motion << '\n';
	os << "    Eccentricity       : " << oe.eccentricity << '\n';
	os << "    Inclination        : " << oe.inclination << '\n';
	os << "    RA of Asc. Node    : " << oe.ra_of_asc_node << '\n';
	os << "    Arg. of Pericenter : " << oe.arg_of_pericenter << '\n';
	os << "    Mean Anomaly       : " << oe.mean_anomaly << '\n';

	os << "  Metadata:\n";
	os << "    Ephemeris Type     : " << oe.ephemeris_type << '\n';
	os << "    Classification     : " << oe.classification_type << '\n';
	os << "    NORAD Cat ID       : " << oe.norad_cat_id << '\n';
	os << "    Element Set No     : " << oe.element_set_no << '\n';
	os << "    Rev at Epoch       : " << oe.rev_at_epoch << '\n';

	os << "  Drag / Motion Derivatives:\n";
	os << std::scientific << std::setprecision(6);
	os << "    BSTAR              : " << oe.bstar << '\n';
	os << "    Mean Motion Dot    : " << oe.mean_motion_dot << '\n';
	os << "    Mean Motion DDot   : " << oe.mean_motion_ddot << '\n';

	os << "}";

	// Restore default formatting
	os.unsetf(std::ios::floatfield);
	os.precision(6);

	return os;
}

#define RAD2DEG (180.0 / M_PI)
#define DEG2RAD (M_PI / 180.0)

struct ThreeLE {
	std::string name;
	std::string line1;
	std::string line2;
};

// Trim from the start (in place)
inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// Trim from the end (in place)
inline void rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
	}).base(), s.end());
}

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

std::vector<ThreeLE> parse_3le_from_string(const std::string& all_tles) {
	std::vector<ThreeLE> result;

	std::istringstream iss(all_tles);
	std::string name, l1, l2;

	while (true) {
		// Read name (skip empty lines)
		while (std::getline(iss, name) && name.empty()) {}

		if (!iss) break;
		if (!std::getline(iss, l1)) break;
		if (!std::getline(iss, l2)) break;

		rtrim(l1);
		rtrim(l2);
		result.push_back({name, l1, l2});
	}

	return result;
}

int main() {
	// HTTPS
	httplib::Client cli("https://celestrak.org");

	if (auto res = cli.Get("/NORAD/elements/gp.php?GROUP=geo&FORMAT=tle")) {
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
			
			libsgp4::SGP4 sat(libsgp4::Tle(tle.name, tle.line1, tle.line2));
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

