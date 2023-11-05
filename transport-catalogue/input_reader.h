#pragma once
// напишите решение с нуля
// код сохраните в свой git-репозиторий


#include <vector>
#include <string>
#include <string_view>
#include <sstream>
#include <iostream>

#include "transport_catalogue.h"
namespace Catalogue {
    namespace InputRequest {
        void AddRequest(TransportCatalogue &catalog, std::istream& input = std::cin);
    }
}