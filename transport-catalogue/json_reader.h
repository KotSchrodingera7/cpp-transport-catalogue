#pragma once

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
#include "transport_catalogue.h"
#include "json.h"
#include "request_handler.h"
#include "map_renderer.h"
#include <iostream>


class JsonReader {
public:
    JsonReader(Catalogue::TransportCatalogue &catalog) : catalog_(catalog), request_handler_(catalog_, render_setting_) {}

    void CreateCatalogFromJson(std::istream& input = std::cin);
    void CreateJSONFromCatalog(void);

private:
    void ParseBaseRequest(json::Node node, bool check_distance = false);
    void ParseStatRequest(json::Node node);
    void ParseRenderRequest(json::Node node);
private:
    Catalogue::TransportCatalogue &catalog_;
    json::Node return_node_;
    RequestHandler request_handler_;
    renderer::MapRenderer render_setting_;
};