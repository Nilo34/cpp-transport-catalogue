#include "transport_catalogue.h"
#include "json_reader.h"
#include "request_handler.h"

#include <iostream>

int main() {
    /*
     * Примерная структура программы:
     *
     * Считать JSON из stdin
     * Построить на его основе JSON базу данных транспортного справочника
     * Выполнить запросы к справочнику, находящиеся в массиве "stat_requests", построив JSON-массив
     * с ответами.
     * Вывести в stdout ответы в виде JSON
     */
    
    
    transport_catalogue::TransportCatalogue TCatalogue;
    
    json::Document input_data;
    json::Document result_data;
    
    transport_catalogue::renderer::RenderSettings render_settings;
    transport_catalogue::RoutingSettings routing_setings;
    
    input_data = json::Load(std::cin);
    
    std::vector<transport_catalogue::StatRequest> stat_requests;
    
    transport_catalogue::handling_json::SplittingDocument(input_data,
                                                          render_settings,
                                                          routing_setings,
                                                          TCatalogue,
                                                          stat_requests);
    
    transport_catalogue::renderer::MapRenderer map_renderer(render_settings);
    transport_catalogue::router::TransportRouter transport_router(TCatalogue, routing_setings);
    
    transport_catalogue::request_handler::RequestHandler request_handler(TCatalogue, map_renderer, transport_router);
    
    result_data = request_handler.ReplyToTheRequest(stat_requests);
    
    json::Print(result_data, std::cout);
}