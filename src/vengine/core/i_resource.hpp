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

    [[nodiscard]] virtual auto needsMainThreadInit() const -> bool {
        return m_needsMainThreadInit;
    }

    virtual auto finalizeOnMainThread() -> bool {
        if (!m_needsMainThreadInit) {
            return false;
        }
        m_needsMainThreadInit = false;
        return true;
    }

   protected:
    bool m_isLoaded = false;
    bool m_needsMainThreadInit = false;
};

}  // namespace Vengine