#pragma once

#include "transport_catalogue.h"

#include <string_view>

namespace transport_catalogue {
namespace reader {

Stop FillInTheStop(std::string_view str);
void FillInTheDistances (std::string_view str, TransportCatalogue& tc);
Bus FillInTheBus(std::string_view str, TransportCatalogue& tc);
void FillInTheData(TransportCatalogue& tc);

} //end namespace reader
} //end namespace transport_catalogue