#include "materials.hpp"

#include <tl/expected.hpp>
#include <utility>

#include "material.hpp"

namespace Vengine {

    [[nodiscard]] auto Materials::init() -> tl::expected<void, Error> {
        // return tl::unexpected(Error{"Materials: init() not implemented"});

        return {};
    }

    auto Materials::get(const std::string& name) -> std::shared_ptr<Material> {
        auto it = m_materials.find(name);
        if (it != m_materials.end()) {
            return it->second;
        }
        return nullptr; 
    }


    void Materials::add(const std::string& name, std::shared_ptr<Material> material) {
        m_materials[name] = std::move(material);
    }
}