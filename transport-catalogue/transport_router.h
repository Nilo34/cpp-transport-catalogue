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

struct RoutingSettings {
    double bus_wait_time = 4;
    double bus_velocity = 30;
};

struct StopEdge {
    std::string_view name;
    double time = 0;
};

struct BusEdge {
    std::string_view name;
    double time = 0;
    int number_of_stops = 0;
};

struct BusWaitingPeriod {
  size_t start_bus_wait;
  size_t end_bus_wait;
};

struct RouteData {
    double time = 0;
    std::vector<std::variant<StopEdge, BusEdge>> edges;
};

class TransportRouter {
public:
    TransportRouter() = delete;
    TransportRouter(TransportCatalogue& db, RoutingSettings& routing_settings);
    
    void SetStops();
    
    void SetGraph();
    
    void AddEdgeToStop();
    void AddEdgeToBus();
    
    const graph::Edge<double> СreateEdgeToBus(Stop* start_stop, Stop* end_stop, const double distance);
    
    std::optional<RouteData> GetRouteInformation(size_t from, size_t to);
    BusWaitingPeriod GetBusWaitingPeriod(Stop* stop);
    
    template <typename Iterator>
    void FillBusToEdge(Iterator first, Iterator last, const Bus* bus);
    
private:
    // параметры
    TransportCatalogue& db_;
    
    RoutingSettings routing_settings_;
    
    std::unique_ptr<graph::DirectedWeightedGraph<double>> graph_;
    std::unique_ptr<graph::Router<double>> router_;
    
    std::unordered_map<size_t, std::variant<StopEdge, BusEdge>> map_id_to_edge_;
    std::unordered_map<const Stop*, BusWaitingPeriod> map_stop_to_bus_period_;
    
};

template <typename Iterator>
void TransportRouter::FillBusToEdge(Iterator first, Iterator last, const Bus* bus) {
    
    int distance = 0;
    int number_of_stops = 0;
    
    for (; first != last; ++first) {
        
        for (auto it = std::next(first); it != last; ++it) {
            distance += db_.GetDistanceBetweenStops(*prev(it), *it);
            ++number_of_stops;
            
            size_t edge_id = graph_->AddEdge(СreateEdgeToBus(*first, *it, distance));
            map_id_to_edge_[edge_id] = BusEdge{bus->name, graph_->GetEdge(edge_id).weight, number_of_stops};
        }
        
        distance = 0;
        number_of_stops = 0;
    }
    
}

} //end namespace router
} //end namespace transport_catalogue