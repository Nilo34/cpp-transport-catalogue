#include "stat_reader.h"

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <iomanip>
#include <unordered_set>
#include <algorithm>

namespace transport_catalogue {
namespace request {

void OBus(TransportCatalogue& tc, std::string_view query_string) {
    auto start_indent = 4;
    
    query_string = query_string.substr(start_indent);
    const Bus* bus = tc.GetBus(query_string);
            
   using namespace std::string_literals;
   if (bus != nullptr) {
       std::cout << "Bus "s << bus->name << ": "
                 << bus->stops.size() << " stops on route, "s
                 << tc.GetNumberOfUniqueStops(bus) << " unique stops, "s
                 << std::setprecision(6) << tc.GetDistanceTravelledBus(bus)
                 << " route length, "s
                 << tc.GetDistanceTravelledBus(bus)/tc.GetDistanceBetweenVisitedStopsOnTheGround(bus)
                 << " curvature"s
                 << std::endl;
   } else {
       std::cout << "Bus "s << query_string << ": not found"s << std::endl;
   }
}

void OStop(TransportCatalogue& tc, std::string_view query_string) {
    auto start_indent = 5;
    
    query_string = query_string.substr(start_indent);
    const Stop* stop = tc.GetStop(query_string);
    
    using namespace std::string_literals;
    if (stop != nullptr) {
        std::unordered_set<Bus*> bus_roster = tc.GetUniqueBusesOfStop(stop);
        
        if (bus_roster.size() > 0) {
            std::cout << "Stop "s << query_string << ": buses"s;
            
            std::vector<Bus*> sorted_name_buses({bus_roster.begin(), bus_roster.end()});
            std::sort(sorted_name_buses.begin(), sorted_name_buses.end(),
                      [] (Bus* rhs, Bus* lhs) {
                          return rhs->name < lhs->name;
                      });
            for (Bus* bus: sorted_name_buses) {
                std::cout << " "s << bus->name;
            }
            std::cout << std::endl;
            
        } else {
            std::cout << "Stop "s << query_string << ": no buses"s << std::endl;
        }
        
    } else {
        std::cout << "Stop "s << query_string << ": not found"s << std::endl;
    }
    
}

void ReadEnquiry(TransportCatalogue& tc) {
    
    std::string number_of_requests_s;
    std::getline(std::cin, number_of_requests_s);
    
    if (number_of_requests_s != "") {
        
        int number_of_requests_i = std::stoi(number_of_requests_s);
        
        std::string read_line;
        
        for (int i = 0; i < number_of_requests_i; i++) {
            std::getline(std::cin, read_line);
            
            if (read_line.substr(0, 3) == "Bus") {
                OBus(tc, read_line);
            } else if (read_line.substr(0, 4) == "Stop") {
                OStop(tc, read_line);
            }
        }
    }
}

} //end namespace request
} //end namespace transport_catalogue