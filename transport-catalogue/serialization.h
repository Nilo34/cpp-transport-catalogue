#pragma once

#include "transport_catalogue.h"
#include "transport_catalogue.pb.h"

#include "map_renderer.h"
#include "map_renderer.pb.h"

#include "transport_router.h"
#include "transport_router.pb.h"

#include <string>
#include <iostream>
#include <variant>

namespace transport_catalogue {
namespace serialization {

struct SerializationSettings {
    std::string file_name;
};

//-------------------SERIALIZATION--------------------

transport_catalogue_serialize::TransportCatalogue TransportCatalogueSerialization(const TransportCatalogue& transport_catalogue);
colors_serialize::Color ColorSerialization(const svg::Color& color);
render_serialize::RenderSettings RenderSettingsSerialization(const renderer::RenderSettings& render_settings);
transport_router_serialize::RoutingSettings RoutingSettingsSerialization(const router::RoutingSettings& routing_settings);

void Serialization(const TransportCatalogue& transport_catalogue,
                   const renderer::RenderSettings& render_settings,
                   const router::RoutingSettings& routing_settings,
                   std::ostream& output);

//-------------------DESERIALIZATION--------------------

TransportCatalogue TransportCatalogueDeserialization(const transport_catalogue_serialize::TransportCatalogue& transport_catalogue_proto);
svg::Color ColorDeserialization(const colors_serialize::Color& color_proto);
renderer::RenderSettings RenderSettingsDeserialization(const render_serialize::RenderSettings& render_settings_proto);
router::RoutingSettings RoutingSettingsDeserialization(const transport_router_serialize::RoutingSettings& routing_settings_proto);

//RoutingSettings

void Deserialization(TransportCatalogue& transport_catalogue, 
                     renderer::RenderSettings& render_settings,
                     router::RoutingSettings& routing_settings,
                     std::istream& input);


} //end namespace serialization
} //end namespace transport_catalogue