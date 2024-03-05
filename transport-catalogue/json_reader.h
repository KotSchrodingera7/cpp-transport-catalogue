#pragma once

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
#include "transport_catalogue.h"
#include "json.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <iostream>


class JsonReader {
public:
    JsonReader(Catalogue::TransportCatalogue &catalog, renderer::MapRenderer &map_renderer) : 
                                            catalog_(catalog), 
                                            map_renderer_(map_renderer),
                                            request_handler_(catalog_, map_renderer),
                                            route_(catalog_)
                                            {}

    void CreateCatalogFromJson(std::istream& input = std::cin);
    void CreateJSONFromCatalog(void);

private:
    void ParseBaseRequest(const json::Node &node, bool check_distance = false);
    void ParseStatRequest(const json::Node &node);
    void ParseRenderRequest(const json::Node &node);
    void ParseRoutingSetting(const json::Node &node);
    void CreateGraph();
private:
    Catalogue::TransportCatalogue &catalog_;
    renderer::MapRenderer &map_renderer_;
    json::Node return_node_;
    RequestHandler request_handler_;
    Route::TransportRouter<double> route_;
};