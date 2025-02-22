#pragma once

#include "transport_catalogue.h"
#include "json.h"
#include "request_handler.h"
#include "map_renderer.h"

namespace json_reader {

    using namespace transport_catalogue;

    class JsonReader {
    public:
        JsonReader(std::istream& input);

        void ApplyCommands(TransportCatalogue& catalogue) const;
        void ApplySettingsCommands(renderer::MapRenderer& renderer) const;
        void PrintJson(const RequestHandler& request_handler, std::ostream& out) const;

    private:
        const json::Document doc_;

        const json::Array* request_commands_;
        const json::Array* stat_commands_;
        const json::Dict* render_settings_;
    };

}  // namespace json_reader