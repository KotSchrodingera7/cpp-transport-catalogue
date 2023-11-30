#include "map_renderer.h"
#include "domain.h"

#include <set>
/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

namespace renderer {

void MapRenderer::RenderPolyline(const std::map<std::string, std::vector<geo::Coordinates>> &xys, 
                                svg::Document &doc, 
                                const SphereProjector &proj) const {
    
    int count_max = render_setting_.color_palete.size();
    int i = 0;
    for( auto &[name, geo_coord] : xys) {
        svg::Polyline polyline_;
        for (auto &geo_point: geo_coord) {
            const svg::Point screen_coord = proj(geo_point);
            polyline_.AddPoint({screen_coord.x, screen_coord.y});
        }

        if( !db_.GetBusInfo(name)->circle) {
            for (auto it_ = geo_coord.rbegin() + 1; it_ != geo_coord.rend(); ++it_) {
                const svg::Point screen_coord = proj(*it_);
                polyline_.AddPoint({screen_coord.x, screen_coord.y});
            }
        }

        if( geo_coord.size() > 0 ) {
            polyline_.SetStrokeColor(render_setting_.color_palete[i++]);
            polyline_.SetFillColor("none")
            .SetStrokeWidth(render_setting_.line_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            if( i >= count_max ) {
                i = 0;
            }
            doc.Add(polyline_);
        }
        
    }
}

void MapRenderer::RenderTextBus(const std::map<std::string, std::vector<geo::Coordinates>> &xys, 
                                svg::Document &doc, 
                                const SphereProjector &proj) const {
    int count_max = render_setting_.color_palete.size();
    int i = 0;
    for( auto &[name, geo_coord] : xys) {
        const auto bus_info_ = db_.GetBusInfo(name);
        for( const auto stop_name : bus_info_->endpoint ) {
            const svg::Point screen_coord = proj(db_.FindStop(stop_name)->xy);
            svg::Text text_;
            svg::Text text_new;
            text_.SetPosition({screen_coord.x, screen_coord.y})
                .SetData(std::string(name))
                .SetFillColor(render_setting_.color_palete[i])
                .SetOffset({render_setting_.bus_label_offset.x, render_setting_.bus_label_offset.y})
                .SetFontSize(render_setting_.bus_label_font_size)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold");

            text_new = text_;
            text_new.SetFillColor(render_setting_.underlayer_color)
                    .SetStrokeColor(render_setting_.underlayer_color)
                    .SetStrokeWidth(render_setting_.underlayer_width)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                    .SetData("");

            doc.Add(text_new);
            doc.Add(text_);
        } 
        if( ++i >= count_max ) {
            i = 0;
        }

    }
}

void MapRenderer::RenderTextStop(svg::Document &doc, 
                                const SphereProjector &proj) const {
    std::set<std::string_view> names_stops_;
    auto ptr = db_.GetAllBus();

    for( const auto &bus : (*ptr) ) {
        names_stops_.insert(bus.unique_stop.begin(), bus.unique_stop.end());
    }

    for( const auto &name_stop: names_stops_ ) {
        svg::Circle circle;
        const svg::Point screen_coord = proj(db_.FindStop(name_stop)->xy);
        circle.SetCenter({screen_coord.x, screen_coord.y})
                .SetRadius(render_setting_.stop_radius)
                .SetFillColor("white");
        doc.Add(circle);

    }

    for( const auto &name_stop: names_stops_ ) {
        const svg::Point screen_coord = proj(db_.FindStop(name_stop)->xy);
        svg::Text text_;
        svg::Text text_new;
        text_.SetPosition({screen_coord.x, screen_coord.y})
                .SetData(std::string(name_stop))
                .SetFillColor("black")
                .SetOffset({render_setting_.stop_label_offset.x, render_setting_.stop_label_offset.y})
                .SetFontSize(render_setting_.stop_label_font_size)
                .SetFontFamily("Verdana");

        text_new = text_;
        text_new.SetFillColor(render_setting_.underlayer_color)
                .SetStrokeColor(render_setting_.underlayer_color)
                .SetStrokeWidth(render_setting_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetData("");

        doc.Add(text_new);
        doc.Add(text_);

    }
}


svg::Document MapRenderer::RenderMap(const std::map<std::string, std::vector<geo::Coordinates>> &xys) const {
    // auto xys = GetPointsWay();
    svg::Document doc_;

    std::vector<geo::Coordinates> geo_coords;
    for( auto &[name, geo_coord] : xys) {
        geo_coords.insert(geo_coords.end(), geo_coord.begin(), geo_coord.end());
    }

    const SphereProjector proj{
        geo_coords.begin(), geo_coords.end(), render_setting_.width, render_setting_.height, render_setting_.padding
    };

    RenderPolyline(xys, doc_, proj);
    RenderTextBus(xys, doc_, proj);
    RenderTextStop(doc_, proj);
    return doc_;
}
}