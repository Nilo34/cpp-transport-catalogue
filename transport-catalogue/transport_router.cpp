#include "transport_router.h"

namespace transport_catalogue {
namespace router {

TransportRouter::TransportRouter(TransportCatalogue& db, RoutingSettings& routing_settings)
: db_(db)
, routing_settings_(routing_settings)
{
    SetGraph();
    router_ = std::make_unique<graph::Router<double>>(*graph_);
}

void TransportRouter::SetStops() {
    size_t position = 0;
    
    for (const auto [_, stop] : db_.GetMapToStop()) {
        map_stop_to_bus_period_[stop] = BusWaitingPeriod{position, position+1};
        
        position += 2;
    }
}

void TransportRouter::AddEdgeToStop() {
    for (const auto [stop, summit_number] : map_stop_to_bus_period_) {
        size_t edge_id = graph_->AddEdge(graph::Edge<double>{summit_number.start_bus_wait,
                                                                    summit_number.end_bus_wait,
                                                                    routing_settings_.bus_wait_time});
        
        map_id_to_edge_[edge_id] = StopEdge{stop->name, routing_settings_.bus_wait_time};
    }
}
void TransportRouter::AddEdgeToBus() {
    for (const auto [_, bus] : db_.GetMapToBus()) {
        FillBusToEdge(bus->stops.begin(),
                      bus->stops.end(),
                      bus);
        if (!bus->is_roundtrip) {
            FillBusToEdge(bus->stops.rbegin(),
                          bus->stops.rend(),
                          bus);
        }
    }
}

void TransportRouter::SetGraph() {
    const size_t stops_ptr_size = db_.GetMapToStop().size();
    
    graph_ = std::make_unique<graph::DirectedWeightedGraph<double>>(2 * stops_ptr_size);
    
    SetStops();
    
    AddEdgeToStop();
    AddEdgeToBus();
    
}

const graph::Edge<double> TransportRouter::СreateEdgeToBus(Stop* start_stop, Stop* end_stop, const double distance) {
    
    graph::Edge<double> result;
    
    result.from = map_stop_to_bus_period_.at(start_stop).end_bus_wait;
    result.to = map_stop_to_bus_period_.at(end_stop).start_bus_wait;
    result.weight = distance  / (routing_settings_.bus_velocity * METERS_IN_A_KILOMETER / MINUTES_PER_HOUR);
    
    return result;
}


std::optional<RouteData> TransportRouter::GetRouteInformation(size_t from, size_t to) {
    const auto& route_info = router_->BuildRoute(from, to);
    
    if (route_info) {
        RouteData result;
        
        result.time = route_info->weight;
        
        for (const size_t id : route_info->edges) {
            result.edges.emplace_back(map_id_to_edge_.at(id));
        }
        
        return result;
    }
    
    return std::nullopt;
}

BusWaitingPeriod TransportRouter::GetBusWaitingPeriod(Stop* stop) {
    return map_stop_to_bus_period_.at(stop);
}

} //end namespace router
} //end namespace transport_catalogue