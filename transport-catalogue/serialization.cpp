#include "serialization.h"

#include <utility>
#include <string_view>
#include <algorithm>
#include <iterator>


namespace transport_catalogue {
namespace serialization {

namespace serialization_detail {

template <typename Iter>
uint32_t FindIdStops(Iter start, Iter end, std::string_view stop_name) {
    auto stop_it = std::find_if(start, end, [&stop_name] (const Stop& stop) {
                                                return stop.name == stop_name;
                                            });
    return std::distance(start, stop_it);
}

} //end namespace serialization_detail

transport_catalogue_serialize::TransportCatalogue TransportCatalogueSerialization(const TransportCatalogue& transport_catalogue) {
    transport_catalogue_serialize::TransportCatalogue result;
    
    const std::deque<Stop>& stops = transport_catalogue.GetStops();
    const std::deque<Bus>& buses = transport_catalogue.GetBuses();
    const std::unordered_map<std::pair<const Stop*, const Stop*>, double, 
                             transport_catalogue::detail::HasherDistancesBetweenStops>& distances_between_stops
                             = transport_catalogue.GetDistancesBetweenStops();
    // не забыть разнести по отдельным функциям для лучшей читаемости
    int id = 0;
    
    for (const Stop& stop : stops) { 
        transport_catalogue_serialize::Stop stop_proto;
        
        stop_proto.set_id(id);
        stop_proto.set_name(stop.name);
        stop_proto.mutable_coordinates()->set_latitude(stop.coord.lat);
        stop_proto.mutable_coordinates()->set_longitude(stop.coord.lng);
        
        *result.add_stops() = std::move(stop_proto);
        ++id;
    }
    
    for (const Bus& bus : buses) {
        transport_catalogue_serialize::Bus bus_proto;
        
        bus_proto.set_name(bus.name);
        
        for (const Stop* stop : bus.stops) {
            bus_proto.add_stops(serialization_detail::FindIdStops(stops.cbegin(),
                                                                  stops.cend(),
                                                                  stop->name));
        }
        
        bus_proto.set_is_roundtrip(bus.is_roundtrip);
        
        *result.add_buses() = std::move(bus_proto);
    }
    
    for (const auto& [pair_stops, distance] : distances_between_stops) {
        
        transport_catalogue_serialize::Distance distance_proto;
        
        distance_proto.set_start_stop_id(serialization_detail::FindIdStops(stops.cbegin(), 
                                                                           stops.cend(), 
                                                                           pair_stops.first->name));
        distance_proto.set_end_stop_id(serialization_detail::FindIdStops(stops.cbegin(), 
                                                                         stops.cend(), 
                                                                         pair_stops.second->name));
        
        distance_proto.set_distance(distance);
        
        *result.add_distances() = std::move(distance_proto);
    }
    
    return result;
}

colors_serialize::Color ColorSerialization(const svg::Color& color) {
    colors_serialize::Color result;
    
    if (std::holds_alternative<svg::Rgb>(color)) {
        
        svg::Rgb rgb = std::get<svg::Rgb>(color);
        
        result.mutable_rgb()->set_red(rgb.red);
        result.mutable_rgb()->set_green(rgb.green);
        result.mutable_rgb()->set_blue(rgb.blue);
        
    } else if (std::holds_alternative<svg::Rgba>(color)) {
        
        svg::Rgba rgba = std::get<svg::Rgba>(color);
        
        result.mutable_rgba()->set_red(rgba.red);
        result.mutable_rgba()->set_green(rgba.green);
        result.mutable_rgba()->set_blue(rgba.blue);
        result.mutable_rgba()->set_opacity(rgba.opacity);
        
    } else if (std::holds_alternative<std::string>(color)) {
        
        result.set_custom_color(std::get<std::string>(color));
        
    } else if (std::holds_alternative<std::monostate>(color)) {
        
        result.set_has_monostate(true);
        
    }
    
    return result;
}

render_serialize::RenderSettings RenderSettingsSerialization(const renderer::RenderSettings& render_settings) {
    render_serialize::RenderSettings result;
    
    result.set_width(render_settings.width_);
    result.set_height(render_settings.height_);
    result.set_padding(render_settings.padding_);
    result.set_stop_radius(render_settings.stop_radius_);
    result.set_line_width(render_settings.line_width_);
    result.set_bus_label_font_size(render_settings.bus_label_font_size_);
    
    render_serialize::Pair bus_label_offset_proto;
    bus_label_offset_proto.set_first(render_settings.bus_label_offset_.first);
    bus_label_offset_proto.set_second(render_settings.bus_label_offset_.second);
    *result.mutable_bus_label_offset() = std::move(bus_label_offset_proto);
    
    result.set_stop_label_font_size(render_settings.stop_label_font_size_);
    
    render_serialize::Pair stop_label_offset_proto;
    stop_label_offset_proto.set_first(render_settings.stop_label_offset_.first);
    stop_label_offset_proto.set_second(render_settings.stop_label_offset_.second);
    *result.mutable_stop_label_offset() = std::move(stop_label_offset_proto);
    
    *result.mutable_underlayer_color() = std::move(ColorSerialization(render_settings.underlayer_color_));
    
    result.set_underlayer_width(render_settings.underlayer_width_);
    
    for (const auto& color : render_settings.color_palette_) {
        *result.add_color_palette() = std::move(ColorSerialization(color));
    }
    
    return result;
}

transport_router_serialize::RoutingSettings RoutingSettingsSerialization(const router::RoutingSettings& routing_settings) {
    transport_router_serialize::RoutingSettings result;
    
    result.set_bus_wait_time(routing_settings.bus_wait_time);
    result.set_bus_velocity(routing_settings.bus_velocity);
    
    return result;
}

void Serialization(const TransportCatalogue& transport_catalogue,
                   const renderer::RenderSettings& render_settings,
                   const router::RoutingSettings& routing_settings,
                   std::ostream& output) {
    
    transport_catalogue_serialize::AggregatedData aggregated_data_proto;
    
    transport_catalogue_serialize::TransportCatalogue transport_catalogue_proto = 
        TransportCatalogueSerialization(transport_catalogue);
    render_serialize::RenderSettings render_settings_proto = RenderSettingsSerialization(render_settings);
    transport_router_serialize::RoutingSettings routing_settings_proto = RoutingSettingsSerialization(routing_settings);
    
    *aggregated_data_proto.mutable_transport_catalogue() = std::move(transport_catalogue_proto);
    *aggregated_data_proto.mutable_render_settings() = std::move(render_settings_proto);
    *aggregated_data_proto.mutable_routing_settings() = std::move(routing_settings_proto);
    
    aggregated_data_proto.SerializeToOstream(&output);
}

//-------------------DESERIALIZATION--------------------


TransportCatalogue TransportCatalogueDeserialization(const transport_catalogue_serialize::TransportCatalogue& transport_catalogue_proto) {
    transport_catalogue::TransportCatalogue result;
    
    const auto& stops_proto = transport_catalogue_proto.stops();
    const auto& buses_proto = transport_catalogue_proto.buses();
    const auto& distances_between_stops_proto = transport_catalogue_proto.distances();
    
    for (const auto& stop_proto : stops_proto) {
        Stop stop;
        
        stop.name = stop_proto.name();
        stop.coord.lat = stop_proto.coordinates().latitude();
        stop.coord.lng = stop_proto.coordinates().longitude();
        
        result.AddStop(std::move(stop));
    }
    
    const std::deque<Stop>& stops = result.GetStops();
    
    for (const auto& distance_proto : distances_between_stops_proto) {
        
        result.AddDistanc(result.GetStop(stops[distance_proto.start_stop_id()].name), 
                          result.GetStop(stops[distance_proto.end_stop_id()].name),
                          distance_proto.distance());
    }
    
    for (const auto& bus_proto : buses_proto) {
        Bus bus;
        
        bus.name = bus_proto.name();
        
        for (uint32_t stop_id : bus_proto.stops()) {
            bus.stops.push_back(result.GetStop(stops[stop_id].name));
        }
        
        bus.is_roundtrip = bus_proto.is_roundtrip();
        
        result.AddBus(std::move(bus));
    }
    
    return result;
}

svg::Color ColorDeserialization(const colors_serialize::Color& color_proto) {
    svg::Color result;
    
    if (color_proto.has_rgb()) {
        
        svg::Rgb rgb;
        
        rgb.red = color_proto.rgb().red();
        rgb.green = color_proto.rgb().green();
        rgb.blue = color_proto.rgb().blue();
        
        result = rgb;
        
    } else if (color_proto.has_rgba()) {
        
        svg::Rgba rgba;
        
        rgba.red = color_proto.rgba().red();
        rgba.green = color_proto.rgba().green();
        rgba.blue = color_proto.rgba().blue();
        rgba.opacity = color_proto.rgba().opacity();
        
        result = rgba;
        
    } else /*if (color_proto.has_custom_color())*/ {
        result = color_proto.custom_color();
    }
    
    return result;
}

renderer::RenderSettings RenderSettingsDeserialization(const render_serialize::RenderSettings& render_settings_proto) {
    
    renderer::RenderSettings result;
    
    result.width_ = render_settings_proto.width();
    result.height_ = render_settings_proto.height();
    result.padding_ = render_settings_proto.padding();
    result.stop_radius_ = render_settings_proto.stop_radius();
    result.line_width_ = render_settings_proto.line_width();
    result.bus_label_font_size_ = render_settings_proto.bus_label_font_size();
    
    result.bus_label_offset_.first = render_settings_proto.bus_label_offset().first();
    result.bus_label_offset_.second = render_settings_proto.bus_label_offset().second();
    
    result.stop_label_font_size_ = render_settings_proto.stop_label_font_size();
    
    result.stop_label_offset_.first = render_settings_proto.stop_label_offset().first();
    result.stop_label_offset_.second = render_settings_proto.stop_label_offset().second();
    
    result.underlayer_color_ = std::move(ColorDeserialization(render_settings_proto.underlayer_color()));
    
    result.underlayer_width_ = render_settings_proto.underlayer_width();
    
    for (const auto& color_proto : render_settings_proto.color_palette()) {
        result.color_palette_.push_back(std::move(ColorDeserialization(color_proto)));
    }
    
    return result;
}

router::RoutingSettings RoutingSettingsDeserialization(const transport_router_serialize::RoutingSettings& routing_settings_proto) {
    router::RoutingSettings result;
    
    result.bus_wait_time = routing_settings_proto.bus_wait_time();
    result.bus_velocity = routing_settings_proto.bus_velocity();
    
    return result;
}

void Deserialization(TransportCatalogue& transport_catalogue, 
                     renderer::RenderSettings& render_settings,
                     router::RoutingSettings& routing_settings,
                     std::istream& input) {
    
    transport_catalogue_serialize::AggregatedData aggregated_data_proto;
    
    aggregated_data_proto.ParseFromIstream(&input);
    
    transport_catalogue = TransportCatalogueDeserialization(aggregated_data_proto.transport_catalogue());
    render_settings = RenderSettingsDeserialization(aggregated_data_proto.render_settings());
    routing_settings = RoutingSettingsDeserialization(aggregated_data_proto.routing_settings());
}





} //end namespace serialization
} //end namespace transport_catalogue