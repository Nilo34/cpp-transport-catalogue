#pragma once

#include "transport_catalogue.h"

namespace transport_catalogue {
namespace request {

void OBus(TransportCatalogue& tc, std::string_view query_string);
void OStop(TransportCatalogue& tc, std::string_view query_string);

void ReadEnquiry(TransportCatalogue& tc);

} //end namespace request
} //end namespace transport_catalogue