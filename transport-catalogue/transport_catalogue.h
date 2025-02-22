#pragma once

#include <cassert>
#include <deque>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <optional>

#include "domain.h"

namespace transport_catalogue {

    using namespace domain;

    class TransportCatalogue {
    public:
        void AddStop(const Stop& stop);

        Stop* FindStop(const std::string_view& stop_name) const;

        void AddDistanceBetweenStops(Stop* stop1, Stop* stop2, int distance);

        int GetDistanceBetweenStops(Stop* stop1, Stop* stop2) const;

        void AddBus(const Bus& bus);

        Bus* FindBus(const std::string_view& bus_name) const;

        std::optional<BusInfo> GetBusInfo(const std::string_view& request) const;

        std::optional<std::set<std::string_view>> GetStopInfo(const std::string_view& request) const;

        const std::deque<Bus>& GetBuses() const;

    private:
        std::deque<Stop> stops_;
        std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
        std::unordered_map<std::pair<Stop*, Stop*>, int, PairHasher, PairEqual> distance_between_stops_;

        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, Bus*> busname_to_bus_;

        std::unordered_map<std::string_view, std::set<std::string_view>> stopname_to_busname_;

        std::size_t GetStopsOnRoute(const std::string_view& request) const;
        std::size_t GetUniqueStops(const std::string_view& request) const;
        int GetRouteLength(const std::string_view& request) const;
        double GetCurvature(const std::string_view& request) const;
    };

} // namespace transport_catalogue