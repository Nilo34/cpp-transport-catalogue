#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

#include "json_builder.h"

#include <optional>

namespace transport_catalogue {
namespace request_handler {

RequestHandler::RequestHandler(transport_catalogue::TransportCatalogue& db, 
                               const transport_catalogue::renderer::MapRenderer& renderer,
                               transport_catalogue::router::TransportRouter& router)
: db_(db)
, renderer_(renderer)
, router_(router)
{
}


json::Node RequestHandler::OutputTheBusData(StatRequest& stat_request) {
    json::Node node;
    
    transport_catalogue::Bus* bus = db_.GetBus(stat_request.name);
    
    using namespace std::string_literals;
    if (bus != nullptr) {
        node = json::Builder{}.
               StartDict().
               Key("curvature").Value(db_.GetDistanceTravelledBus(bus)/
                                      db_.GetDistanceBetweenVisitedStopsOnTheGround(bus)).
               Key("request_id").Value(stat_request.id).
               Key("route_length").Value(db_.GetDistanceTravelledBus(bus)).
               Key("stop_count").Value(int(bus->stops.size())).
               Key("unique_stop_count").Value(db_.GetNumberOfUniqueStops(bus)).
               EndDict().
               Build();
    } else {
        std::string not_found_str = "not found";
        node = json::Builder{}.
               StartDict().
               Key("request_id").Value(stat_request.id).
               Key("error_message").Value(not_found_str).
               EndDict().
               Build();
    }
    
    return node;
}

json::Node RequestHandler::OutputTheStopData(StatRequest& stat_request) {
    json::Node node;
    
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
        
        node = json::Builder{}.
               StartDict().
               Key("buses").Value(buffer).
               Key("request_id").Value(stat_request.id).
               EndDict().
               Build();
        
    } else {
        std::string not_found_str = "not found";
        node = json::Builder{}.
               StartDict().
               Key("request_id").Value(stat_request.id).
               Key("error_message").Value(not_found_str).
               EndDict().
               Build();
    }
    
    return node;
}

json::Node RequestHandler::OutputTheSVGMapData(StatRequest& stat_request) {
    json::Node node;
    
    std::ostringstream svg_stream;
    std::string svg_string;
    
    renderer_.RenderMap(svg_stream, db_);
    
    svg_string = svg_stream.str();
    
    node = json::Builder{}.
           StartDict().
           Key("map").Value(svg_string).
           Key("request_id").Value(stat_request.id).
           EndDict().
           Build();
    
    return node;
}

json::Node RequestHandler::OutputTheRouteData(StatRequest& stat_request) {
    json::Node node;
    
    std::optional<router::RouteData> route_data = router_.GetRouteInformation(router_.GetBusWaitingPeriod(db_.GetStop(stat_request.from)).start_bus_wait,
                                                                      router_.GetBusWaitingPeriod(db_.GetStop(stat_request.to)).start_bus_wait);
    
    if (route_data) {
        json::Array buffer_array_node;
        
        for (const auto& edge : route_data->edges) {
            buffer_array_node.emplace_back(std::visit(EdgePrinter{}, edge));
        }
        
        node = json::Builder{}.
               StartDict().
               Key("request_id").Value(stat_request.id).
               Key("total_time").Value(route_data->time).
               Key("items").Value(buffer_array_node).
               EndDict().
               Build();
        
    } else {
        std::string not_found_str = "not found";
        node = json::Builder{}.
               StartDict().
               Key("request_id").Value(stat_request.id).
               Key("error_message").Value(not_found_str).
               EndDict().
               Build();
    }
    
    return node;
}

json::Document RequestHandler::ReplyToTheRequest(
               std::vector<StatRequest>& stat_requests) {
    
    json::Array result;
    
    for (StatRequest& stat_request : stat_requests) {
        if (stat_request.type == "Bus") {
            result.push_back(OutputTheBusData(stat_request));
        } else if (stat_request.type == "Stop") {
            result.push_back(OutputTheStopData(stat_request));
        } else if (stat_request.type == "Map") {
            result.push_back(OutputTheSVGMapData(stat_request));
        } else if (stat_request.type == "Route") {
            result.push_back(OutputTheRouteData(stat_request));
        }
    }
    
    return json::Document{json::Node{result}};
}


json::Node RequestHandler::EdgePrinter::operator()(const router::StopEdge& stop_edge) {
    json::Node node;
    
    node = json::Builder{}.
           StartDict().
           Key("stop_name").Value(std::string(stop_edge.name)).
           Key("time").Value(stop_edge.time).
           Key("type").Value("Wait").
           EndDict().
           Build();
    
    return node;
}
json::Node RequestHandler::EdgePrinter::operator()(const router::BusEdge& bus_edge) {
    json::Node node;
    
    node = json::Builder{}.
           StartDict().
           Key("bus").Value(std::string(bus_edge.name)).
           Key("span_count").Value(bus_edge.number_of_stops).
           Key("time").Value(bus_edge.time).
           Key("type").Value("Bus").
           EndDict().
           Build();
    
    return node;
}

} //end namespace request_handler
} //end namespace transport_catalogue