
#include "json_reader.h"

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

    using namespace Catalogue;
    using namespace renderer;
    TransportCatalogue catalog;
    MapRenderer map_renderer(catalog);
    JsonReader reader_(catalog, map_renderer);
    reader_.CreateCatalogFromJson();
}   