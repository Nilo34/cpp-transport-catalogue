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

namespace reader {

Stop FillInTheStop(json::Node& node);
void FillInTheDistances (json::Node& node, TransportCatalogue& tc);
Bus FillInTheBus(json::Node& node, TransportCatalogue& tc);
void FillInTheData(const json::Node& node, TransportCatalogue& tc);

svg::Color ParsingCollor(const json::Node& node);

void FillInTheRenderSettings(const json::Node& node, renderer::RenderSettings& render_settings);

} //end namespace reader

namespace request {

struct StatRequest {
    int id;
    std::string name;
    std::string type;
};

void ParsingRequest(const json::Node& node, std::vector<StatRequest>& stat_requests);

} //end namespace request

void SplittingDocument(json::Document& document_in,
                       renderer::RenderSettings& render_settings,
                       TransportCatalogue& tc,
                       std::vector<request::StatRequest>& stat_requests);

} //end namespace handling_json
} //end namespace transport_catalogue