#pragma once

#include "transport_catalogue.h"

namespace transport_catalogue {
namespace request {

void OutputTheBusData(const TransportCatalogue& tc, std::string_view query_string);
void OutputTheStopData(const TransportCatalogue& tc, std::string_view query_string);

void ReadEnquiry(const TransportCatalogue& tc);

} //end namespace request
} //end namespace transport_catalogue