#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "geo.h"
#include "transport_catalogue.h"

using namespace std::literals;

namespace transport_catalogue {

enum class CommandType { BUS, STOP, UNKNOWN };

CommandType GetCommandType(std::string_view command);

struct CommandDescription {
    explicit operator bool() const {
        return !command.empty();
    }

    bool operator!() const {
        return !operator bool();
    }

    std::string command;
    std::string id;
    std::string description;
};

class InputReader {
public:
    void ParseLine(std::string_view line);

    void ApplyCommands(TransportCatalogue& catalogue) const;

private:
    std::vector<CommandDescription> commands_;
};

} // namespace transport_catalogue