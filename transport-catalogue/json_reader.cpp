#include "json_reader.h"

#include <string>
#include <sstream>
#include <utility>

namespace json_reader {

    using namespace std::literals;
    using namespace domain;
    using namespace geo;

namespace {

    std::string GetColor(const json::Node& value) {
        std::stringstream res;
        if (value.IsString()) {
            res << value.AsString();
        } else {
            const auto& color = value.AsArray();
            if (color.size() == 3) {
                res << "rgb("
                    << color[0].AsInt() << ','
                    << color[1].AsInt() << ','
                    << color[2].AsInt() << ')';
            } else {
                res << "rgba("
                    << color[0].AsInt() << ','
                    << color[1].AsInt() << ','
                    << color[2].AsInt() << ','
                    << color[3].AsDouble() << ')';
            }
        }
        return res.str();
    }

}  // namespace

    JsonReader::JsonReader (std::istream& input) :
        doc_(json::Load(input))
    {
        const auto& root = doc_.GetRoot();
        for (const auto& dict : root.AsMap()) {
            if (dict.first == "base_requests") {
                request_commands_ = &dict.second.AsArray();
            } else if (dict.first == "stat_requests") {
                stat_commands_ = &dict.second.AsArray();
            } else if (dict.first == "render_settings") {
                render_settings_ = &dict.second.AsMap();
            }
        }
    }

    void JsonReader::ApplyCommands(TransportCatalogue& catalogue) const {
        json::Array only_stop_commands;
        json::Array only_bus_commands;

        for (const auto& command : *request_commands_) {
            const auto& description = command.AsMap();
            const auto& type = description.at("type").AsString();
            if (type == "Stop") {
                catalogue.AddStop({description.at("name").AsString(),
                                  Coordinates{description.at("latitude").AsDouble(), 
                                              description.at("longitude").AsDouble()}});
                only_stop_commands.push_back(std::move(command));
            } else if (type == "Bus") {
                only_bus_commands.push_back(std::move(command));
            }
        }

        for (const auto& command : only_stop_commands) {
            const auto& description = command.AsMap();
            const auto& name = description.at("name").AsString();
            const auto& road_distances = description.at("road_distances").AsMap();
            if (!road_distances.empty()) {
                for (const auto& distance : road_distances) {
                    catalogue.AddDistanceBetweenStops(
                        catalogue.FindStop(name), catalogue.FindStop(distance.first), distance.second.AsInt()
                    );
                }
            }
        }

        for (const auto& command : only_bus_commands) {
            const auto& description = command.AsMap();
            const auto& route = description.at("stops").AsArray();
            std::vector<Stop*> stops;
            for (const auto& stop : route) {
                stops.push_back(catalogue.FindStop(stop.AsString()));
            }
            bool is_roundtrip = description.at("is_roundtrip").AsBool();
            if (!is_roundtrip) {
                if (route.size() > 1) {
                    for (auto it = route.rbegin() + 1; it != route.rend(); ++it) {
                        const auto& stop = *it;
                        stops.push_back(catalogue.FindStop(stop.AsString()));
                    }
                }
            }
            catalogue.AddBus({description.at("name").AsString(),
                              std::move(stops),
                              is_roundtrip});
        }
    }

    void JsonReader::ApplySettingsCommands(renderer::MapRenderer& renderer) const {
        renderer::RenderSettings settings;
        for (const auto& [key, value] : *render_settings_) {
            if (key == "width") {
                settings.width = value.AsDouble();
            } else if (key == "height") {
                settings.height = value.AsDouble();
            } else if (key == "padding") {
                settings.padding = value.AsDouble();
            } else if (key == "line_width") {
                settings.line_width = value.AsDouble();
            } else if (key == "stop_radius") {
                settings.stop_radius = value.AsDouble();
            } else if (key == "bus_label_font_size") {
                settings.bus_label_font_size = value.AsInt();
            } else if (key == "bus_label_offset") {
                const auto& offset = value.AsArray();
                if (offset.size() == 2) {
                    settings.bus_label_offset[0] = offset[0].AsDouble();
                    settings.bus_label_offset[1] = offset[1].AsDouble();
                }
            } else if (key == "stop_label_font_size") {
                settings.stop_label_font_size = value.AsInt();
            } else if (key == "stop_label_offset") {
                auto offset = value.AsArray();
                if (offset.size() == 2) {
                    settings.stop_label_offset[0] = offset[0].AsDouble();
                    settings.stop_label_offset[1] = offset[1].AsDouble();
                }
            } else if (key == "underlayer_color") {
                settings.underlayer_color = GetColor(value);
            } else if (key == "underlayer_width") {
                settings.underlayer_width = value.AsDouble();
            } else if (key == "color_palette") {
                const auto& colors = value.AsArray();
                for (const auto& color : colors) {
                    settings.color_palette.push_back(GetColor(color));
                }
            }
        }
        renderer.SetSettings(settings);
    }

    void JsonReader::PrintJson(const RequestHandler& request_handler, std::ostream& out) const {
        json::Array res;
        for (const auto& command : *stat_commands_) {
            const auto& description = command.AsMap();
            const auto& type = description.at("type").AsString();
            if (type == "Bus") {
                json::Dict bus;
                const auto& bus_info = request_handler.GetBusStat(description.at("name").AsString());
                bus.insert({"request_id", description.at("id").AsInt()});
                if (bus_info.has_value()) {
                    bus.insert({"curvature", bus_info->curvature});
                    bus.insert({"route_length", bus_info->route_length});
                    bus.insert({"stop_count", static_cast<int>(bus_info->stops_on_route)});
                    bus.insert({"unique_stop_count", static_cast<int>(bus_info->unique_stops)});
                } else {
                    bus.insert({"error_message", std::string("not found")});
                }
                res.push_back(bus);
            } else if (type == "Stop") {
                json::Dict stop;
                const auto& stop_info = request_handler.GetBusesByStop(description.at("name").AsString());
                stop.insert({"request_id", description.at("id").AsInt()});
                if (stop_info.has_value()) {
                    if (!stop_info->empty()) {
                        json::Array buses;
                        for (const auto& bus_name : *stop_info) {
                            buses.push_back(std::string(bus_name));
                        }
                        stop.insert({"buses", buses});
                    } else {
                        json::Array buses;
                        stop.insert({"buses", buses});
                    }
                } else {
                    stop.insert({"error_message", std::string("not found")});
                }
                res.push_back(stop);
            } else if (type == "Map") {
                json::Dict map;
                std::ostringstream map_out;
                request_handler.RenderMap().Render(map_out);
                map.insert({"map", map_out.str()});
                map.insert({"request_id", description.at("id").AsInt()});
                res.push_back(map);
            }
        }
        json::Document doc(res);
        json::Print(doc, out);
    }

}  // namespace json_reader