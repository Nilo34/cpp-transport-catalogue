#pragma once

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика запросов.
 * Вы можете реализовать обработку запросов способом, который удобнее вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json.h"
#include "domain.h"
#include "json_reader.h"

#include <iostream>
#include <vector>

namespace transport_catalogue {
namespace request_handler {

class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(transport_catalogue::TransportCatalogue& db, 
                   const transport_catalogue::renderer::MapRenderer& renderer);
    
    json::Node OutputTheBusData(handling_json::request::StatRequest& stat_request);
    json::Node OutputTheStopData(handling_json::request::StatRequest& stat_request);
    json::Node OutputTheSVGMapData(handling_json::request::StatRequest& stat_request);
    
    json::Document ReplyToTheRequest(std::vector<handling_json::request::StatRequest>& stat_requests);
    
private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};




} //end namespace request_handler
} //end namespace transport_catalogue