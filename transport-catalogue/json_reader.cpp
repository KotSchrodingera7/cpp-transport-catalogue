#include "json_reader.h"

#include "json_builder.h"
#include "domain.h"
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */


void JsonReader::ParseBaseRequest(const json::Node &node, bool check_distance) {

    for(const auto &node_ : node.AsType<json::Array>()) {
        auto map_ = node_.AsType<json::Dict>();
        if( map_.count("type") ) {
            if( map_.at("type").AsType<std::string>() == "Stop" ) {
                if( check_distance ) {
                    for( const auto &[stop, distance] : map_.at("road_distances").AsType<json::Dict>() ) {
                        catalog_.AddDistance(catalog_.FindStop(map_.at("name").AsType<std::string>()), catalog_.FindStop(stop), distance.AsType<int>());
                    }
                } else {
                    TStop result;
                    result.name = map_.at("name").AsType<std::string>();
                    result.xy.lat = map_.at("latitude").AsType<double>();
                    result.xy.lng = map_.at("longitude").AsType<double>();
                    catalog_.AddStop(result);
                }
                
            }
        }
    }

    if( check_distance ) {
        return;
    }

    for(const auto &node_ : node.AsType<json::Array>()) {
        auto map_ = node_.AsType<json::Dict>();
        if( map_.count("type") ) {
            if( map_.at("type").AsType<std::string>() == "Bus" ) {
                TBus result;
                result.name = map_.at("name").AsType<std::string>();
                result.circle = map_.at("is_roundtrip").AsType<bool>();

                if( map_.count("stops") ) {
                    for( const json::Node &stop : map_.at("stops").AsType<json::Array>()  ) {

                        auto stop_addr = catalog_.FindStop(stop.AsType<std::string>());

                        if( stop_addr ) {
                            result.way.push_back(stop_addr);
                            result.unique_stop.insert(stop_addr->name);
                            catalog_.LinkStopWithBuses(stop_addr->name, result.name);
                        }
                    }
                }
                if( !result.circle && result.way.front()->name != result.way.back()->name ) {
                    result.endpoint = {result.way.front()->name, result.way.back()->name};
                } else {
                    result.endpoint.push_back(result.way.front()->name);
                }
                catalog_.AddBus(result);
            }
        }
    }
}

void JsonReader::ParseStatRequest(const json::Node &node) {
    json::Array arr_;
    for( const auto &req: node.AsType<json::Array>() ) {
        auto map = req.AsType<json::Dict>();
        int id = map.at("id").AsType<int>();
        auto type_req = map.at("type");
        if( type_req.AsType<std::string>() == "Stop" ) {
            auto stop_ = map.at("name").AsType<std::string>();
            if( catalog_.FindStop(stop_) == nullptr ) {
                arr_.emplace_back(json::Builder{}.StartDict()
                                    .Key("request_id").Value(id)
                                    .Key("error_message").Value("not found")
                                    .EndDict().Build()
                    );
            } else {
                auto number_buses_on_stop = catalog_.GetBusesOnStop(stop_);

                arr_.emplace_back(json::Builder{}.StartDict()
                                    .Key("buses").Value(json::Array(number_buses_on_stop.begin(), number_buses_on_stop.end()))
                                    .Key("request_id").Value(id)
                                .EndDict().Build());
            }
        } else if ( type_req.AsType<std::string>() == "Bus" ) {
            auto bus = map.at("name").AsType<std::string>();
            auto stat = request_handler_.GetBusStat(bus);

            if( stat ) {
                arr_.emplace_back(json::Builder{}.StartDict()
                        .Key("request_id").Value(id)
                        .Key("curvature").Value((*stat).corvature)
                        .Key("route_length").Value((*stat).route_length)
                        .Key("stop_count").Value((*stat).stop_count)
                        .Key("unique_stop_count").Value((*stat).unique_stop)
                    .EndDict().Build());
            } else {
                arr_.emplace_back(json::Builder{}.StartDict()
                                    .Key("request_id").Value(id)
                                    .Key("error_message").Value("not found")
                                    .EndDict().Build());
            }

        } else if( type_req.AsType<std::string>() == "Map" ) {
            std::ostringstream out;
            auto doc_ = request_handler_.RenderMap();
            doc_.Render(out);

            arr_.emplace_back(json::Builder{}.StartDict()
                        .Key("map").Value(out.str())
                        .Key("request_id").Value(id)
                    .EndDict().Build());
        } else if( type_req.AsType<std::string>() == "Route") {
            auto data_result = route_.GetWeigth(map.at("from").AsType<std::string>(), map.at("to").AsType<std::string>());
            

            if( data_result ) {
                json::Array array_route;
                for(const auto &data: (*data_result).id_stops) {
                    if( data.type == "wait" ) {
                        array_route.emplace_back(json::Builder{}.StartDict()
                                        .Key("stop_name").Value(data.stop_name.value())
                                        .Key("time").Value(data.weight)
                                        .Key("type").Value("Wait")
                                        .EndDict().Build());
                    } else if( data.type == "bus") {
                        array_route.emplace_back(json::Builder{}.StartDict()
                                        .Key("bus").Value(data.bus_name.value())
                                        .Key("span_count").Value(data.count)
                                        .Key("time").Value(data.weight)
                                        .Key("type").Value("Bus")
                                        .EndDict().Build());
                    }
                }

                arr_.emplace_back(json::Builder{}.StartDict()
                                    .Key("request_id").Value(id)
                                    .Key("total_time").Value((*data_result).weight)
                                    .Key("items").Value(array_route)
                                    .EndDict().Build());
            } else {

                arr_.emplace_back(json::Builder{}.StartDict()
                                    .Key("request_id").Value(id)
                                    .Key("error_message").Value("not found")
                                    .EndDict().Build());
            }
        }
    }
    
    json::Print(json::Document{arr_}, std::cout);
}

void JsonReader::ParseRenderRequest(const json::Node &node) {
    json::Array arr_;
    for( const auto &[key, value]: node.AsType<json::Dict>() ) {

        if( key == "width") {
            map_renderer_.render_setting_.width = value.AsType<double>();
        } else if( key == "height") {
            map_renderer_.render_setting_.height = value.AsType<double>();
        } else if( key == "padding") {
            map_renderer_.render_setting_.padding = value.AsType<double>();
        } else if( key == "line_width") {
            map_renderer_.render_setting_.line_width = value.AsType<double>();
        } else if( key == "stop_radius") {
            map_renderer_.render_setting_.stop_radius = value.AsType<double>();
        } else if( key == "bus_label_font_size") {
            map_renderer_.render_setting_.bus_label_font_size = value.AsType<int>();
        } else if( key == "bus_label_offset") {
            auto point = value.AsType<json::Array>();
            map_renderer_.render_setting_.bus_label_offset = svg::Point(point[0].AsType<double>(), point[1].AsType<double>());
        } else if( key == "stop_label_font_size") {
            map_renderer_.render_setting_.stop_label_font_size = value.AsType<double>();
        } else if( key == "stop_label_offset") {
            auto point = value.AsType<json::Array>();
            map_renderer_.render_setting_.stop_label_offset = svg::Point(point[0].AsType<double>(), point[1].AsType<double>());
        } else if( key == "underlayer_color") {
            if( value.IsType<json::Array>() ) {
                auto color = value.AsType<json::Array>();
                if( color.back().IsPureDouble() ) {
                    map_renderer_.render_setting_.underlayer_color = svg::Rgba(color[0].AsType<int>(),
                                                            color[1].AsType<int>(),
                                                            color[2].AsType<int>(),
                                                            color[3].AsType<double>());
                } else {
                    map_renderer_.render_setting_.underlayer_color = svg::Rgb(color[0].AsType<int>(),
                                                            color[1].AsType<int>(),
                                                            color[2].AsType<int>());
                }
            } else if( value.IsType<std::string>() ) {
                map_renderer_.render_setting_.underlayer_color = value.AsType<std::string>();
            }
        } else if( key == "underlayer_width") {
            map_renderer_.render_setting_.underlayer_width = value.AsType<double>();
        } else if( key == "color_palette") {
            auto colors = value.AsType<json::Array>();
            for(const auto &node : colors) {
                if( node.IsType<std::string>() ) {
                    map_renderer_.render_setting_.color_palete.push_back(std::move(node.AsType<std::string>()));
                } else if( node.IsType<json::Array>() ) {
                    auto rgb_ = node.AsType<json::Array>();
                    if( rgb_.size() == 4 ) {
                        map_renderer_.render_setting_.color_palete.push_back(std::move(svg::Rgba(rgb_[0].AsType<int>(),
                                                                    rgb_[1].AsType<int>(),
                                                                    rgb_[2].AsType<int>(),
                                                                    rgb_[3].AsType<double>())));
                    } else {
                        map_renderer_.render_setting_.color_palete.push_back(std::move(svg::Rgb(rgb_[0].AsType<int>(),
                                                                    rgb_[1].AsType<int>(),
                                                                    rgb_[2].AsType<int>())));
                    }
                }
            }
        }
    }
}

void JsonReader::ParseRoutingSetting(const json::Node &node) {
    auto map_ = node.AsType<json::Dict>();
    if( map_.count("bus_wait_time") ) {
        route_.WaitTime( map_.at("bus_wait_time").AsType<int>() );
    }

    if( map_.count("bus_velocity") ) {
        route_.SetSpeed(map_.at("bus_velocity").AsType<int>());
    }

    route_.CreateGraph();
}

void JsonReader::CreateCatalogFromJson(std::istream& input) {

    json::Document json_input_ = json::Load(input);

    if( json_input_.GetRoot().IsType<json::Dict>() ) {

        const auto map_ = json_input_.GetRoot().AsType<json::Dict>();

        if( map_.count("base_requests") ) {
            auto &base_req_ = map_.at("base_requests"); 
            ParseBaseRequest(base_req_);
            ParseBaseRequest(base_req_, true);
        }

        if( map_.count("render_settings") ) {
            ParseRenderRequest(map_.at("render_settings"));
        }
        
        if( map_.count("routing_settings") ) {
            ParseRoutingSetting(map_.at("routing_settings"));
        }

        if( map_.count("stat_requests") ) {
            ParseStatRequest(map_.at("stat_requests"));
        }
    }
}


void JsonReader::CreateJSONFromCatalog(void) {

}