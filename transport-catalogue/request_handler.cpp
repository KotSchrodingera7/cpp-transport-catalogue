#include "request_handler.h"
#include "domain.h"
/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

#include <iostream>

std::optional<BusStat> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    const Catalogue::TBus *addr_bus = db_.GetBusInfo(bus_name);
    BusStat stat_result_;
    if( addr_bus ) {
        stat_result_.stop_count = addr_bus->way.size();
        stat_result_.unique_stop = addr_bus->unique_stop.size();
        stat_result_.corvature = 0;
        stat_result_.route_length = 0;

        auto vect_stops_ = addr_bus->way;
        for(auto it = vect_stops_.begin(); it != vect_stops_.end(); ++it) {
            if( std::next(it) != vect_stops_.end() ) {
                stat_result_.corvature += geo::ComputeDistance((*it)->xy, (*std::next(it))->xy);
                stat_result_.route_length += db_.GetDistance(*it, *std::next(it));
            }
        }
        if( !addr_bus->circle ) {
            stat_result_.stop_count = stat_result_.stop_count * 2 - 1;
            for(auto it = vect_stops_.rbegin(); it != vect_stops_.rend(); ++it) {
                if( std::next(it) != vect_stops_.rend() ) {
                    stat_result_.corvature += geo::ComputeDistance((*it)->xy, (*std::next(it))->xy);
                    stat_result_.route_length += db_.GetDistance(*it, *std::next(it));
                }
            }

        }
        stat_result_.corvature = stat_result_.route_length / stat_result_.corvature;
        return stat_result_;

    }

    return {};
}

std::map<std::string_view, std::vector<geo::Coordinates>> RequestHandler::GetPointsWay() const {
    auto ptr = db_.GetAllBus();

    std::map<std::string_view, std::vector<geo::Coordinates>> result_;
    for( const auto &bus : (*ptr) ) {
        std::vector<geo::Coordinates> ways_(0);
        for( const auto &way : bus.way ) {
            ways_.push_back(std::move(way->xy));
        }

        result_.insert({bus.name, ways_});
    }
    return result_;
}


svg::Document RequestHandler::RenderMap() const {
    svg::Document doc;
    auto xys_ = GetPointsWay();

    std::vector<geo::Coordinates> geo_coords;
    for( auto &[name, geo_coord] : xys_) {
        geo_coords.insert(geo_coords.end(), geo_coord.begin(), geo_coord.end());
    }
    const SphereProjector proj{
        geo_coords.begin(), geo_coords.end(), renderer_.width, renderer_.height, renderer_.padding
    };
    
    int count_max = renderer_.color_palete.size();
    int i = 0;
    for( auto &[name, geo_coord] : xys_) {
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
            polyline_.SetStrokeColor(renderer_.color_palete[i++]);
            polyline_.SetFillColor("none")
            .SetStrokeWidth(renderer_.line_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            if( i >= count_max ) {
                i = 0;
            }
            doc.Add(polyline_);
        }
        
    }

    i = 0;
    for( auto &[name, geo_coord] : xys_) {
        const auto bus_info_ = db_.GetBusInfo(name);
        for( const auto stop_name : bus_info_->endpoint ) {
            const svg::Point screen_coord = proj(db_.FindStop(stop_name)->xy);
            svg::Text text_;
            svg::Text text_new;
            text_.SetPosition({screen_coord.x, screen_coord.y})
                .SetData(std::string(name))
                .SetFillColor(renderer_.color_palete[i])
                .SetOffset({renderer_.bus_label_offset.x, renderer_.bus_label_offset.y})
                .SetFontSize(renderer_.bus_label_font_size)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold");

            text_new = text_;
            text_new.SetFillColor(renderer_.underlayer_color)
                    .SetStrokeColor(renderer_.underlayer_color)
                    .SetStrokeWidth(renderer_.underlayer_width)
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

    std::set<std::string_view> names_stops_;
    auto ptr = db_.GetAllBus();

    for( const auto &bus : (*ptr) ) {
        names_stops_.insert(bus.unique_stop.begin(), bus.unique_stop.end());
    }

    for( const auto &name_stop: names_stops_ ) {
        svg::Circle circle;
        const svg::Point screen_coord = proj(db_.FindStop(name_stop)->xy);
        circle.SetCenter({screen_coord.x, screen_coord.y})
                .SetRadius(renderer_.stop_radius)
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
                .SetOffset({renderer_.stop_label_offset.x, renderer_.stop_label_offset.y})
                .SetFontSize(renderer_.stop_label_font_size)
                .SetFontFamily("Verdana");

        text_new = text_;
        text_new.SetFillColor(renderer_.underlayer_color)
                .SetStrokeColor(renderer_.underlayer_color)
                .SetStrokeWidth(renderer_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetData("");

        doc.Add(text_new);
        doc.Add(text_);

    }

    return doc;
}
