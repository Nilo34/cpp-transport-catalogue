#pragma once

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "serialization.h"

namespace transport_catalogue {
namespace handling_json {

namespace reader {

void FillInTheSerializationSettings(json::Node& node, serialization::SerializationSettings& serialization_settings);

Stop FillInTheStop(json::Node& node);
void FillInTheDistances (json::Node& node, TransportCatalogue& tc);
Bus FillInTheBus(json::Node& node, TransportCatalogue& tc);
void FillInTheData(const json::Node& node, TransportCatalogue& tc);

svg::Color ParsingCollor(const json::Node& node);

void FillInTheRenderSettings(const json::Node& node, renderer::RenderSettings& render_settings);
void FillInTheRoutingSettings(const json::Node& node, router::RoutingSettings& routing_settings);

} //end namespace reader

namespace request {

void ParsingRequest(const json::Node& node, std::vector<StatRequest>& stat_requests);

} //end namespace request

void SplittingDocumentMakeBase(json::Document& document_in,
                               renderer::RenderSettings& render_settings,
                               router::RoutingSettings& routing_settings,
                               TransportCatalogue& tc,
                               serialization::SerializationSettings& serialization_settings);
void SplittingDocumentProcessRequests(json::Document& document_in,
                                      serialization::SerializationSettings& serialization_settings,
                                      std::vector<StatRequest>& stat_requests);

} //end namespace handling_json
} //end namespace transport_catalogue