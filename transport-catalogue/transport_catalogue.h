#pragma once
// напишите решение с нуля
// код сохраните в свой git-репозиторий


#include <string>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>

#include "geo.h"



namespace Catalogue {
    struct TStop {
        std::string name;
        Geo::Coordinates xy;
    };

    struct TBus {
        std::string name;
        std::vector<const struct TStop*> way;
        std::unordered_set<std::string_view> unique_stop;
        bool circle;
    };

    struct PairHash {

        size_t operator() (const std::pair<const TStop*,const TStop* > &pair) const {
            size_t h1 = v_hasher_(pair.first);
            size_t h2 = v_hasher_(pair.second);
            return h1 ^ (h2 << 1);

        }
        private:
            std::hash<const void *> v_hasher_;
    };

    class TransportCatalogue {

    public:
        TransportCatalogue() {}
        ~TransportCatalogue() {}

        void AddStop(const struct TStop &stop);
        const TStop *FindStop(const std::string_view name);
        void AddBus(const struct TBus &bus);
        const TBus *GetBusInfo(std::string_view number_bus);

        void LinkStopWithBuses(std::string_view stop, std::string_view bus);
        void AddDistance(const TStop *stop1, const TStop *stop2, double distance);
        const double GetDistance(const TStop *stop1, const TStop *stop2);
        const std::set<std::string> GetBusesOnStop(std::string_view stop);
    private:

        std::unordered_map<std::string_view, struct TStop *> stopname_to_stop_;
        std::unordered_map<std::string_view, struct TBus *> busname_to_bus_;
        std::unordered_map<std::string_view, std::set<std::string>> links_;
        std::unordered_map<std::pair<const TStop*, const TStop*>, double, PairHash> distance_between_stops_;

        std::deque<struct TBus> buses_;
        std::deque<struct TStop> stops_;
    };
}