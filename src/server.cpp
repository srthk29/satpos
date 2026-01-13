#include <grpcpp/grpcpp.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/support/status.h>
#include <string>
#include "api/v1/sat.grpc.pb.h"
#include "api/v1/sat.pb.h"

#include "server.h"

// Each RPC becomes a virtual method you override
class PropogationServiceImpl final: public propogation_service::PropogationService::Service {
	public:
	grpc::Status GetPropogation(
		grpc::ServerContext* context,
		const propogation_service::PropogationRequest* request,
		propogation_service::PropogationReply* reply) override {
		
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
	
		return grpc::Status::OK;
	}
};

void run_server() {
	std::string server_address("127.0.0.1:50051");
	PropogationServiceImpl service;

	grpc::ServerBuilder builder;
	builder.AddListeningPort(
		server_address,
		grpc::InsecureServerCredentials());

	builder.RegisterService(&service);

	std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

	std::cout << "Server listening on " << server_address << '\n';

	server->Wait();
}
