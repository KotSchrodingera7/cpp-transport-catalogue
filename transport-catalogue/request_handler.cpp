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
    const TBus *addr_bus = db_.GetBusInfo(bus_name);
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

std::map<std::string, std::vector<geo::Coordinates>> RequestHandler::GetPointsWay() const {
    auto ptr = db_.GetAllBus();

    std::map<std::string, std::vector<geo::Coordinates>> result_;
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
    auto xys = GetPointsWay();

    return renderer_.RenderMap(xys);
}
