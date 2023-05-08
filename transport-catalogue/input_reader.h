#pragma once

#include "transport_catalogue.h"

#include <string_view>

namespace transport_catalogue {
namespace reader {

Stop IStop(std::string_view str);
void IDistances (std::string_view str, TransportCatalogue& tc);
Bus IBus(std::string_view str, TransportCatalogue& tc);
void Input(TransportCatalogue& tc);

} //end namespace reader
} //end namespace transport_catalogue