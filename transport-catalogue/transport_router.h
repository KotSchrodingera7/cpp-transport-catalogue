#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <map>
#include <unordered_map>

namespace Route {

template<typename Weight>
struct InfoWay {
    int count;
    std::string type;
    std::optional<std::string> bus_name;
    std::optional<std::string> stop_name;
    Weight weight;
};

template<typename Weight>
class TransportRouter {
public:

    struct ResultData {
        Weight weight;
        std::vector<InfoWay<Weight>> id_stops;
    };

    TransportRouter(Catalogue::TransportCatalogue &catalog) : catalog_(catalog) {};
    ~TransportRouter() {
        if( router_ ) {
            delete router_;
        }
    }
    void SetSpeed(const int speed) {
        bus_speed_ = speed * 1000.0/60.0;
    }

    void WaitTime(const int time) {
        time_wait_bus_ = time;
    }

    void CreateGraph(void);
    std::optional<ResultData> GetWeigth(const std::string from, const std::string to) const;

private:
template <typename Iterator>
    void AddEdgeForBus(Iterator begin, Iterator end, const std::string &name);

private:

    Catalogue::TransportCatalogue &catalog_;
    graph::DirectedWeightedGraph<Weight> graph_;
    std::unordered_map<std::string, graph::VertexId> stop_name_to_id_;
    graph::Router<Weight> *router_;
    std::unordered_map<graph::EdgeId, InfoWay<Weight>> dict_info_ways_;
    
    double bus_speed_;
    int time_wait_bus_;
};

template<typename Weight>
void TransportRouter<Weight>::CreateGraph() {
    const auto stops = catalog_.GetAllStop();
    const auto buses = catalog_.GetAllBus();
    int size_graph = (*stops).size() * 2;

    int count_ways = 0;
    for( const auto &bus: (*buses) ) {
        if( bus.circle ) {
            count_ways += bus.way.size();
        } else {
            count_ways += bus.way.size() * 2;
        }
    }
    
    // size_graph += count_ways * (*buses).size();
    // std::cout << size_graph << " " << count_ways << " " << (*stops).size() << std::endl;
    graph_ = std::move(graph::DirectedWeightedGraph<double>(size_graph));
    graph::VertexId id = 0;
    for( const auto &stop : (*stops) ) {
        auto id_graph = graph_.AddEdge(graph::Edge<double>{id, id + 1, static_cast<double>(time_wait_bus_)});
        stop_name_to_id_[stop.name] = id;
        dict_info_ways_[id_graph] = {0, "wait", {}, stop.name, static_cast<double>(time_wait_bus_)};
        id += 2;
    }

    for( const auto &bus: (*buses) ) {
        AddEdgeForBus(bus.way.begin(), bus.way.end(), bus.name);

        if( bus.circle != true) {
            AddEdgeForBus(bus.way.rbegin(), bus.way.rend(), bus.name);
        }

    }
    router_ = new graph::Router<double>(graph_);
}


template<typename Weight> 
template <typename Iterator>
void TransportRouter<Weight>::AddEdgeForBus(Iterator begin, Iterator end, const std::string &name) {
    for(auto it = begin; it != end; ++it) {
        double prev_dist = catalog_.GetDistance(*it, *std::next(it));
        int count_bus = 1;
        for( auto it_next = std::next(it); it_next != end; ++it_next) {
            graph::VertexId from_id = stop_name_to_id_[(*it)->name] + 1;
            graph::VertexId to_id = stop_name_to_id_[(*it_next)->name];
            Weight weight_local_ = prev_dist/bus_speed_;
            auto id_graph = graph_.AddEdge(graph::Edge<double>{
                from_id, 
                to_id, 
                weight_local_});
            dict_info_ways_[id_graph] = {count_bus++, "bus", name, {}, weight_local_};
            // std::cout << "Added stop id from " << stop_name_to_id_[(*it)->name] + 1 << " id to " << stop_name_to_id_[(*it_next)->name]  << " weight " << prev_dist/bus_speed_ << std::endl;
            if( std::next(it_next) != end) {
                prev_dist += catalog_.GetDistance(*it_next, *std::next(it_next));
            }
        }
    }    
}

template<typename Weight>
std::optional<typename TransportRouter<Weight>::ResultData> TransportRouter<Weight>::GetWeigth(const std::string from, const std::string to) const {
    if( router_ ) {
        graph::VertexId from_ = stop_name_to_id_.at(from);
        graph::VertexId to_ = stop_name_to_id_.at(to);
        auto info = router_->BuildRoute(from_, to_);

        std::vector<InfoWay<Weight>> result;
        if( info ) {
            for(const auto id : info.value().edges) {
                result.push_back(dict_info_ways_.at(id));
            }
            return ResultData{(*info).weight, std::move(result)};
        }
    }

    return {};
}
}