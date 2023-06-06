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



















} //end namespace renderer
} //end namespace transport_catalogue