#pragma once

/*
 * Здесь можно разместить код транспортного справочника
 */

#include "geo.h"
#include "domain.h"


#include <string>
#include <vector>
#include <deque>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <functional>

namespace transport_catalogue {

namespace detail {

class HasherDistancesBetweenStops {
public:
    size_t operator()(const std::pair<const Stop*, const Stop*> pair_stops) const {
        const int simple_number = 27;
        
        return d_hesher_(pair_stops.first->coord.lat) 
               + d_hesher_(pair_stops.first->coord.lng) * simple_number
               + d_hesher_(pair_stops.second->coord.lat) * simple_number*simple_number
               + d_hesher_(pair_stops.second->coord.lng) * simple_number*simple_number*simple_number;
    }
    
private:
    std::hash<double> d_hesher_;
};

} //end namespace detail

class TransportCatalogue {
public:
    void AddStop(Stop&& stop);
    void AddBus(Bus&& bus);
    void AddDistanc(const Stop* stop1, const Stop* stop2, double distance);
    
    Stop* GetStop(std::string_view stop_name);
    Bus* GetBus(std::string_view bus_name);
    
    int GetNumberOfUniqueStops(const Bus* bus) const;
    double GetDistanceBetweenVisitedStopsOnTheGround (const Bus* bus)const;
    
    double GetDistanceBetweenStops(const Stop* stop1, const Stop* stop2) const;
    double GetDistanceTravelledBus(const Bus* bus) const;
    
    std::unordered_set<Bus*> GetUniqueBusesOfStop(const Stop* stop) const;
    
    std::unordered_map<std::string_view, Stop*> GetMapToStop() const;
    std::unordered_map<std::string_view, Bus*> GetMapToBus() const;
    
private:
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    
    std::unordered_map<std::string_view, Stop*> map_to_stops_;
    std::unordered_map<std::string_view, Bus*> map_to_buses_;
    
    std::unordered_map<std::pair<const Stop*, const Stop*>, double, detail::HasherDistancesBetweenStops> distances_between_stops_;
};

} //end namespace transport_catalogue