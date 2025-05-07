#pragma once

#include <string>

namespace Vengine {

class IResource {
   public:
    virtual ~IResource() = default;
    virtual auto load(const std::string& fileName) -> bool = 0;
    virtual auto unload() -> bool = 0;

    [[nodiscard]] auto isLoaded() const -> bool {
        return m_isLoaded;
    }

   protected:
    bool m_isLoaded = false;
};

}  // namespace Vengine