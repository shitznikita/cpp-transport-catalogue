#include "transport_catalogue.h"

#include <utility>

using namespace std::literals;

namespace transport_catalogue {

void TransportCatalogue::AddStop(const Stop& stop) {
    stops_.push_back(stop);
    stopname_to_stop_[stops_.back().name] = &stops_.back();
}

const Stop* TransportCatalogue::FindStop(std::string_view stop_name) const {
    auto stop = stopname_to_stop_.find(stop_name);
    if (stop == stopname_to_stop_.end()) {
        return nullptr;
    }
    return stop->second;
}

void TransportCatalogue::AddDistanceBetweenStops(const Stop* from, const Stop* to, int distance) {
    distance_between_stops_[{from, to}] = distance;
}

int TransportCatalogue::GetDistanceBetweenStops(const Stop* from, const Stop* to) const {
    auto distance = distance_between_stops_.find({from, to});
    if (distance != distance_between_stops_.end()) {
        return distance->second;
    }
    return GetDistanceBetweenStops(to, from);
}

void TransportCatalogue::AddBus(const Bus& bus) {
    buses_.push_back(bus);
    busname_to_bus_[buses_.back().name] = &buses_.back();
    for (const auto& stop : buses_.back().stops) {
        stopname_to_busname_[stop->name].insert(buses_.back().name);
    }
}

const Bus* TransportCatalogue::FindBus(std::string_view bus_name) const {
    auto bus = busname_to_bus_.find(bus_name);
    if (bus == busname_to_bus_.end()) {
        return nullptr;
    }
    return bus->second;
}

std::size_t TransportCatalogue::GetStopsOnRoute(std::string_view request) const {
    return busname_to_bus_.at(request)->stops.size();
}

std::size_t TransportCatalogue::GetUniqueStops(std::string_view request) const {
    std::unordered_set<const Stop*> unique_stops;
    for (const auto& stop : busname_to_bus_.at(request)->stops) {
        unique_stops.insert(stop);
    }
    return unique_stops.size();
}

int TransportCatalogue::GetRouteLength(std::string_view request) const {
    int res = 0;
    auto& bus = busname_to_bus_.at(request);
    for (std::size_t i = 1; i < bus->stops.size(); ++i) {
        res += GetDistanceBetweenStops(bus->stops[i-1], bus->stops[i]);
    }
    return res;
}

double TransportCatalogue::GetCurvature(std::string_view request) const {
    double res = 0.0;
    auto& bus = busname_to_bus_.at(request);
    for (std::size_t i = 1; i < bus->stops.size(); ++i) {
        res += ComputeDistance(bus->stops[i-1]->coordinates, bus->stops[i]->coordinates);
    }
    return res;
}

BusInfo TransportCatalogue::GetBusInfo(std::string_view request) const {
    return BusInfo({GetStopsOnRoute(request), GetUniqueStops(request), GetRouteLength(request), GetRouteLength(request) / GetCurvature(request)});
}

const std::set<std::string_view>& TransportCatalogue::GetStopInfo(std::string_view request) const {
    if (stopname_to_busname_.count(request) == 0) {
        return empty_set_;
    }
    return stopname_to_busname_.at(request);
}

} // namespace transport_catalogue