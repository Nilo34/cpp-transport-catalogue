#pragma once

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки. 
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

#include "geo.h"
#include "graph.h"

#include <vector>
#include <variant>
#include <string>

namespace transport_catalogue {

struct Bus;

struct Stop {
    std::string name;
    geo::Coordinates coord;
    std::vector<Bus*> buses;
};

struct Bus {
    std::string name;
    std::vector<Stop*> stops;
    bool is_roundtrip;
};

struct StatRequest {
    int id;
    std::string name;
    std::string type;
    std::string from;
    std::string to;
};

} //end namespace transport_catalogue