#pragma once

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

#include "geo.h"
#include "svg.h"
#include "domain.h"
#include "transport_catalogue.h"


#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>

namespace transport_catalogue {
namespace renderer {

inline const double EPSILON = 1e-6;
bool IsZero(double value);

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding);
   

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

struct RenderSettings {
    double width_;
    double height_;
    double padding_;
    double stop_radius_;
    double line_width_;
    int bus_label_font_size_;
    std::pair<double, double> bus_label_offset_;
    int stop_label_font_size_;
    std::pair<double, double>  stop_label_offset_;
    svg::Color underlayer_color_;
    double underlayer_width_;
    std::vector<svg::Color> color_palette_;
};

class MapRenderer {
public:
    MapRenderer(RenderSettings& render_settings);
    
    void SetPolylineProperties(svg::Polyline& polyline, int number) const;
    
    void SetRouteNameParameters(svg::Text& text, const std::string& name, svg::Point point) const;
    void SetRouteNameBackingParameters(svg::Text& text, const std::string& name, svg::Point point) const;
    void SetRouteNameInscriptionParameters(svg::Text& text, const std::string& name, svg::Point point, int number) const;
    
    void SetStopsCirclesParameters(svg::Circle& circle, svg::Point point) const;
    
    void SetStopNameParameters(svg::Text& text, const std::string& name, svg::Point point) const;
    void SetStopNameBackingParameters(svg::Text& text, const std::string& name, svg::Point point) const;
    void SetStopNameInscriptionParameters(svg::Text& text, const std::string& name, svg::Point point) const;
    
    int GetColorPaletteSize() const;
    SphereProjector GetSphereProjector(const std::vector<geo::Coordinates>& Items) const;
    
    std::vector<geo::Coordinates> GetStopsCoordinates(TransportCatalogue& db) const;
    
    void RenderMapLine(std::vector<std::pair<Bus*, int>>& buses_palette, 
                       svg::Document& document, 
                       SphereProjector& sphere_projector) const;
    void RenderMapRoutesName(std::vector<std::pair<Bus*, int>>& buses_palette, 
                             svg::Document& document, 
                             SphereProjector& sphere_projector) const;
    
    void RenderMapStops(svg::Document& document, 
                        SphereProjector& sphere_projector, 
                        TransportCatalogue& db) const;
    void RenderMapStopsName(svg::Document& document, 
                            SphereProjector& sphere_projector, 
                            TransportCatalogue& db) const;
    
    void RenderMap(std::ostream& out, TransportCatalogue& db) const;
    
private:
    RenderSettings render_settings_;
    int color_palette_size_;
};

//---------------- implementation -------------------

template <typename PointInputIt>
SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                double max_width, double max_height, double padding)
    : padding_(padding) //
{
    // Если точки поверхности сферы не заданы, вычислять нечего
    if (points_begin == points_end) {
        return;
    }
    
    // Находим точки с минимальной и максимальной долготой
    const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
    min_lon_ = left_it->lng;
    const double max_lon = right_it->lng;
    
    // Находим точки с минимальной и максимальной широтой
    const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
    const double min_lat = bottom_it->lat;
    max_lat_ = top_it->lat;
    
    // Вычисляем коэффициент масштабирования вдоль координаты x
    std::optional<double> width_zoom;
    if (!IsZero(max_lon - min_lon_)) {
        width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
    }
    
    // Вычисляем коэффициент масштабирования вдоль координаты y
    std::optional<double> height_zoom;
    if (!IsZero(max_lat_ - min_lat)) {
        height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
    }
    
    if (width_zoom && height_zoom) {
        // Коэффициенты масштабирования по ширине и высоте ненулевые,
        // берём минимальный из них
        zoom_coeff_ = std::min(*width_zoom, *height_zoom);
    } else if (width_zoom) {
        // Коэффициент масштабирования по ширине ненулевой, используем его
        zoom_coeff_ = *width_zoom;
    } else if (height_zoom) {
        // Коэффициент масштабирования по высоте ненулевой, используем его
        zoom_coeff_ = *height_zoom;
    }
}

} //end namespace renderer
} //end namespace transport_catalogue