#include "simdjson.h"

#include <iostream>
#include <ostream>
#include <iomanip>
#include "orbital_elements.h"

OrbitalElements OrbitalElements::from_simdjson(simdjson::ondemand::object obj) {
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
