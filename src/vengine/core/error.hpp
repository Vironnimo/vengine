#pragma once

#include <string>
#include <source_location>

namespace Vengine {

struct Error {
    std::string message;
    std::source_location location;

    Error(std::string msg, std::source_location loc = std::source_location::current())
        : message(std::move(msg)), location(loc) {}

    [[nodiscard]] auto toString() const -> std::string {
        return "Error: " + message + "\n"
               "Location: " + location.file_name() + ":" + std::to_string(location.line()) + "\n";
    }
};

}  // namespace Vengine