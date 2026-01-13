#include <iostream>
#include <csignal>
#include <string>
#include <thread>

#include <grpcpp/grpcpp.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/support/status.h>

#include "api/v1/sat.grpc.pb.h"
#include "api/v1/sat.pb.h"
#include "server.h"
#include "client.h"

// Each RPC becomes a virtual method you override
class PropogationServiceImpl final: public propogation_service::PropogationService::Service {
	public:
	grpc::Status GetPropogation(
		grpc::ServerContext* context,
		const propogation_service::PropogationRequest* request,
		propogation_service::PropogationReply* reply) override {

		/*
		propogation_service::TLE* tle = reply->mutable_tle();
		tle->set_name("ISS");
		tle->set_line1("LINE1");
		tle->set_line2("LINE2");
		propogation_service::Propogation* prop = reply->add_propogations();
		propogation_service::LatLng* latlng = prop->mutable_lat_lng();
		latlng->set_latitude(-42.90);
		latlng->set_longitude(150.90);
		propogation_service::Timestamp* timestamp = prop->mutable_timestamp();
		timestamp->set_seconds(177774374);
		prop->set_altitude_meters(454000);
		*/


		std::cout << "REQUEST: " << request->noradcategory() << '\n';

		std::string err{""};
		std::string tles = get_tle(request->noradcategory(), err);
		if (err != "") {
			return grpc::Status(grpc::INTERNAL, err);
		}

		std::cout << "TLES: " << tles << '\n';

		propogation_service::PropogationReply* rep = parse_tle(tles);
		reply = rep;

		return grpc::Status::OK;
	}
};

std::unique_ptr<grpc::Server> server;

/*
Shutdown():
1. Stops accepting new connections
2. Waits for in-flight RPCs to finish
3. Safe to call multiple times
*/
// https://grpc.github.io/grpc/cpp/classgrpc_1_1_server_interface.html#a6a1d337270116c95f387e0abf01f6c6c
void signal_handler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received. Shutting down server..." << std::endl;
    if (server) {
        std::thread shutdown_thread([]{ server->Shutdown(); });
        shutdown_thread.join();
    }
}

void run_server() {
	std::string server_address("127.0.0.1:50051");
	PropogationServiceImpl service;

	grpc::ServerBuilder builder;
	builder.AddListeningPort(
		server_address,
		grpc::InsecureServerCredentials());

	builder.RegisterService(&service);

	server = builder.BuildAndStart();

	std::cout << "Server listening on " << server_address << '\n';

	// void(*signal(int, void (*)(int)))(int);
	std::signal(SIGINT, signal_handler);
	std::signal(SIGTERM, signal_handler);

	server->Wait();

	std::cout << "Server shutdown gracefully\n";
}
