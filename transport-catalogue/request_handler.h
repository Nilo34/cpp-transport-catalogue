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

namespace request_handler {

class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(transport_catalogue::TransportCatalogue& db, 
                   const transport_catalogue::renderer::MapRenderer& renderer);
    
    // Возвращает информацию о маршруте (запрос Bus)
    //std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;
    
    // Возвращает маршруты, проходящие через
    //const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;
    
    std::vector<geo::Coordinates> GetStopsCoordinates() const;
    
    void RenderMapLine(std::vector<std::pair<transport_catalogue::Bus*, int>>& buses_palette, 
                       svg::Document& document, 
                       transport_catalogue::renderer::SphereProjector& sphere_projector) const;
    void RenderMapRoutesName(std::vector<std::pair<transport_catalogue::Bus*, int>>& buses_palette, 
                             svg::Document& document, 
                             transport_catalogue::renderer::SphereProjector& sphere_projector) const;
    
    void RenderMapStops(svg::Document& document, 
                        transport_catalogue::renderer::SphereProjector& sphere_projector) const;
    void RenderMapStopsName(svg::Document& document, 
                            transport_catalogue::renderer::SphereProjector& sphere_projector) const;
    
    void RenderMap(std::ostream& out) const;
    
    json::Node OutputTheBusData(domain::StatRequest& stat_request);
    json::Node OutputTheStopData(domain::StatRequest& stat_request);
    json::Node OutputTheSVGMapData(domain::StatRequest& stat_request);
    
    json::Document ReplyToTheRequest(std::vector<domain::StatRequest>& stat_requests);
    
private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    transport_catalogue::TransportCatalogue& db_;
    const transport_catalogue::renderer::MapRenderer& renderer_;
};




} //end namespace request_handler