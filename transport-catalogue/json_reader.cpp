#include "json_reader.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <iomanip>
#include <unordered_set>
#include <algorithm>


namespace transport_catalogue {
namespace handling_json {

namespace reader {

Stop FillInTheStop(json::Node& node) {
    Stop result;
    json::Dict dict = node.AsDict();
    
    result.name = dict.at("name").AsString();
    
    result.coord.lat = dict.at("latitude").AsDouble();
    result.coord.lng = dict.at("longitude").AsDouble();
    
    return result;
}

void FillInTheDistances (json::Node& node, TransportCatalogue& tc) {
    
    json::Dict dict = node.AsDict();
    
    json::Dict dict_distance_to_stops = dict.at("road_distances").AsDict();
    
    if (!dict_distance_to_stops.empty()) {
        std::string_view name_stop1 = dict.at("name").AsString();
        auto stop1 = tc.GetStop(name_stop1);
        
        std::string_view name_stop2;
        for (auto& [name_stop, node_distance] : dict_distance_to_stops) {
            name_stop2 = name_stop;
            tc.AddDistanc(stop1, tc.GetStop(name_stop2), node_distance.AsDouble());
        }
    }
    
}

Bus FillInTheBus(json::Node& node, TransportCatalogue& tc) {
    Bus result;
    json::Dict dict = node.AsDict();
    
    result.name = dict.at("name").AsString();
    result.is_roundtrip = dict.at("is_roundtrip").AsBool();
    
    json::Array stops = dict.at("stops").AsArray();
    
    for (auto& stop : stops) {
        result.stops.push_back(const_cast<Stop*>(tc.GetStop(stop.AsString())));
    }
    
    if (!result.is_roundtrip) {
        size_t number_of_stops_there = result.stops.size();
        
        for (size_t i = 2; i <= number_of_stops_there; ++i) {
            result.stops.push_back(result.stops.at(number_of_stops_there-i));
        }
    }
    
    return result;
}

void FillInTheData(const json::Node& node, TransportCatalogue& tc) {
    json::Array base_requests;
    
    std::vector<json::Node> stops;
    std::vector<json::Node> buses;
    
    if (node.IsArray()) {
        base_requests = node.AsArray();
        json::Dict dict; //Содержит map с полями входных данных
        json::Node buffer_request_type; //Содержит тип входных данных: остановка или автобус
        
        for (json::Node& base_request_node : base_requests) {
            if (base_request_node.IsDict()) {
                dict = base_request_node.AsDict();
                buffer_request_type = dict.at("type");
                
                if (buffer_request_type.AsString() == "Bus") {
                    buses.push_back(base_request_node);
                } else if (buffer_request_type.AsString() == "Stop") {
                    stops.push_back(base_request_node);
                }
            }
        }
    }
    
    for (auto& stop: stops) {
        tc.AddStop(FillInTheStop(stop));
    }
    
    for (auto& stop: stops) {
        FillInTheDistances(stop, tc);
    }
    
    for (auto& bus: buses) {
        tc.AddBus(FillInTheBus(bus, tc));
    }
    
}

svg::Color ParsingCollor(const json::Node& node) {
    svg::Color result;
    
    if (node.IsString()) {
        result = svg::Color(node.AsString());
    } else if (node.IsArray()) {
        json::Array array_color = node.AsArray();
        int red_ = array_color[0].AsInt();
        int green_ = array_color[1].AsInt();
        int blue_ = array_color[2].AsInt();
        
        if(array_color.size() == 4){
            double copacity_ = array_color[3].AsDouble();
            result = svg::Color(svg::Rgba(red_, green_, blue_, copacity_));
        } else if (array_color.size() == 3) {
            result = svg::Color(svg::Rgb(red_, green_, blue_));
        }
    }
    
    return result;
}

void FillInTheRenderSettings(const json::Node& node, renderer::RenderSettings& render_settings) {
    if (node.IsDict()) {
        json::Dict render_settings_map = node.AsDict();
        
        render_settings.width_ = render_settings_map.at("width").AsDouble();
        render_settings.height_ = render_settings_map.at("height").AsDouble();
        render_settings.padding_ = render_settings_map.at("padding").AsDouble();
        render_settings.line_width_ = render_settings_map.at("line_width").AsDouble();
        render_settings.stop_radius_ = render_settings_map.at("stop_radius").AsDouble();
        
        render_settings.bus_label_font_size_ = render_settings_map.at("bus_label_font_size").AsInt();
        
        if (render_settings_map.at("bus_label_offset").IsArray()) {
            json::Array bus_label_offset = render_settings_map.at("bus_label_offset").AsArray();
            render_settings.bus_label_offset_ = std::make_pair(bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble());
        }
        
        render_settings.stop_label_font_size_ = render_settings_map.at("stop_label_font_size").AsInt();
        
        if (render_settings_map.at("stop_label_offset").IsArray()) {
            json::Array stop_label_offset = render_settings_map.at("stop_label_offset").AsArray();
            render_settings.stop_label_offset_ = std::make_pair(stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble());
        }
        
        render_settings.underlayer_color_ = ParsingCollor(render_settings_map.at("underlayer_color"));
        
        render_settings.underlayer_width_ = render_settings_map.at("underlayer_width").AsDouble();
        
        if (render_settings_map.at("color_palette").IsArray()) {
            json::Array array_palette = render_settings_map.at("color_palette").AsArray();
            
            for (json::Node color_palette : array_palette) {
                render_settings.color_palette_.push_back(ParsingCollor(color_palette));
            }
        }
        
    }
}

void FillInTheRoutingSettings(const json::Node& node, router::RoutingSettings& routing_settings) {
    if (node.IsDict()) {
        json::Dict routing_settings_map = node.AsDict();
        
        routing_settings.bus_velocity = routing_settings_map.at("bus_velocity").AsDouble();
        routing_settings.bus_wait_time = routing_settings_map.at("bus_wait_time").AsDouble();
    }
}

} //end namespace reader

namespace request {

void ParsingRequest(const json::Node& node, std::vector<StatRequest>& stat_requests) {
    json::Array requests;
    
    if (node.IsArray()) {
        requests = node.AsArray();
        json::Dict buffer_dict;
        StatRequest buffer_request; //промежуточное хранилище данных о запросе
        
        for (const json::Node& request : requests) {
            buffer_dict = request.AsDict();
            buffer_request.id = buffer_dict.at("id").AsInt();
            buffer_request.type = buffer_dict.at("type").AsString();
            
            if ((buffer_request.type == "Bus") || (buffer_request.type == "Stop")) {
                buffer_request.name = buffer_dict.at("name").AsString();
                buffer_request.from = "";
                buffer_request.to = "";
            } else if (buffer_request.type == "Route") {
                buffer_request.name = "";
                buffer_request.from = buffer_dict.at("from").AsString();
                buffer_request.to = buffer_dict.at("to").AsString();
            } else if (buffer_request.type == "Map") {
                buffer_request.name = "";
                buffer_request.from = "";
                buffer_request.to = "";
            }
            
            stat_requests.push_back(buffer_request);
        }
    }
}

} //end namespace request

void SplittingDocument(json::Document& document_in,
                       renderer::RenderSettings& render_settings,
                       router::RoutingSettings& routing_settings,
                       TransportCatalogue& tc,
                       std::vector<StatRequest>& stat_requests) {
    json::Node node = document_in.GetRoot();
    
    if (node.IsDict()) {
        
        json::Dict dict = node.AsDict();
        
        reader::FillInTheData(dict.at("base_requests"), tc);
        reader::FillInTheRenderSettings(dict.at("render_settings"), render_settings);
        reader::FillInTheRoutingSettings(dict.at("routing_settings"), routing_settings);
        
        request::ParsingRequest(dict.at("stat_requests"), stat_requests);
        
    }
    
}

} //end namespace handling_json
} //end namespace transport_catalogue