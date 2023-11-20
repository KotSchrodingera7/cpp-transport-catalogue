#include "json_reader.h"

#include "domain.h"
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */


void JsonReader::ParseBaseRequest(json::Node node, bool check_distance) {

    for(const auto &node_ : node.AsType<json::Array>()) {
        auto map_ = node_.AsType<json::Dict>();
        if( map_.count("type") ) {
            if( map_.at("type").AsType<std::string>() == "Stop" ) {
                if( check_distance ) {
                    for( const auto &[stop, distance] : map_.at("road_distances").AsType<json::Dict>() ) {
                        catalog_.AddDistance(catalog_.FindStop(map_.at("name").AsType<std::string>()), catalog_.FindStop(stop), distance.AsType<int>());
                    }
                } else {
                    Catalogue::TStop result;
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
                Catalogue::TBus result;
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

void JsonReader::ParseStatRequest(json::Node node) {
    json::Array arr_;
    for( const auto &req: node.AsType<json::Array>() ) {
        auto map = req.AsType<json::Dict>();
        int id = map.at("id").AsType<int>();

        if( map.at("type").AsType<std::string>() == "Stop" ) {
            auto stop_ = map.at("name").AsType<std::string>();
            if( catalog_.FindStop(stop_) == nullptr ) {
                arr_.emplace_back(json::Dict{
                    {"request_id", id},
                    {"error_message", std::string{"not found"}}
                });
            } else {
                auto number_buses_on_stop = catalog_.GetBusesOnStop(stop_);

                arr_.emplace_back(json::Dict{
                    {"buses", json::Array(number_buses_on_stop.begin(), number_buses_on_stop.end()) },
                    {"request_id", id}
                });
            }
        } else if ( map.at("type").AsType<std::string>() == "Bus" ) {
            auto bus = map.at("name").AsType<std::string>();
            auto stat = request_handler_.GetBusStat(bus);

            if( stat ) {
                arr_.emplace_back(json::Dict{
                    {"request_id", id},
                    {"curvature", (*stat).corvature},
                    {"route_length", (*stat).route_length},
                    {"stop_count", (*stat).stop_count},
                    {"unique_stop_count", (*stat).unique_stop}
                });
            } else {
                arr_.emplace_back(json::Dict{
                    {"request_id", id},
                    {"error_message", std::string{"not found"}}
                });
            }

        } else if( map.at("type").AsType<std::string>() == "Map" ) {
            std::ostringstream out;
            auto doc_ = request_handler_.RenderMap();
            doc_.Render(out);

            arr_.emplace_back(json::Dict{
                    {"map", out.str()},
                    {"request_id", id}
                });
        }
    }
    
    json::Print(json::Document{arr_}, std::cout);
}

void JsonReader::ParseRenderRequest(json::Node node) {
    json::Array arr_;
    for( const auto &[key, value]: node.AsType<json::Dict>() ) {

        if( key == "width") {
            render_setting_.width = value.AsType<double>();
        } else if( key == "height") {
            render_setting_.height = value.AsType<double>();
        } else if( key == "padding") {
            render_setting_.padding = value.AsType<double>();
        } else if( key == "line_width") {
            render_setting_.line_width = value.AsType<double>();
        } else if( key == "stop_radius") {
            render_setting_.stop_radius = value.AsType<double>();
        } else if( key == "bus_label_font_size") {
            render_setting_.bus_label_font_size = value.AsType<int>();
        } else if( key == "bus_label_offset") {
            auto point = value.AsType<json::Array>();
            render_setting_.bus_label_offset = svg::Point(point[0].AsType<double>(), point[1].AsType<double>());
        } else if( key == "stop_label_font_size") {
            render_setting_.stop_label_font_size = value.AsType<double>();
        } else if( key == "stop_label_offset") {
            auto point = value.AsType<json::Array>();
            render_setting_.stop_label_offset = svg::Point(point[0].AsType<double>(), point[1].AsType<double>());
        } else if( key == "underlayer_color") {
            if( value.IsType<json::Array>() ) {
                auto color = value.AsType<json::Array>();
                if( color.back().IsPureDouble() ) {
                    render_setting_.underlayer_color = svg::Rgba(color[0].AsType<int>(),
                                                            color[1].AsType<int>(),
                                                            color[2].AsType<int>(),
                                                            color[3].AsType<double>());
                } else {
                    render_setting_.underlayer_color = svg::Rgb(color[0].AsType<int>(),
                                                            color[1].AsType<int>(),
                                                            color[2].AsType<int>());
                }
            } else if( value.IsType<std::string>() ) {
                render_setting_.underlayer_color = value.AsType<std::string>();
            }
        } else if( key == "underlayer_width") {
            render_setting_.underlayer_width = value.AsType<double>();
        } else if( key == "color_palette") {
            auto colors = value.AsType<json::Array>();
            for(const auto &node : colors) {
                if( node.IsType<std::string>() ) {
                    render_setting_.color_palete.push_back(std::move(node.AsType<std::string>()));
                } else if( node.IsType<json::Array>() ) {
                    auto rgb_ = node.AsType<json::Array>();
                    if( rgb_.size() == 4 ) {
                        render_setting_.color_palete.push_back(std::move(svg::Rgba(rgb_[0].AsType<int>(),
                                                                    rgb_[1].AsType<int>(),
                                                                    rgb_[2].AsType<int>(),
                                                                    rgb_[3].AsType<double>())));
                    } else {
                        render_setting_.color_palete.push_back(std::move(svg::Rgb(rgb_[0].AsType<int>(),
                                                                    rgb_[1].AsType<int>(),
                                                                    rgb_[2].AsType<int>())));
                    }
                }
            }
        }
    }
}

void JsonReader::CreateCatalogFromJson(std::istream& input) {

    json::Document json_input_ = json::Load(input);

    if( json_input_.GetRoot().IsType<json::Dict>() ) {

        const auto map_ = json_input_.GetRoot().AsType<json::Dict>();

        if( map_.count("base_requests") ) {
            ParseBaseRequest(map_.at("base_requests"));
            ParseBaseRequest(map_.at("base_requests"), true);
        }

        if( map_.count("render_settings") ) {
            ParseRenderRequest(map_.at("render_settings"));
        }
        
        if( map_.count("stat_requests") ) {
            ParseStatRequest(map_.at("stat_requests"));
        }
    }
}


void JsonReader::CreateJSONFromCatalog(void) {

}