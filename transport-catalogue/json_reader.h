#pragma once

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "domain.h"

namespace transport_catalogue {
namespace handling_json {

void SplittingDocument(json::Document& document_in,
                       renderer::RenderSettings& render_settings,
                       TransportCatalogue& tc,
                       std::vector<domain::StatRequest>& stat_requests);

namespace reader {

Stop FillInTheStop(json::Node& node);
void FillInTheDistances (json::Node& node, TransportCatalogue& tc);
Bus FillInTheBus(json::Node& node, TransportCatalogue& tc);
void FillInTheData(const json::Node& node, TransportCatalogue& tc);

void FillInTheRenderSettings(const json::Node& node, renderer::RenderSettings& render_settings);

} //end namespace reader

namespace request {

void ParsingRequest(const json::Node& node, std::vector<domain::StatRequest>& stat_requests);

} //end namespace request



} //end namespace handling_json
} //end namespace transport_catalogue