#pragma once

#include <map>
#include <string>
#include <memory>
#include <tl/expected.hpp>
#include "vengine/core/error.hpp"

#include "material.hpp"

namespace Vengine {

class Materials {
   public:
    Materials();
    ~Materials();
    [[nodiscard]] auto init() -> tl::expected<void, Error>;
    [[nodiscard]] auto get(const std::string& name) -> std::shared_ptr<Material>;
    auto add(const std::string& name, std::shared_ptr<Material> material) -> void;

   private:
    std::map<std::string, std::shared_ptr<Material>> m_materials;
};

}  // namespace Vengine