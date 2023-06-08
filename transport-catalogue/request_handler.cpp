#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

namespace transport_catalogue {
namespace request_handler {

RequestHandler::RequestHandler(transport_catalogue::TransportCatalogue& db, 
                               const transport_catalogue::renderer::MapRenderer& renderer)
: db_(db)
, renderer_(renderer)
{
}


json::Node RequestHandler::OutputTheBusData(handling_json::request::StatRequest& stat_request) {
    json::Dict dict;
    
    transport_catalogue::Bus* bus = db_.GetBus(stat_request.name);
    
    using namespace std::string_literals;
    if (bus != nullptr) {
        dict.emplace("curvature", json::Node{db_.GetDistanceTravelledBus(bus)/
                                             db_.GetDistanceBetweenVisitedStopsOnTheGround(bus)});
        dict.emplace("request_id", json::Node{stat_request.id});
        dict.emplace("route_length", json::Node{db_.GetDistanceTravelledBus(bus)});
        dict.emplace("stop_count", json::Node{int(bus->stops.size())});
        dict.emplace("unique_stop_count", json::Node{db_.GetNumberOfUniqueStops(bus)});
    } else {
        dict.emplace("request_id", json::Node{stat_request.id});
        std::string not_found_str = "not found";
        dict.emplace("error_message", json::Node{not_found_str});
    }
    
    return json::Node{dict};
}

json::Node RequestHandler::OutputTheStopData(handling_json::request::StatRequest& stat_request) {
    json::Dict dict;
    
    const transport_catalogue::Stop* stop = db_.GetStop(stat_request.name);
    
    if (stop != nullptr) {
        std::unordered_set<transport_catalogue::Bus*> bus_roster = db_.GetUniqueBusesOfStop(stop);
        
        std::vector<transport_catalogue::Bus*> sorted_name_buses({bus_roster.begin(), bus_roster.end()});
        std::sort(sorted_name_buses.begin(), sorted_name_buses.end(),
                  [] (transport_catalogue::Bus* rhs, transport_catalogue::Bus* lhs) {
                      return rhs->name < lhs->name;
                  });
        
        json::Array buffer;
        for (transport_catalogue::Bus* bus: sorted_name_buses) {
            buffer.push_back(json::Node{bus->name});
        }
        
        dict.emplace("buses", json::Node{buffer});
        dict.emplace("request_id", json::Node{stat_request.id});
        
    } else {
        dict.emplace("request_id", json::Node{stat_request.id});
        std::string not_found_str = "not found";
        dict.emplace("error_message", json::Node{not_found_str});
    }
    
    return json::Node{dict};
}

json::Node RequestHandler::OutputTheSVGMapData(handling_json::request::StatRequest& stat_request) {
    json::Dict result;
    
    std::ostringstream svg_stream;
    std::string svg_string;
    
    renderer_.RenderMap(svg_stream, db_);
    
    svg_string = svg_stream.str();
    
    result.emplace("map", json::Node(svg_string));
    result.emplace("request_id", json::Node(stat_request.id));
    
    return json::Node(result);
}

json::Document RequestHandler::ReplyToTheRequest(
               std::vector<handling_json::request::StatRequest>& stat_requests) {
    
    json::Array result;
    
    for (handling_json::request::StatRequest& stat_request : stat_requests) {
        if (stat_request.type == "Bus") {
            result.push_back(OutputTheBusData(stat_request));
        } else if (stat_request.type == "Stop") {
            result.push_back(OutputTheStopData(stat_request));
        } else if (stat_request.type == "Map") {
            result.push_back(OutputTheSVGMapData(stat_request));
        }
    }
    
    return json::Document{json::Node{result}};
}

} //end namespace request_handler
} //end namespace transport_catalogue