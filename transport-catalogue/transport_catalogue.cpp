#include <utility>
#include <unordered_set>

#include "transport_catalogue.h"
#include "geo.h"

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

void TransportCatalogue::AddDistanc(const Stop* stop1, const Stop* stop2, double&& distance) {
    distances_between_stops_.insert({std::make_pair(stop1, stop2), std::move(distance)});
    
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

int TransportCatalogue::NumberOfUniqueStops(Bus* bus) {
    std::unordered_set<Stop*> buffer;
    buffer.insert(bus->stops.begin(), bus->stops.end());
    return buffer.size();
}

double TransportCatalogue::GetDistanceBetweenVisitedStopsOnTheGround(Bus* bus) {
    double result = 0.0;
    size_t count_stops = bus->stops.size();
    
    for (int i=0; i < count_stops - 1; i++) {
        result += detail::geo::ComputeDistance({bus->stops.at(i)->latitude, bus->stops.at(i)->longitude},
                                  {bus->stops.at(i+1)->latitude, bus->stops.at(i+1)->longitude});
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

double TransportCatalogue::DistanceTravelled(Bus* bus) {
    double result = 0.0;
    size_t count_stops = bus->stops.size();
    
    for (int i=0; i < count_stops - 1; i++) {
        result += GetDistanceBetweenStops(bus->stops.at(i), bus->stops.at(i+1));
    }
    
    return result;
}

std::unordered_set<Bus*> TransportCatalogue::UniqueBusesOfStop(Stop* stop) {
    std::unordered_set<Bus*> buffer;
    buffer.insert(stop->buses.begin(), stop->buses.end());
    return buffer;
}

} //end namespace transport_catalogue