#include "svg.h"

#include <algorithm>

namespace svg {

using namespace std::literals;

// ---------- Rgb ------------------

Rgb::Rgb(uint8_t input_red, uint8_t input_green, uint8_t input_blue)
    : red(input_red)
    , green(input_green)
    , blue(input_blue)
{
}

// ---------- Rgba ------------------

Rgba::Rgba(uint8_t input_red, uint8_t input_green, uint8_t input_blue, double input_opacity)
    : red(input_red)
    , green(input_green)
    , blue(input_blue)
    , opacity(input_opacity)
{
}

// ---------- OstreamColorPrinter ------------------

void OstreamColorPrinter::operator()(std::monostate) const {
    using namespace std::literals;
    out << "none"sv;
}
void OstreamColorPrinter::operator()(std::string color) const {
    out << color;
}
void OstreamColorPrinter::operator()(svg::Rgb color) const {
    using namespace std::literals;
    out << "rgb("sv << +color.red << ","sv
                    << +color.green << ","sv
                    << +color.blue << ")";
}
void OstreamColorPrinter::operator()(svg::Rgba color) const {
    using namespace std::literals;
    out << "rgba("sv << +color.red << ","sv
                     << +color.green << ","sv
                     << +color.blue << ","sv
                     << color.opacity << ")"sv;
}








// ---------- Object ------------------

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
    points_.emplace_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    
    out << "<polyline points=\""sv;
    
    bool need_a_space = false;
    for (const Point& point : points_) {
        if (need_a_space) {
            out << " "sv;
        } else {
            need_a_space = true;
        }
        out << point.x << ","sv << point.y;
    }
    
    out << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Text ------------------

// Задаёт координаты опорной точки (атрибуты x и y)
Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

// Задаёт размеры шрифта (атрибут font-size)
Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}

// Задаёт название шрифта (атрибут font-family)
Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = std::move(font_family);
    return *this;
}

// Задаёт толщину шрифта (атрибут font-weight)
Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = std::move(font_weight);
    return *this;
}

// Задаёт текстовое содержимое объекта (отображается внутри тега text)
Text& Text::SetData(std::string data) {
    data_ = std::move(data);
    return *this;
}

// Заменяет особые символы
std::string Text::ReplaceTheSpecialChar(const std::string& str) {
    std::string buffer;
    
    for (const char ch : str) {
        
        if (ch == '"') {
            buffer += "&quot;"sv;
            continue;
        } else if (ch == '<') {
            buffer += "&lt;"sv;
            continue;
        } else if (ch == '>') {
            buffer += "&gt;"sv;
            continue;
        } else if (ch == '\'') {
            buffer += "&apos;"sv;
            continue;
        } else if (ch == '&') {
            buffer += "&amp;"sv;
            continue;
        }
        
        buffer += ch;
        
    }
    
    return buffer;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    
    out << "<text x=\""sv << pos_.x
        << "\" y=\""sv << pos_.y
        << "\" dx=\""sv << offset_.x
        << "\" dy=\""sv << offset_.y
        << "\" font-size=\""sv << size_;
    if (!font_family_.empty()) {
        out << "\" font-family=\""sv << font_family_;
    }
    if (!font_weight_.empty()) {
        out << "\" font-weight=\""sv << font_weight_;
    }
    
    out << "\""sv;
    RenderAttrs(context.out);
    out << ">"sv << ReplaceTheSpecialChar(data_) << "</text>"sv;
    
}

// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    
    RenderContext render_context(out, 0, 4);
    
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;
    
    for (const auto& object : objects_) {
        object->Render(render_context);
    }
    
    out << "</svg>"sv;
}

}  // namespace svg