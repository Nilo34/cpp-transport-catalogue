#include "input_reader.h"

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace transport_catalogue {
namespace reader {

Stop FillInTheStop(std::string_view str) {
    Stop result;
    
    auto start_indent = 5;
    auto start_first_parametr_stop = str.find(':');
    auto start_second_parametr_stop = str.find(',');
    auto indent = 2;
    
    result.name = str.substr(start_indent, start_first_parametr_stop - start_indent);
    
    result.coord.lat = std::stod(std::string{str.substr(start_first_parametr_stop + indent, 
                                 start_second_parametr_stop - start_first_parametr_stop - indent)});
    
    result.coord.lng = std::stod(std::string{str.substr(start_second_parametr_stop + indent)});
    
    return result;
}

void FillInTheDistances (std::string_view str, TransportCatalogue& tc) {
    auto start_indent = 5;
    auto start_parametr_stop = str.find(':');
    auto indent = 2;
    
    std::string_view name_stop1 = str.substr(start_indent, start_parametr_stop - start_indent);
    auto stop1 = tc.GetStop(name_stop1);
    
    str = str.substr(str.find(',') + 1);
    str = str.substr(str.find(',') + indent);
    std::string_view name_stop2;
    
    while (str.find(',') != std::string_view::npos) {
        double distance = std::stof(std::string{str.substr(0, str.find('m'))});
        
        name_stop2 = str.substr(str.find('m') + start_indent);
        name_stop2 = name_stop2.substr(0, name_stop2.find(','));
        
        tc.AddDistanc(stop1, tc.GetStop(name_stop2), std::move(distance));
        str = str.substr(str.find(',') + indent);
    }
    
    if (str.find('m') != std::string_view::npos) {
        double distance = std::stof(std::string{str.substr(0, str.find('m'))});
        name_stop2 = str.substr(str.find('m') + start_indent);
        tc.AddDistanc(stop1, tc.GetStop(name_stop2), std::move(distance));
    }
    
}

Bus FillInTheBus(std::string_view str, TransportCatalogue& tc) {
    Bus result;
    
    auto start_indent = 4;
    auto start_first_parametr_bus = str.find(':');
    auto indent = 2;
    
    result.name = str.substr(start_indent, start_first_parametr_bus - start_indent);
    
    str = str.substr(start_first_parametr_bus + indent);
    
    auto separator_position = str.find('>');
    
    if (separator_position != std::string_view::npos) {
        
        while (separator_position != std::string_view::npos) {
            result.stops.push_back(const_cast<Stop*>(tc.GetStop(str.substr(0, separator_position-1))));
            str = str.substr(separator_position + indent);
            separator_position = str.find('>');
        }
        result.stops.push_back(const_cast<Stop*>(tc.GetStop(str.substr(0, separator_position-1))));
        
    } else {
        
        separator_position = str.find('-');
        while (separator_position != std::string_view::npos) {
            result.stops.push_back(const_cast<Stop*>(tc.GetStop(str.substr(0, separator_position-1))));
            str = str.substr(separator_position + indent);
            separator_position = str.find('-');
        }
        result.stops.push_back(const_cast<Stop*>(tc.GetStop(str.substr(0, separator_position-1))));
        
        size_t number_of_stops_there = result.stops.size();
        
        for (size_t i = 2; i <= number_of_stops_there; ++i) {
            result.stops.push_back(result.stops.at(number_of_stops_there-i));
        }
        
    }
    
    return result;
}


void FillInTheData(TransportCatalogue& tc) {
    
    std::string number_of_requests_s;
    std::getline(std::cin, number_of_requests_s);
    
    if (number_of_requests_s != "") {
        
        int number_of_requests_i = std::stoi(number_of_requests_s);
        
        std::string read_line;
        
        std::vector<std::string> stops;
        std::vector<std::string> buses;
        
        for (int i = 0; i < number_of_requests_i; i++) {
            std::getline(std::cin, read_line);
            
            if (read_line.substr(0, 3) == "Bus") {
                buses.push_back(std::move(read_line));
            } else {
                stops.push_back(std::move(read_line));
            }
        }
        
        for (auto& stop: stops) {
            tc.AddStop(FillInTheStop(stop));
        }
        
        for (auto& stop: stops) {
            FillInTheDistances(stop, tc);
        }
        
        for (auto& bus: buses) {
            tc.AddBus(FillInTheBus(bus, tc));
        }
        
    }
    
}

} //end namespace reader
} //end namespace transport_catalogue