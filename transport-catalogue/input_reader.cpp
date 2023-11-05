// напишите решение с нуля
// код сохраните в свой git-репозиторий

#include "input_reader.h"

#include <iostream>

namespace Catalogue {
    namespace InputRequest {
        std::string_view EraseEmptySymbol(std::string_view line) {

            line.remove_prefix(std::min(line.find_first_not_of(" "), line.size()));

            auto it = std::prev(line.end());
            while(*it == ' ') {
                line.remove_suffix(std::distance(it, line.end()));
                it = std::prev(line.end());
            }

            return line;
        }

        std::pair<std::string_view, size_t> EraseEmptyFindSymbol(std::string_view line, size_t remove_pos, char c) {
            if( remove_pos ) {
                line.remove_prefix(remove_pos);
            }

            line.remove_prefix(std::min(line.find_first_not_of(" "), line.size()));

            auto it = std::prev(line.end());
            while(*it == ' ') {
                line.remove_suffix(std::distance(it, line.end()));
                it = std::prev(line.end());
            }

            return {line, line.find(c)};
        }


        void ParseStop(std::string_view line, TransportCatalogue &catalog, bool parse_dst) {
            struct TStop result;

            std::pair<std::string_view, size_t> str_and_pos = EraseEmptyFindSymbol(line, 0, ' ');

            if( std::string(str_and_pos.first.substr(0, str_and_pos.second)) != "Stop") {
                return;
            }
            
            str_and_pos = EraseEmptyFindSymbol(str_and_pos.first, str_and_pos.second + 1, ':');
            result.name = std::move(std::string(str_and_pos.first.substr(0, str_and_pos.second)));

            str_and_pos = EraseEmptyFindSymbol(str_and_pos.first, str_and_pos.second + 1, ',');
            result.xy.lat = std::move(std::atof(std::string(str_and_pos.first.substr(0, str_and_pos.second)).c_str()));

            str_and_pos = EraseEmptyFindSymbol(str_and_pos.first, str_and_pos.second + 1, ',');
            result.xy.lng = std::move(std::atof(std::string(str_and_pos.first.substr(0, str_and_pos.second)).c_str()));

            if( parse_dst  ) {
                if( str_and_pos.second != std::string::npos ) {
                    str_and_pos = EraseEmptyFindSymbol(str_and_pos.first, str_and_pos.second + 1, 'm');

                    std::string stop;
                    while( str_and_pos.second != std::string::npos ) {
                        int distance = std::atoi(std::string(str_and_pos.first.substr(0, str_and_pos.second)).c_str());

                        str_and_pos = EraseEmptyFindSymbol(str_and_pos.first, str_and_pos.second + 1, 't');
                        str_and_pos = EraseEmptyFindSymbol(str_and_pos.first, str_and_pos.second + 2, ',');
                        
                        if( str_and_pos.second != std::string::npos) {
                            stop = std::move(std::string(str_and_pos.first.substr(0, str_and_pos.second)));
                            str_and_pos = EraseEmptyFindSymbol(str_and_pos.first, str_and_pos.second + 1, 'm');

                        } else {
                            stop = std::move(std::string(str_and_pos.first.substr(0, str_and_pos.first.size())));
                        }

                        catalog.AddDistance(catalog.FindStop(result.name), catalog.FindStop(stop), distance);
                    }
                }

                return;
            }
            catalog.AddStop(result);
        }

        void ParseBus(std::string_view line, TransportCatalogue &catalog) {
            struct TBus result;

            std::pair<std::string_view, size_t> str_and_pos = EraseEmptyFindSymbol(line, 0, ' ');

            if( std::string(str_and_pos.first.substr(0, str_and_pos.second)) != "Bus") {
                return;
            }

            str_and_pos = EraseEmptyFindSymbol(str_and_pos.first, str_and_pos.second + 1, ':');
            result.name = std::move(std::string(str_and_pos.first.substr(0, str_and_pos.second)));
            result.circle = false;
            char find_symbol = '>';
            str_and_pos = EraseEmptyFindSymbol(str_and_pos.first, str_and_pos.second + 1, find_symbol);

            size_t pos = str_and_pos.second;
            std::vector<std::string_view> stops_;

            if ( str_and_pos.second == std::string::npos ) {

                result.circle = true;
                find_symbol = '-';
                pos = str_and_pos.first.find(find_symbol);
            }

            while(pos != std::string::npos) {
                std::string_view stop = str_and_pos.first.substr(0, pos);
                stops_.push_back(EraseEmptySymbol(stop));
                str_and_pos.first.remove_prefix(pos + 1);
                pos = str_and_pos.first.find(find_symbol);
            }
            stops_.push_back(EraseEmptySymbol(str_and_pos.first));
            
            for(const auto &stop: stops_) {

                auto stop_addr = catalog.FindStop(stop);

                if( stop_addr ) {
                    result.way.push_back(stop_addr);
                    result.unique_stop.insert(stop_addr->name);
                    catalog.LinkStopWithBuses(stop_addr->name, result.name);
                }
            }

            catalog.AddBus(result);
        }

        void AddRequest(TransportCatalogue &catalog, std::istream& input) {

            int count_line = 0;

            std::vector<std::vector<std::string>> result;
            input >> count_line;
            std::string line;
            std::vector<std::string> parse_;
            getline(input, line);

            for(int i = 0; i < count_line; i++ ) {
            
                getline(input, line);
                parse_.push_back(line);
            }

            for(const auto &line : parse_ ) {
                ParseStop(line, catalog, false);
            }

            for(const auto &line : parse_ ) {
                ParseBus(line, catalog);
            }

            for(const auto &line : parse_ ) {
                ParseStop(line, catalog, true);
            }
        }

    }
}