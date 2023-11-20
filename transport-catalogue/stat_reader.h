#pragma once
// напишите решение с нуля
// код сохраните в свой git-репозиторий


#include "transport_catalogue.h"
#include <sstream>
#include <iostream>

namespace Catalogue {
    namespace GetRequest {
        void GetRequest(TransportCatalogue &catalog, std::ostream& out = std::cout, std::istream& input = std::cin);
    }
}