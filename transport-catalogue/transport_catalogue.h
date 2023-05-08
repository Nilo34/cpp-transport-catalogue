#pragma once

#include "geo.h"

#include <string>
#include <vector>
#include <deque>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <functional>

namespace transport_catalogue {

struct Bus;

struct Stop {
    std::string name;
    double latitude;
    double longitude;
    std::vector<Bus*> buses;
};

struct Bus {
    std::string name;
    std::vector<Stop*> stops;
};

class HasherDistancesBetweenStops {
public:
    size_t operator()(const std::pair<const Stop*, const Stop*> pair_stops) const {
        const int simple_number = 17;
        
        return d_hesher_(pair_stops.first->latitude) 
               + d_hesher_(pair_stops.first->longitude) * simple_number
               + d_hesher_(pair_stops.second->latitude) * simple_number*simple_number
               + d_hesher_(pair_stops.second->longitude) * simple_number*simple_number*simple_number;
    }
    
private:
    std::hash<double> d_hesher_;
};


class TransportCatalogue {
public:
    void AddStop(Stop&& stop);
    void AddBus(Bus&& bus);
    void AddDistanc(const Stop* stop1, const Stop* stop2, double&& distance);
    
    Stop* GetStop(std::string_view stop_name);
    Bus* GetBus(std::string_view bus_name);
    
    int NumberOfUniqueStops(Bus* bus);
    double GetDistanceBetweenVisitedStopsOnTheGround (Bus* bus);
    
    double GetDistanceBetweenStops(const Stop* stop1, const Stop* stop2) const;
    double DistanceTravelled(Bus* bus);
    
    std::unordered_set<Bus*> UniqueBusesOfStop(Stop* stop);
private:
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    
    std::unordered_map<std::string_view, Stop*> map_to_stops_;
    std::unordered_map<std::string_view, Bus*> map_to_buses_;
    
    std::unordered_map<std::pair<const Stop*, const Stop*>, double, HasherDistancesBetweenStops> distances_between_stops_;
};

} //end namespace transport_catalogue