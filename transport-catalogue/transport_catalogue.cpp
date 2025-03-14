// напишите решение с нуля
// код сохраните в свой git-репозиторий

#include "transport_catalogue.h"

#include <iostream>

namespace Catalogue {
    void TransportCatalogue::AddStop(const struct TStop &stop) {
        stops_.push_back(std::move(stop));
        stopname_to_stop_[stops_.back().name] = &stops_.back();
    }


    const TStop *TransportCatalogue::FindStop(const std::string_view name) const {
        if( stopname_to_stop_.count(name) ) {
            return stopname_to_stop_.at(name);
        }

        return nullptr;
    }

    void TransportCatalogue::AddBus(const struct TBus &bus) {
        buses_.push_back(std::move(bus));
        busname_to_bus_[buses_.back().name] = &buses_.back();
    }

    const TBus *TransportCatalogue::GetBusInfo(std::string_view number_bus) const {
        if( busname_to_bus_.count(number_bus) ) {
            return busname_to_bus_.at(number_bus);
        }

        return nullptr;
    }

    void TransportCatalogue::LinkStopWithBuses(std::string_view stop, std::string_view bus) {

        links_[stop].insert(std::string(bus));
    }

    const std::set<std::string> TransportCatalogue::GetBusesOnStop(std::string_view stop) {
        if( links_.count(stop) ) {
            return links_.at(stop);
        }

        return {};
    }

    void TransportCatalogue::AddDistance(const TStop *stop1, const TStop *stop2, double distance) {
        distance_between_stops_[std::make_pair(stop1, stop2)] = distance;
    }

    double TransportCatalogue::GetDistance(const TStop *stop1, const TStop *stop2) const {
        auto pair_find = std::make_pair(stop1, stop2);
        if( distance_between_stops_.count(pair_find) ) {
            return distance_between_stops_.at(pair_find);
        } else {
            auto pair_find_reverse = std::make_pair(stop2, stop1);

            if( distance_between_stops_.count(pair_find_reverse) ) {
                return distance_between_stops_.at(pair_find_reverse);
            }
        }

        return 0;
    }

    const std::unique_ptr<const std::deque<struct TBus>> TransportCatalogue::GetAllBus() const {
        return std::make_unique<const std::deque<struct TBus>>(buses_);
    }

    const std::unique_ptr<const std::deque<struct TStop>> TransportCatalogue::GetAllStop() const {
        return std::make_unique<const std::deque<struct TStop>>(stops_);
    }
}