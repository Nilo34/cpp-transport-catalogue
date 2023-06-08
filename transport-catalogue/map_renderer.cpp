#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */


namespace transport_catalogue {
namespace renderer {

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

// Проецирует широту и долготу в координаты внутри SVG-изображения
svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}

MapRenderer::MapRenderer(RenderSettings& render_settings) 
: render_settings_(render_settings)
, color_palette_size_(render_settings_.color_palette_.size())
{
}


void MapRenderer::SetPolylineProperties (svg::Polyline& polyline, int number) const {
    polyline.SetFillColor(svg::NoneColor);
    polyline.SetStrokeColor(render_settings_.color_palette_[number]);
    polyline.SetStrokeWidth(render_settings_.line_width_);
    polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
}


void MapRenderer::SetRouteNameParameters(svg::Text& text, const std::string& name, svg::Point point) const {
    text.SetPosition(point);
    text.SetOffset({render_settings_.bus_label_offset_.first,
                     render_settings_.bus_label_offset_.second});
    text.SetFontSize(render_settings_.bus_label_font_size_);
    text.SetFontFamily("Verdana");
    text.SetFontWeight("bold");
    text.SetData(name);
    
    
}
void MapRenderer::SetRouteNameBackingParameters(svg::Text& text, const std::string& name, svg::Point point) const {
    SetRouteNameParameters(text, name, point);
    
    text.SetFillColor(render_settings_.underlayer_color_);
    text.SetStrokeColor(render_settings_.underlayer_color_);
    text.SetStrokeWidth(render_settings_.underlayer_width_);
    text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
}
void MapRenderer::SetRouteNameInscriptionParameters(svg::Text& text, const std::string& name, svg::Point point, int number) const{
    SetRouteNameParameters(text, name, point);
    
    text.SetFillColor(render_settings_.color_palette_[number]);
}


void MapRenderer::SetStopsCirclesParameters(svg::Circle& circle, svg::Point point) const {
    circle.SetCenter(point);
    circle.SetRadius(render_settings_.stop_radius_);
    circle.SetFillColor("white");
}


void MapRenderer::SetStopNameParameters(svg::Text& text, const std::string& name, svg::Point point) const {
    text.SetPosition(point);
    text.SetOffset({render_settings_.stop_label_offset_.first,
                     render_settings_.stop_label_offset_.second});
    text.SetFontSize(render_settings_.stop_label_font_size_);
    text.SetFontFamily("Verdana");
    //text.SetFontWeight("bold");
    text.SetData(name);
    
    
}
void MapRenderer::SetStopNameBackingParameters(svg::Text& text, const std::string& name, svg::Point point) const {
    SetStopNameParameters(text, name, point);
    
    text.SetFillColor(render_settings_.underlayer_color_);
    text.SetStrokeColor(render_settings_.underlayer_color_);
    text.SetStrokeWidth(render_settings_.underlayer_width_);
    text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
}
void MapRenderer::SetStopNameInscriptionParameters(svg::Text& text, const std::string& name, svg::Point point) const{
    SetStopNameParameters(text, name, point);
    
    text.SetFillColor("black");
}


int MapRenderer::GetColorPaletteSize() const {
    return color_palette_size_;
}

SphereProjector MapRenderer::GetSphereProjector(const std::vector<geo::Coordinates>& items) const {
    return SphereProjector(items.begin(),
                           items.end(),
                           render_settings_.width_,
                           render_settings_.height_,
                           render_settings_.padding_);
}


std::vector<geo::Coordinates> MapRenderer::GetStopsCoordinates(TransportCatalogue& db) const {
    std::vector<geo::Coordinates> result; 
    std::unordered_map<std::string_view, transport_catalogue::Bus*> buses = db.GetMapToBus();
    
    for (auto& [name, bus] : buses) {
        for (auto& stop : bus->stops) {
            result.push_back(stop->coord);
        }
    }
    return result;
}

//---------------- render -------------------

void MapRenderer::RenderMapLine(std::vector<std::pair<Bus*, int>>& buses_palette, 
                                svg::Document& document, 
                                SphereProjector& sphere_projector) const {
    
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
            SetPolylineProperties(bus_line, palette);
            document.Add(bus_line);
        }

        stops_coordinates.clear();
    }
}

void MapRenderer::RenderMapRoutesName(std::vector<std::pair<Bus*, int>>& buses_palette, 
                                      svg::Document& document, 
                                      SphereProjector& sphere_projector) const {
    
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
                SetRouteNameBackingParameters(route_name_backing_circular,
                                              std::string(bus->name),
                                              sphere_projector(stops_coordinates[0]));
                document.Add(route_name_backing_circular);
                
                SetRouteNameInscriptionParameters(route_name_inscription_circular,
                                                  std::string(bus->name),
                                                  sphere_projector(stops_coordinates[0]),
                                                  palette);
                document.Add(route_name_inscription_circular);
                
            } else {
                
                SetRouteNameBackingParameters(route_name_backing_circular, 
                                              std::string(bus->name),
                                              sphere_projector(stops_coordinates[0]));
                document.Add(route_name_backing_circular);
                
                SetRouteNameInscriptionParameters(route_name_inscription_circular,
                                                  std::string(bus->name),
                                                  sphere_projector(stops_coordinates[0]),
                                                  palette);
                document.Add(route_name_inscription_circular);
                
                if (stops_coordinates[0] != stops_coordinates[stops_coordinates.size()/2]) {
                    
                    SetRouteNameBackingParameters(route_name_backing,
                                                  std::string(bus->name),
                                                  sphere_projector(stops_coordinates[stops_coordinates.size()/2]));
                    document.Add( route_name_backing);
                    
                    SetRouteNameInscriptionParameters(route_name_inscription,
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

void MapRenderer::RenderMapStops(svg::Document& document, 
                                 SphereProjector& sphere_projector, 
                                 TransportCatalogue& db) const {
    
    std::vector<geo::Coordinates> stops_coordinates;    
    
    auto stops = db.GetMapToStop();    
    std::vector<std::string_view> stops_names;
    
    for (auto& [name, stop] : stops) {
        
        if (stop->buses.size() > 0) {
            stops_names.push_back(name);
        }
    }
    
    std::sort(stops_names.begin(), stops_names.end());
    
    svg::Circle circle;
    Stop* buffer_stop;
    
    for(std::string_view stop_name : stops_names){
        buffer_stop = db.GetStop(stop_name);
        
        if (buffer_stop) {
            SetStopsCirclesParameters(circle, sphere_projector(buffer_stop->coord));
            document.Add(circle);
        }
    }
    
}

void MapRenderer::RenderMapStopsName(svg::Document& document, 
                                     SphereProjector& sphere_projector, 
                                     TransportCatalogue& db) const {
    std::vector<geo::Coordinates> stops_coordinates;
    
    auto stops = db.GetMapToStop();    
    std::vector<std::string_view> stops_names;
    
    for (auto& [name, stop] : stops) {
        
        if (stop->buses.size() > 0) {
            stops_names.push_back(name);
        }
    }
    
    std::sort(stops_names.begin(), stops_names.end());
    
    svg::Text stop_name_backing;
    svg::Text stop_name_inscription;
    Stop* buffer_stop;
    
    for (std::string_view stop_name : stops_names) {
        buffer_stop = db.GetStop(stop_name);
        
        if (buffer_stop) {        
            
            SetStopNameBackingParameters(stop_name_backing, 
                                         buffer_stop->name, 
                                         sphere_projector(buffer_stop->coord));
            document.Add(stop_name_backing);
            
            SetStopNameInscriptionParameters(stop_name_inscription, 
                                             buffer_stop->name, 
                                             sphere_projector(buffer_stop->coord));
            document.Add(stop_name_inscription);
        }
    }
    
}


void MapRenderer::RenderMap(std::ostream& out, TransportCatalogue& db) const {
    svg::Document document;
    SphereProjector sphere_projector = GetSphereProjector(GetStopsCoordinates(db));
    std::vector<std::string_view> buses_name; 
    std::vector<std::pair<Bus*, int>> buses_palette;   
    
    int palette_size = 0;
    int palette_index = 0;
    
    palette_size = GetColorPaletteSize();
    
    std::unordered_map<std::string_view, Bus*> buses = db.GetMapToBus();
    if (buses.size() > 0) {
        
        for (auto& [name, bus] : buses) {
            buses_name.push_back(name);
        }   
 
        std::sort(buses_name.begin(), buses_name.end());
 
        for (std::string_view bus_name : buses_name) {
            Bus* bus_info = db.GetBus(bus_name);
 
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
    
    RenderMapStops(document, sphere_projector, db);
    RenderMapStopsName(document, sphere_projector, db);
    
    document.Render(out);
}

} //end namespace renderer
} //end namespace transport_catalogue