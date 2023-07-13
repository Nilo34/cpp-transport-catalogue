#pragma once

#include "router.h"
#include "domain.h"
#include "transport_catalogue.h"

#include <unordered_map>
#include <memory>
#include <variant>
#include <deque>
#include <optional>

namespace transport_catalogue {
namespace router {

static const int METERS_IN_A_KILOMETER = 1000;
static const int MINUTES_PER_HOUR = 60;

class TransportRouter {
public:
    TransportRouter() = delete;
    TransportRouter(TransportCatalogue& db, RoutingSettings& routing_settings);
    
    void SetStops(const std::deque<Stop*>);
    
    void SetGraph();
    
    void AddEdgeToStop();
    void AddEdgeToBus();
    
    const graph::Edge<double> СreateEdgeToBus(Stop* start_stop, Stop* end_stop, const double distance);
    
    std::optional<RouteData> GetRouteInformation(graph::VertexId from, graph::VertexId to);
    BusWaitingPeriod GetBusWaitingPeriod(Stop* stop);
    
    template <typename Iterator>
    void FillBusToEdge(Iterator first, Iterator last, Bus* bus);
    
private:
    // параметры
    TransportCatalogue& db_;
    
    RoutingSettings routing_settings_;
    
    std::unique_ptr<graph::DirectedWeightedGraph<double>> graph_;
    std::unique_ptr<graph::Router<double>> router_;
    
    std::unordered_map<graph::EdgeId, std::variant<StopEdge, BusEdge>> map_id_to_edge_;
    std::unordered_map<Stop*, BusWaitingPeriod> map_stop_to_bus_period_;
    
    // методы
    std::deque<Stop*> GetStopsPtr();
    std::deque<Bus*> GetBusesPtr();
};

template <typename Iterator>
void TransportRouter::FillBusToEdge(Iterator first, Iterator last, Bus* bus) {
    
    int distance = 0;
    int number_of_stops = 0;
    
    for (; first != last; ++first) {
        
        for (auto it = std::next(first); it != last; ++it) {
            distance += db_.GetDistanceBetweenStops(*prev(it), *it);
            ++number_of_stops;
            
            graph::EdgeId edge_id = graph_->AddEdge(СreateEdgeToBus(*first, *it, distance));
            map_id_to_edge_[edge_id] = BusEdge{bus->name, graph_->GetEdge(edge_id).weight, number_of_stops};
        }
        
        distance = 0;
        number_of_stops = 0;
    }
    
}

} //end namespace router
} //end namespace transport_catalogue