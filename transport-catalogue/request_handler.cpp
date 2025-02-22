#include "request_handler.h"

#include <algorithm>

namespace transport_catalogue {

    using namespace domain;

    const std::optional<BusInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
        return db_.GetBusInfo(bus_name);
    }

    const std::optional<std::set<std::string_view>> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
        return db_.GetStopInfo(stop_name);
    }

    svg::Document RequestHandler::RenderMap() const {
        auto buses = db_.GetBuses();
        std::sort(buses.begin(), buses.end(), [](const Bus& a, const Bus& b) 
            {
                return a.name < b.name;
            }
        );
        return renderer_.Render(buses);
    }

}  // namespace transport_catalogue