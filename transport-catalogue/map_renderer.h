#pragma once

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

/*
{
  "width": 1200.0,
  "height": 1200.0,

  "padding": 50.0,

  "line_width": 14.0,
  "stop_radius": 5.0,

  "bus_label_font_size": 20,
  "bus_label_offset": [7.0, 15.0],

  "stop_label_font_size": 20,
  "stop_label_offset": [7.0, -3.0],

  "underlayer_color": [255, 255, 255, 0.85],
  "underlayer_width": 3.0,

  "color_palette": [
    "green",
    [255, 160, 0],
    "red"
  ]
} */
#include "svg.h"
#include "geo.h"
#include "transport_catalogue.h"
#include <variant>
#include <map>
#include "json.h"


namespace renderer {
    using Setting = std::variant<double, int, svg::Color, svg::Point, std::vector<std::string>>;

    struct RenderSetting {
        double width;
        double height;
        double padding;  

        double line_width;
        double stop_radius;

        int bus_label_font_size;
        svg::Point bus_label_offset;

        int stop_label_font_size;
        svg::Point stop_label_offset;

        svg::Color underlayer_color;
        double underlayer_width;

        std::vector<svg::Color> color_palete;
    };

    class MapRenderer {
	public:
		MapRenderer( const Catalogue::TransportCatalogue& db) : db_(db) {};
		svg::Document RenderMap(const std::map<std::string, std::vector<geo::Coordinates>> &xys) const;

	public:
		RenderSetting render_setting_;

	private:
		void RenderPolyline(const std::map<std::string, std::vector<geo::Coordinates>> &xys, 
							svg::Document &doc, 
							const SphereProjector &proj) const;
		void RenderTextBus(const std::map<std::string, std::vector<geo::Coordinates>> &xys, 
							svg::Document &doc, 
							const SphereProjector &proj) const;
		void RenderTextStop(svg::Document &doc, 
							const SphereProjector &proj) const;

	private:
		const Catalogue::TransportCatalogue& db_;
		
    };
}

