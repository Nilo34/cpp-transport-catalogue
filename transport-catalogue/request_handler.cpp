#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

namespace request_handler {

RequestHandler::RequestHandler(transport_catalogue::TransportCatalogue& db, 
                               const transport_catalogue::renderer::MapRenderer& renderer)
: db_(db)
, renderer_(renderer)
{
}

std::vector<geo::Coordinates> RequestHandler::GetStopsCoordinates() const {
    std::vector<geo::Coordinates> result; 
    std::unordered_map<std::string_view, transport_catalogue::Bus*> buses = db_.GetMapToBus();
    
    for (auto& [name, bus] : buses) {
        for (auto& stop : bus->stops) {
            result.push_back(stop->coord);
        }
    }
    return result;
}

void RequestHandler::RenderMapLine(std::vector<std::pair<transport_catalogue::Bus*, int>>& buses_palette, 
                                   svg::Document& document, 
                                   transport_catalogue::renderer::SphereProjector& sphere_projector) const {
    
    std::vector<geo::Coordinates> stops_coordinates; 
    
    for (auto [bus, palette] : buses_palette) {

        for (auto& stop : bus->stops) {
            stops_coordinates.push_back(stop->coord);
        }

        svg::Polyline bus_line;           

        for (auto& point : stops_coordinates) {
            bus_line.AddPoint(sphere_projector(point));
        } 

        if (!stops_coordinates.empty()) {
            renderer_.SetPolylineProperties(bus_line, palette);
            document.Add(bus_line);
        }

        stops_coordinates.clear();
    }
}

void RequestHandler::RenderMapRoutesName(std::vector<std::pair<transport_catalogue::Bus*, int>>& buses_palette, 
                                         svg::Document& document, 
                                         transport_catalogue::renderer::SphereProjector& sphere_projector) const {
    
    std::vector<geo::Coordinates> stops_coordinates;
    
    for (auto [bus, palette] : buses_palette) {  
        
        for (auto& stop : bus->stops) {
            stops_coordinates.push_back(stop->coord);
        }
        
        svg::Text route_name_backing_circular;
        svg::Text route_name_inscription_circular;
        svg::Text route_name_backing;
        svg::Text route_name_inscription; 
        
        if (!stops_coordinates.empty()) {
            if (bus->is_roundtrip) {
                renderer_.SetRouteNameBackingParameters(route_name_backing_circular,
                                                        std::string(bus->name),
                                                        sphere_projector(stops_coordinates[0]));
                document.Add(route_name_backing_circular);
                
                renderer_.SetRouteNameInscriptionParameters(route_name_inscription_circular,
                                                           std::string(bus->name),
                                                           sphere_projector(stops_coordinates[0]),
                                                           palette);
                document.Add(route_name_inscription_circular);
                
            } else {
                
                renderer_.SetRouteNameBackingParameters(route_name_backing_circular, 
                                                        std::string(bus->name),
                                                        sphere_projector(stops_coordinates[0]));
                document.Add(route_name_backing_circular);
                
                renderer_.SetRouteNameInscriptionParameters(route_name_inscription_circular,
                                                            std::string(bus->name),
                                                            sphere_projector(stops_coordinates[0]),
                                                            palette);
                document.Add(route_name_inscription_circular);
                
                if (stops_coordinates[0] != stops_coordinates[stops_coordinates.size()/2]) {
                    
                    renderer_.SetRouteNameBackingParameters(route_name_backing,
                                                            std::string(bus->name),
                                                            sphere_projector(stops_coordinates[stops_coordinates.size()/2]));
                    document.Add( route_name_backing);
                    
                    renderer_.SetRouteNameInscriptionParameters(route_name_inscription,
                                                                std::string(bus->name),
                                                                sphere_projector(stops_coordinates[stops_coordinates.size()/2]),
                                                                palette);
                    document.Add(route_name_inscription);
                }
            } 
        }
        
        stops_coordinates.clear();
    }
}

void RequestHandler::RenderMapStops(svg::Document& document, 
                                    transport_catalogue::renderer::SphereProjector& sphere_projector) const {
    
    std::vector<geo::Coordinates> stops_coordinates;    
    
    auto stops = db_.GetMapToStop();    
    std::vector<std::string_view> stops_names;
    
    for (auto& [name, stop] : stops) {
        
        if (stop->buses.size() > 0) {
            stops_names.push_back(name);
        }
    }
    
    std::sort(stops_names.begin(), stops_names.end());
    
    svg::Circle circle;
    transport_catalogue::Stop* buffer_stop;
    
    for(std::string_view stop_name : stops_names){
        buffer_stop = db_.GetStop(stop_name);
        
        if (buffer_stop) {
            renderer_.SetStopsCirclesParameters(circle,
                                                sphere_projector(buffer_stop->coord));
            document.Add(circle);
        }
    }
    
}

void RequestHandler::RenderMapStopsName(svg::Document& document, 
                                        transport_catalogue::renderer::SphereProjector& sphere_projector) const {
    std::vector<geo::Coordinates> stops_coordinates;
    
    auto stops = db_.GetMapToStop();    
    std::vector<std::string_view> stops_names;
    
    for (auto& [name, stop] : stops) {
        
        if (stop->buses.size() > 0) {
            stops_names.push_back(name);
        }
    }
    
    std::sort(stops_names.begin(), stops_names.end());
    
    svg::Text stop_name_backing;
    svg::Text stop_name_inscription;
    transport_catalogue::Stop* buffer_stop;
    
    for (std::string_view stop_name : stops_names) {
        buffer_stop = db_.GetStop(stop_name);
        
        if (buffer_stop) {        
            
            renderer_.SetStopNameBackingParameters(stop_name_backing, 
                                                   buffer_stop->name, 
                                                   sphere_projector(buffer_stop->coord));
            document.Add(stop_name_backing);
            
            renderer_.SetStopNameInscriptionParameters(stop_name_inscription, 
                                                       buffer_stop->name, 
                                                       sphere_projector(buffer_stop->coord));
            document.Add(stop_name_inscription);
        }
    }
    
}


void RequestHandler::RenderMap(std::ostream& out) const {
    svg::Document document;
    transport_catalogue::renderer::SphereProjector sphere_projector = renderer_.GetSphereProjector(GetStopsCoordinates());
    std::vector<std::string_view> buses_name; 
    std::vector<std::pair<transport_catalogue::Bus*, int>> buses_palette;   
    
    int palette_size = 0;
    int palette_index = 0;
    
    palette_size = renderer_.GetColorPaletteSize();
    
    std::unordered_map<std::string_view, transport_catalogue::Bus*> buses = db_.GetMapToBus();
    if (buses.size() > 0) {
        
        for (auto& [name, bus] : buses) {
            buses_name.push_back(name);
        }   
 
        std::sort(buses_name.begin(), buses_name.end());
 
        for (std::string_view bus_name : buses_name) {
            transport_catalogue::Bus* bus_info = db_.GetBus(bus_name);
 
            if (bus_info) {  
 
                if (bus_info->stops.size() > 0) {
                    buses_palette.push_back(std::make_pair(bus_info, palette_index));
                    palette_index++;
                    
                    if (palette_index == palette_size) {
                        palette_index = 0;
                    }
                }
            }
        }
 
        if (buses_palette.size() > 0) {
            
            if (buses_palette.size() > 0) {
                RenderMapLine(buses_palette, document, sphere_projector);
                
                RenderMapRoutesName(buses_palette, document, sphere_projector); 
            }
        }
    }
    
    RenderMapStops(document, sphere_projector);
    RenderMapStopsName(document, sphere_projector);
    
    document.Render(out);
}


json::Node RequestHandler::OutputTheBusData(domain::StatRequest& stat_request) {
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

json::Node RequestHandler::OutputTheStopData(domain::StatRequest& stat_request) {
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

json::Node RequestHandler::OutputTheSVGMapData(domain::StatRequest& stat_request) {
    json::Dict result;
    
    std::ostringstream svg_stream;
    std::string svg_string;
    
    RenderMap(svg_stream);
    
    svg_string = svg_stream.str();
    
    result.emplace("map", json::Node(svg_string));
    result.emplace("request_id", json::Node(stat_request.id));
    
    return json::Node(result);
}

json::Document RequestHandler::ReplyToTheRequest(std::vector<domain::StatRequest>& stat_requests) {
    json::Array result;
    
    for (domain::StatRequest& stat_request : stat_requests) {
        if (stat_request.type == "Bus") {
            //result.push_back(transport_catalogue::handling_json::request::OutputTheBusData(db_, stat_request));
            result.push_back(OutputTheBusData(stat_request));
        } else if (stat_request.type == "Stop") {
            //result.push_back(transport_catalogue::handling_json::request::OutputTheStopData(db_, stat_request));
            result.push_back(OutputTheStopData(stat_request));
        } else if (stat_request.type == "Map") {
            result.push_back(OutputTheSVGMapData(stat_request));
        }
    }
    
    return json::Document{json::Node{result}};
}

} //end namespace request_handler