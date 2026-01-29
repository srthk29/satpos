#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>

#include "CoordGeodetic.h"
#include "DateTime.h"
#include "Eci.h"
#include "SGP4.h"
#include "Tle.h"
#include "Util.h"

#include "Vector.h"
#include "api/v3/sat.pb.h"
#include "parse_tle.h"
#include "utils.h"

std::string get_tle(int catnr) {
    try {
        // What the previous example done there was simply
        // to create a curlpp::Easy class, which is the basic
        // object in cURLpp, and then set the Url option.
        // curlpp::options classes are the primitives that allow to specify
        // values to the requests.
        curlpp::Easy myRequest;

        const std::string url =
            "https://celestrak.org/NORAD/elements/gp.php?CATNR=" +
            std::to_string(catnr) + "&FORMAT=TLE";

        curlpp::options::Url myUrl(url);
        myRequest.setOpt(myUrl);
        myRequest.setOpt(curlpp::options::ConnectTimeout(5));
        myRequest.setOpt(curlpp::options::Timeout(5));

        // If we wanted to put the content of the URL within a string stream
        // (or any type of std::ostream, for that matter), like the first
        // example, we would use the WriteStream option like this:
        std::ostringstream os;
        curlpp::options::WriteStream ws(&os);
        myRequest.setOpt(ws);

        myRequest.perform();

        // There is some shortcut within curlpp that allow you to write shorter
        // code like this: os << myRequest;

        // That would do exactly what the previous code was doing.

        return os.str();
    } catch (const curlpp::RuntimeError &e) {
        throw std::runtime_error(std::string("curlpp runtime error: ") +
                                 e.what());
    } catch (const curlpp::LogicError &e) {
        throw std::runtime_error(std::string("curlpp logic error: ") +
                                 e.what());
    }
}

inline void FillEciStateVector(const libsgp4::Eci &eci,
                               api::v3::StateVector *out) {
    const libsgp4::Vector pos = eci.Position();
    const libsgp4::Vector vel = eci.Velocity();

    api::v3::Vector3 *position = out->mutable_position();
    position->set_x(pos.x);
    position->set_y(pos.y);
    position->set_z(pos.z);

    api::v3::Vector3 *velocity = out->mutable_velocity();
    velocity->set_x(vel.x);
    velocity->set_y(vel.y);
    velocity->set_z(vel.z);

    out->set_frame(api::v3::REFERENCE_FRAME_ECI);
}

inline void FillGeodeticPosition(const libsgp4::Eci &eci,
                                 api::v3::GeodeticPosition *out) {
    const libsgp4::CoordGeodetic geo = eci.ToGeodetic();

    out->set_latitude_deg(libsgp4::Util::RadiansToDegrees(geo.latitude));
    out->set_longitude_deg(libsgp4::Util::RadiansToDegrees(geo.longitude));
    out->set_altitude_km(geo.altitude);
}

void parse_tle(const std::string &body,
               api::v3::GetPropagationResponse *reply) {
    // libsgp4::DateTime utc(2026, 1, 2, 0, 0, 0.0);
    for (const auto &tlestruct : parser::parse_3le_direct(body)) {
        libsgp4::Tle tle(std::string{tlestruct.name},
                         std::string{tlestruct.line1},
                         std::string{tlestruct.line2});

        api::v3::Tle *ps_tle = reply->mutable_tle();
        ps_tle->set_satellite_name(tle.Name());
        ps_tle->set_satellite_number(tle.NoradNumber());
        ps_tle->set_line1(tle.Line1());
        ps_tle->set_line2(tle.Line2());

        api::v3::OrbitElements *oe = ps_tle->mutable_orbit_elements();
        oe->set_semi_major_axis_km(0.0);
        oe->set_eccentricity(tle.Eccentricity());
        oe->set_inclination_deg(tle.Inclination(true));
        oe->set_raan_deg(tle.RightAscendingNode(true));
        oe->set_argument_of_perigee_deg(tle.ArgumentPerigee(true));
        oe->set_true_anomaly_deg(tle.MeanAnomaly(true));

        libsgp4::DateTime epoch = tle.Epoch();

        ps_tle->mutable_epoch()->set_seconds(utils::to_unix_timestamp(epoch));

        // std::cout << "Epoch = " << tle.Epoch() << '\n';
        // std::cout << "Epoch Ticks = " << tle.Epoch().Ticks() << '\n';
        std::cout << tle.ToString() << '\n';

        // radians per minutes
        // the mean motion (revolutions per day)
        std::cout << "Mean Motion: " << tle.MeanMotion() << '\n';
        std::cout << "Orbital Period(in minutes): " << 1440 / tle.MeanMotion()
                  << '\n';

        libsgp4::SGP4 sat(tle);
        {
            // At TLE Epoch
            const libsgp4::Eci eci_epoch = sat.FindPosition(epoch);

            api::v3::Propagation *at_tle_epoch_prop =
                reply->mutable_at_tle_epoch();

            // --- ECI state vector ---
            FillEciStateVector(eci_epoch, at_tle_epoch_prop->mutable_state());

            // --- Geodetic position ---
            FillGeodeticPosition(eci_epoch,
                                 at_tle_epoch_prop->mutable_geodetic());

            // --- Metadata ---
            at_tle_epoch_prop->mutable_epoch()->set_seconds(
                utils::to_unix_timestamp(epoch));
            at_tle_epoch_prop->set_propagator(api::v3::PROPAGATOR_TYPE_SGP4);
        }

        libsgp4::DateTime now = libsgp4::DateTime::Now(true);
        {
            // At Now
            const libsgp4::Eci now_epoch = sat.FindPosition(now);

            api::v3::Propagation *at_now_utc_prop = reply->mutable_at_now_utc();

            // --- ECI state vector ---
            FillEciStateVector(now_epoch, at_now_utc_prop->mutable_state());

            // --- Geodetic position ---
            FillGeodeticPosition(now_epoch,
                                 at_now_utc_prop->mutable_geodetic());

            // --- Metadata ---
            at_now_utc_prop->mutable_epoch()->set_seconds(
                utils::to_unix_timestamp(epoch));
            at_now_utc_prop->set_propagator(api::v3::PROPAGATOR_TYPE_SGP4);
        }
        // std::cout << "Now = " << now.ToString() << '\n';
        // std::cout << "Now Ticks = " << now.Ticks() << '\n';
        reply->mutable_tle_age()->set_seconds(utils::to_unix_timestamp(now) -
                                              utils::to_unix_timestamp(epoch));

        for (int tick = -20; tick < 90; ++tick) {
            libsgp4::DateTime nowtick = now.AddMinutes(tick);
            std::cout << nowtick.ToString() << '\n';
            // std::cout << "Now + 10mins = " << now.ToString() << '\n';
            // std::cout << "Now Ticks + 10mins = " << now.Ticks() << '\n';
            {
                // At Now
                const libsgp4::Eci nowtick_epoch = sat.FindPosition(nowtick);

                api::v3::Propagation *at_nowtick_utc_prop =
                    reply->add_propagations();

                // --- ECI state vector ---
                FillEciStateVector(nowtick_epoch,
                                   at_nowtick_utc_prop->mutable_state());

                // --- Geodetic position ---
                FillGeodeticPosition(nowtick_epoch,
                                     at_nowtick_utc_prop->mutable_geodetic());

                // --- Metadata ---
                // now_unix + tick * 60
                at_nowtick_utc_prop->mutable_epoch()->set_seconds(
                    utils::to_unix_timestamp(nowtick));
                at_nowtick_utc_prop->set_propagator(
                    api::v3::PROPAGATOR_TYPE_SGP4);
            }
        }
    }
}
