#include <iostream>

#include "transport_catalogue.h"

using namespace std::literals;

namespace transport_catalogue {

void TransportCatalogue::AddStop(const Stop& stop) {
    stops_.push_back(stop);
    stopname_to_stop_[stops_.back().name] = &stops_.back();
}

TransportCatalogue::Stop* TransportCatalogue::FindStop(const std::string_view& stop_name) {
    assert(stopname_to_stop_.count(stop_name) == 1);
    return stopname_to_stop_.at(stop_name);
}

void TransportCatalogue::AddBus(const Bus& bus) {
    buses_.push_back(bus);
    busname_to_bus_[buses_.back().name] = &buses_.back();
    for (const auto& stop : buses_.back().stops) {
        stopname_to_busname_[stop->name].insert(buses_.back().name);
    }
}

TransportCatalogue::Bus* TransportCatalogue::FindBus(const std::string_view& bus_name) {
    assert(busname_to_bus_.count(bus_name) == 1);
    return busname_to_bus_.at(bus_name);
}

std::size_t TransportCatalogue::GetStopsOnRoute(const std::string_view& request) const {
    return busname_to_bus_.at(request)->stops.size();
}

std::size_t TransportCatalogue::GetUniqueStops(const std::string_view& request) const {
    std::unordered_set<Stop*> unique_stops;
    for (const auto& stop : busname_to_bus_.at(request)->stops) {
        unique_stops.insert(stop);
    }
    return unique_stops.size();
}

double TransportCatalogue::GetRouteLength(const std::string_view& request) const {
    double res = 0.0;
    auto& bus = busname_to_bus_.at(request);
    for (std::size_t i = 1; i < bus->stops.size(); ++i) {
        res += ComputeDistance(bus->stops[i-1]->coordinates, bus->stops[i]->coordinates);
    }
    return res;
}

void TransportCatalogue::GetBusInfo(const std::string_view& request, std::ostream& output) const {
    if (busname_to_bus_.count(request) == 0) {
        output << "Bus "s << request << ": not found"s << std::endl;
    } else {
        output << "Bus "s << request << ": "s << GetStopsOnRoute(request) << " stops on route, "s << GetUniqueStops(request) << " unique stops, "s
               << GetRouteLength(request) << " route length"s << std::endl;
    }
}

void TransportCatalogue::GetStopInfo(const std::string_view& request, std::ostream& output) const {
    if (stopname_to_stop_.count(request) == 0) {
        output << "Stop "s << request << ": not found"s << std::endl;
    } else if (stopname_to_busname_.count(request) == 0) {
        output << "Stop "s << request << ": no buses"s << std::endl;
    } else {
        output << "Stop "s << request << ": buses"s;
        for (const auto& bus_name : stopname_to_busname_.at(request)) {
            output << " "s << bus_name;
        }
        output << std::endl;
    }
}

} // namespace transport_catalogue