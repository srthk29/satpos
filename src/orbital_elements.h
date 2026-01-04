#ifndef SATPOS_ORBITAL_ELEMENTS_H
#define SATPOS_ORBITAL_ELEMENTS_H

#include "simdjson.h"

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

	static OrbitalElements from_simdjson(simdjson::ondemand::object obj);
};

std::vector<OrbitalElements> parseOrbitalElements(const std::string& json);

std::ostream& operator<<(std::ostream& os, const OrbitalElements& oe);
#endif
