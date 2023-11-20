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

#include <optional>
#include <map>



struct BusStat {
    int unique_stop;
    int stop_count;
    int route_length;
    double corvature;
};

class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const Catalogue::TransportCatalogue& db, 
                    const renderer::MapRenderer& renderer) : db_(db), renderer_(renderer) {}//, const renderer::MapRenderer& renderer);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;
    // Возвращает маршруты, проходящие через
    // const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;
    
    // Этот метод будет нужен в следующей части итогового проекта
    svg::Document RenderMap() const;
private:
    std::map<std::string_view, std::vector<geo::Coordinates>> GetPointsWay() const;
private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const Catalogue::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};