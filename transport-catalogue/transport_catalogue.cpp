#include "transport_catalogue.h"
#include "geo.h"


#include <utility>
#include <unordered_set>


namespace transport_catalogue {

void TransportCatalogue::AddStop(Stop&& stop) {
    stops_.push_back(std::move(stop));
    map_to_stops_.insert({(&stops_.back())->name, &stops_.back()});
}

void TransportCatalogue::AddBus(Bus&& bus) {
    buses_.push_back(std::move(bus));
    map_to_buses_.insert({(&buses_.back())->name, &buses_.back()});
    
    for (Stop* stop: buses_.back().stops) {
        stop->buses.push_back(&buses_.back());
    }
}

void TransportCatalogue::AddDistanc(const Stop* stop1, const Stop* stop2, double distance) {
    distances_between_stops_.insert({std::make_pair(stop1, stop2), distance});
    
}

Stop* TransportCatalogue::GetStop(std::string_view stop_name) {
    if (map_to_stops_.count(stop_name) != 0) {
        return map_to_stops_.at(stop_name);
    }
    return nullptr;
}
Bus* TransportCatalogue::GetBus(std::string_view bus_name) {
    if (map_to_buses_.count(bus_name) != 0) {
        return map_to_buses_.at(bus_name);
    }
    return nullptr;
}

int TransportCatalogue::GetNumberOfUniqueStops(const Bus* bus) const {
    std::unordered_set<Stop*> buffer;
    buffer.insert(bus->stops.begin(), bus->stops.end());
    return buffer.size();
}

double TransportCatalogue::GetDistanceBetweenVisitedStopsOnTheGround(const Bus* bus) const {
    double result = 0.0;
    size_t count_stops = bus->stops.size();
    
    for (size_t i=0; i < count_stops - 1; i++) {
        result += geo::ComputeDistance(bus->stops.at(i)->coord, bus->stops.at(i+1)->coord);
    }
    
    return result;
}

double TransportCatalogue::GetDistanceBetweenStops(const Stop* stop1, const Stop* stop2) const {
    if (distances_between_stops_.count({stop1, stop2}) != 0) {
        return distances_between_stops_.at({stop1, stop2});
    } else if (distances_between_stops_.count({stop2, stop1}) != 0) {
        return distances_between_stops_.at({stop2, stop1});
    }
    return 0;
}

double TransportCatalogue::GetDistanceTravelledBus(const Bus* bus) const {
    double result = 0.0;
    size_t count_stops = bus->stops.size();
    
    for (size_t i=0; i < count_stops - 1; i++) {
        result += GetDistanceBetweenStops(bus->stops.at(i), bus->stops.at(i+1));
    }
    
    return result;
}

std::unordered_set<Bus*> TransportCatalogue::GetUniqueBusesOfStop(const Stop* stop) const {
    std::unordered_set<Bus*> buffer;
    buffer.insert(stop->buses.begin(), stop->buses.end());
    return buffer;
}

const std::unordered_map<std::string_view, Stop*>& TransportCatalogue::GetMapToStop() const {
    return map_to_stops_;
}
const std::unordered_map<std::string_view, Bus*>& TransportCatalogue::GetMapToBus() const {
    return map_to_buses_;
}

const std::deque<Stop>& TransportCatalogue::GetStops() const {
    return stops_;
}
const std::deque<Bus>& TransportCatalogue::GetBuses() const {
    return buses_;
}
const std::unordered_map<std::pair<const Stop*, const Stop*>, double, detail::HasherDistancesBetweenStops>& TransportCatalogue::GetDistancesBetweenStops() const {
    return distances_between_stops_;
}

} //end namespace transport_catalogue