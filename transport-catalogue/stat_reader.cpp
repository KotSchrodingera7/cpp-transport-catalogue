// напишите решение с нуля
// код сохраните в свой git-репозиторий

#include "stat_reader.h"
#include "geo.h"
#include <iomanip>
namespace Catalogue {
    namespace GetRequest {

        std::string_view EraseEmptySymbol(std::string_view line) {

            line.remove_prefix(std::min(line.find_first_not_of(" "), line.size()));

            auto it = std::prev(line.end());
            while(*it == ' ') {
                line.remove_suffix(std::distance(it, line.end()));
                it = std::prev(line.end());
            }

            return {line};
        }


        void ParseNumberBus(std::string_view line, TransportCatalogue &catalog) {
            std::string_view number_bus_ = EraseEmptySymbol(line);

            auto addr_bus = catalog.GetBusInfo(number_bus_);

            if( addr_bus ) {
                int count_stops = addr_bus->way.size();
                int count_unique_stop = addr_bus->unique_stop.size();
                double route_length = 0;
                int route_length_int = 0;

                auto vect_stops_ = addr_bus->way;
                for(auto it = vect_stops_.begin(); it != vect_stops_.end(); ++it) {
                    if( std::next(it) != vect_stops_.end() ) {
                        route_length += ComputeDistance((*it)->xy, (*std::next(it))->xy);
                        route_length_int += catalog.GetDistance(*it, *std::next(it));
                    }
                }
                if( addr_bus->circle ) {
                    count_stops = count_stops * 2 - 1;
                    for(auto it = vect_stops_.rbegin(); it != vect_stops_.rend(); ++it) {
                        if( std::next(it) != vect_stops_.rend() ) {
                            route_length += ComputeDistance((*it)->xy, (*std::next(it))->xy);
                            route_length_int += catalog.GetDistance(*it, *std::next(it));
                        }
                    }

                }

                std::cout << "Bus " << number_bus_ 
                            << ": " << count_stops 
                            << " stops on route, " 
                            << count_unique_stop 
                            <<  " unique stops, " 
                            << route_length_int
                            << " route length, "  
                            << std::setprecision(6) << static_cast<double>(route_length_int/route_length)
                            << " curvature" << std::endl;

                return;

            }


            std::cout << "Bus " << number_bus_ << ": not found" << std::endl;
        }

        static void ParseStop(std::string_view line, TransportCatalogue &catalog) {
            std::string_view stop_ = EraseEmptySymbol(line);

            if( catalog.FindStop(stop_) == nullptr) {
                std::cout << "Stop " << stop_ << ": not found" << std::endl;
                return;
            }

            auto number_buses_on_stop = catalog.GetBusesOnStop(stop_);

            if( number_buses_on_stop.size() ) {
                std::cout << "Stop " << stop_ << ": buses ";

                for( const auto name_bus_ : number_buses_on_stop ) {
                    std::cout << name_bus_ << " ";
                }
                std::cout << std::endl;
            } else {
                std::cout << "Stop " << stop_ << ": no buses" << std::endl;
            }

            
        }

        void ParseRequest(std::string_view line, TransportCatalogue &catalog) {
            size_t pos = EraseEmptySymbol(line).find(' ');

            std::string request = std::string(line.substr(0, pos));
            line.remove_prefix(pos + 1);

            if( request == "Bus" ) {
                ParseNumberBus(line, catalog);
            } else if( request == "Stop" ) {
                ParseStop(line, catalog);
            }

        }


        void GetRequest(TransportCatalogue &catalog, std::istream& input) {
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
                ParseRequest(line, catalog);
            }

        }
    }
}