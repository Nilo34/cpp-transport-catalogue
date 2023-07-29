#include "transport_catalogue.h"
#include "json_reader.h"
#include "request_handler.h"
#include "serialization.h"

#include <fstream>
#include <iostream>
#include <string_view>

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);
    
    transport_catalogue::TransportCatalogue TCatalogue;
    
    json::Document input_data;
    
    transport_catalogue::renderer::RenderSettings render_settings;
    transport_catalogue::router::RoutingSettings routing_setings;
    
    transport_catalogue::serialization::SerializationSettings serialization_settings;
    
    input_data = json::Load(std::cin);
    
    if (mode == "make_base"sv) {
        
        // make base here
        transport_catalogue::handling_json::SplittingDocumentMakeBase(input_data,
                                                                      render_settings,
                                                                      routing_setings,
                                                                      TCatalogue,
                                                                      serialization_settings);
        
        std::ofstream output(serialization_settings.file_name, std::ios::binary);
        transport_catalogue::serialization::Serialization(TCatalogue,
                                                          render_settings,
                                                          routing_setings,
                                                          output);

    } else if (mode == "process_requests"sv) {

        // process requests here
        std::vector<transport_catalogue::StatRequest> stat_requests;
        
        transport_catalogue::handling_json::SplittingDocumentProcessRequests(input_data,
                                                                             serialization_settings,
                                                                             stat_requests);
        
        std::ifstream input(serialization_settings.file_name, std::ios::binary);
        transport_catalogue::serialization::Deserialization(TCatalogue,
                                                            render_settings,
                                                            routing_setings,
                                                            input);
        
        json::Document result_data;
        
        transport_catalogue::renderer::MapRenderer map_renderer(render_settings);
        transport_catalogue::router::TransportRouter transport_router(TCatalogue, routing_setings);
    
        transport_catalogue::request_handler::RequestHandler request_handler(TCatalogue, 
                                                                             map_renderer, 
                                                                             transport_router);
        
        result_data = request_handler.ReplyToTheRequest(stat_requests);
    
        json::Print(result_data, std::cout);

    } else {
        PrintUsage();
        return 1;
    }
}