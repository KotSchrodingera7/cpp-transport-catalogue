#include "svg.h"
#include <unordered_map>
#include <algorithm>

#define _USE_MATH_DEFINES 
#include <cmath>


namespace svg {

const std::unordered_map<char, std::string> symbol_replace_ {
    {'"', "&quot;"},
    {'\'', "&apos;"},
    {'<', "&lt;"},
    {'>', "&gt;"},
    {'&', "&amp;"},
};

std::string StrokeLineCapToString(StrokeLineCap linecap) {
    switch (linecap) {
        case StrokeLineCap::BUTT:
            return "butt"s;
        case StrokeLineCap::ROUND:
            return "round"s;
        case StrokeLineCap::SQUARE:
            return "square"s;
        default:
            throw std::invalid_argument("Invalid line cap"s);
    }
}

std::string StrokeLineJoinToString(StrokeLineJoin linejoin) {
     switch (linejoin) {
        case StrokeLineJoin::ARCS:
            return "arcs"s;
        case StrokeLineJoin::BEVEL:
            return "bevel"s;
        case StrokeLineJoin::MITER:
            return "miter"s;
        case StrokeLineJoin::MITER_CLIP:
            return "miter-clip"s;
        case StrokeLineJoin::ROUND:
            return "round"s;
        default:
            throw std::invalid_argument("Invalid line join"s);
    }
}

std::ostream& operator<<(std::ostream &out, StrokeLineCap linecap) {
    out << StrokeLineCapToString(linecap);
    return out;
}

std::ostream& operator<<(std::ostream &out, StrokeLineJoin linejoin) {
    out << StrokeLineJoinToString(linejoin);
    return out;
}

std::ostream& operator<<(std::ostream &out, Color color) {
    visit(OstreamPrinterColor{out}, color);
    return out;
}

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
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    bool start_ = false;
    for( const auto &point: points_ ) {
        if( start_ ) {
            out << " ";
        } else {
            start_ = true;
        }
        out << point.x << "," << point.y;
    }
    
    out << "\"";
    RenderAttrs(context.out);
    out << "/>"sv;
}



//  <text x="35" y="20" dx="0" dy="6" font-size="12" font-family="Verdana" font-weight="bold">Hello C++</text>
// ---------- Text ------------------
Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
Text& Text::SetOffset(Point offset) {
    offset_= offset;
    return *this;
}

    // Задаёт размеры шрифта (атрибут font-size)
Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}

    // Задаёт название шрифта (атрибут font-family)
Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

    // Задаёт толщину шрифта (атрибут font-weight)
Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
Text& Text::SetData(std::string data) {
    for(const char &c : data) {
        if( symbol_replace_.count(c) ) {
            data_ += symbol_replace_.at(c);
            continue;
        }
        data_ += c;
    }
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text";
    RenderAttrs(context.out);
    out << " x=\"" << pos_.x << "\" y=\"" << pos_.y << "\" dx=\"" << offset_.x << "\" dy=\"" << offset_.y << "\" font-size=\"" << size_ <<"\"";

    if( font_family_ ) {
        out << " font-family=\"" << *font_family_ <<"\"";
    }

    if( font_weight_ ) {
        out << " font-weight=\"" << *font_weight_ <<"\"";
    }
    out << ">";
    out << data_ << "</text>";
}

// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(std::move(obj));
}   

void Document::Render(std::ostream& out) const {
   out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
   out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    for(const auto &object: objects_) {
        object->Render(RenderContext(out));
    }
    out << "</svg>"sv;
}

}  // namespace svg


namespace shapes {
svg::Polyline CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays) {
    using namespace svg;
    Polyline polyline;
    for (int i = 0; i <= num_rays; ++i) {
        double angle = 2 * M_PI * (i % num_rays) / num_rays;
        polyline.AddPoint({center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle)});
        if (i == num_rays) {
            break;
        }
        angle += M_PI / num_rays;
        polyline.AddPoint({center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle)});
    }
    return polyline;
}
}