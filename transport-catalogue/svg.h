#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <cassert>
#include <variant>
#include <sstream>
namespace svg {

struct Rgb {
    Rgb(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {}
    Rgb() {}
    uint8_t red{0};
    uint8_t green{0};
    uint8_t blue{0};
};

struct Rgba : public Rgb {
    Rgba(uint8_t r, uint8_t g, uint8_t b, double o) : Rgb(r, g, b), opacity(o) {}
    Rgba() : Rgb() {}
    double opacity{1.0};
};

using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;
using namespace std::literals;

inline const Color NoneColor{"none"};

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};

std::string StrokeLineCapToString(StrokeLineCap linecap);
std::string StrokeLineJoinToString(StrokeLineJoin linejoin);

std::ostream& operator<<(std::ostream &out, StrokeLineCap linecap);
std::ostream& operator<<(std::ostream &out, StrokeLineJoin linejoin);
std::ostream& operator<<(std::ostream &out, Color color);

struct OstreamPrinterColor {
    std::ostream& out;

    void operator()(std::monostate) const {
        out << "none"sv;
    }

    void operator()(Rgb rgb) const {
        out << "rgb(" << (int)rgb.red << "," << (int)rgb.green << "," << (int)rgb.blue << ")";
    }

    void operator()(Rgba rgba) const {
        out << "rgba(" << (int)rgba.red << "," << (int)rgba.green << "," << (int)rgba.blue << "," << rgba.opacity << ")";
    }
    void operator()(std::string str) const {
        out << str;
    }
};


/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};


template <typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }

    Owner& SetStrokeLineCap(StrokeLineCap linecap) {
        linecap_ = std::move(linecap);
        return AsOwner();
    }
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        linejoin_ = std::move(line_join);
        return AsOwner();
    }

    Owner& SetStrokeWidth(double width) {
        stroke_width_ = std::move(width);
        return AsOwner();
    }
protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;
        
        if (fill_color_) {
            out << " fill=\""sv << *fill_color_ << "\""sv;
        }
        if (stroke_color_) {
            out << " stroke=\""sv << *stroke_color_ << "\""sv;
        }

        if( stroke_width_ ) {
            out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
        }

        if( linecap_ ) {
            out << " stroke-linecap=\""sv << *linecap_ << "\""sv;
        }

        if( linejoin_ ) {
            out << " stroke-linejoin=\""sv << *linejoin_ << "\""sv;
        }   
    }

private:
    Owner& AsOwner() {
        // static_cast безопасно преобразует *this к Owner&,
        // если класс Owner — наследник PathProps
        return static_cast<Owner&>(*this);
    }

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<StrokeLineCap> linecap_;
    std::optional<StrokeLineJoin> linejoin_;
    std::optional<double> stroke_width_;
};


/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
public:
    
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_{0,0};
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline> {
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

    /*
     * Прочие методы и данные, необходимые для реализации элемента <polyline>
     */
private:
    void RenderObject(const RenderContext& context) const override;

    std::vector<Point> points_;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text> {
public:
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);
private:
    void RenderObject(const RenderContext& context) const override;

    Point pos_{0,0};
    Point offset_{0,0};
    uint32_t size_{1};
    std::optional<std::string> font_family_;
    std::optional<std::string> font_weight_;
    std::string data_{""};
};


class ObjectContainer {
public:
    template <typename Obj>
    void Add(Obj obj) {
        AddPtr(std::make_unique<Obj>(std::move(obj)));
    } 
    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
};

class Drawable {
public:
    virtual void Draw(ObjectContainer& object) const = 0;
    virtual ~Drawable() = default;
}; 

class Document : public ObjectContainer {
public:
    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;

private:
    std::vector<std::unique_ptr<Object>> objects_;
};

}  // namespace svg


namespace shapes {

class Triangle final : public svg::Drawable {
public:
    Triangle(svg::Point p1, svg::Point p2, svg::Point p3)
        : p1_(p1)
        , p2_(p2)
        , p3_(p3) {
    }

    // Реализует метод Draw интерфейса svg::Drawable
    void Draw(svg::ObjectContainer& container) const override {
        container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
    }

private:
    svg::Point p1_, p2_, p3_;
};

svg::Polyline CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays);

class Star final : public svg::Drawable { 
public:
    Star(svg::Point center, double outer_rad, double inner_rad, int num_rays)
        : center_(center)
        , outer_rad_(outer_rad)
        , inner_rad_(inner_rad)
        , num_rays_(num_rays) {

        }

    void Draw(svg::ObjectContainer& container) const override {
        container.Add(CreateStar(center_, outer_rad_, inner_rad_, num_rays_)
                    .SetFillColor("red")
                    .SetStrokeColor("black"));
        
    }
private:
    svg::Point center_;
    double outer_rad_;
    double inner_rad_;
    int num_rays_;
};

class Snowman final : public svg::Drawable { 
public:
    Snowman(svg::Point point, double radius) 
    : point_(point)
    , radius_(radius) {

    }
    void Draw(svg::ObjectContainer& container) const override {
        container.Add(svg::Circle()
                        .SetCenter({point_.x, point_.y + 5 * radius_})
                        .SetRadius(radius_ * 2)
                        .SetFillColor("rgb(240,240,240)")
                        .SetStrokeColor("black"));
        container.Add(svg::Circle()
                    .SetCenter({point_.x, point_.y + 2 * radius_})
                    .SetRadius(radius_ * 1.5)
                    .SetFillColor("rgb(240,240,240)")
                    .SetStrokeColor("black"));
        container.Add(svg::Circle()
                    .SetCenter(point_)
                    .SetRadius(radius_)
                    .SetFillColor("rgb(240,240,240)")
                    .SetStrokeColor("black"));
    }
private:
    svg::Point point_;
    double radius_;
};

} // namespace shapes 