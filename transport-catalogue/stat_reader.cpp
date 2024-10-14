#include <algorithm>
#include <string>

#include "stat_reader.h"

using namespace std::literals;

namespace transport_catalogue {

void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output) {
    auto command_begin = request.find_first_not_of(' ');
    auto space_pos = request.find(' ');
    auto not_space = request.find_first_not_of(' ', space_pos);

    std::string_view command = request.substr(command_begin, space_pos - command_begin);
    if (command == "Bus"s) {
        transport_catalogue.GetBusInfo(request.substr(not_space), output);
    } else {
        transport_catalogue.GetStopInfo(request.substr(not_space), output);
    }
}

} // namespace transport_catalogue