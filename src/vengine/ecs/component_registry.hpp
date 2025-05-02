#pragma once

#include <unordered_map>
#include <typeindex>
#include <string>
#include <bitset>
#include <spdlog/spdlog.h>

namespace Vengine {

// Maximum number of component types
constexpr size_t MAX_COMPONENTS = 32;
using ComponentBitset = std::bitset<MAX_COMPONENTS>;
using ComponentId = uint32_t;

class ComponentRegistry {
   public:
    ComponentRegistry() = default;

    template <typename T>
    auto registerComponent(const std::string& name = "") -> ComponentId {
        const std::type_index typeIdx = std::type_index(typeid(T));

        auto it = m_typeToId.find(typeIdx);
        if (it != m_typeToId.end()) {
            return it->second;
        }

        ComponentId id = m_nextComponentId++;
        if (id >= MAX_COMPONENTS) {
            spdlog::error("Maximum number of components exceeded");
            // todo real error handling
            return MAX_COMPONENTS;
        }

        m_typeToId.insert({typeIdx, id});
        m_idToType.insert({id, typeIdx});
        m_idToName[id] = name.empty() ? typeid(T).name() : name;

        return id;
    }

    template <typename T>
    auto getComponentId() const -> ComponentId {
        const std::type_index typeIdx = std::type_index(typeid(T));
        auto it = m_typeToId.find(typeIdx);

        if (it == m_typeToId.end()) {
            throw std::runtime_error("Component type not registered");
        }

        return it->second;
    }

    [[nodiscard]] auto getComponentName(ComponentId id) const -> std::string {
        auto it = m_idToName.find(id);
        if (it == m_idToName.end()) {
            return "Unknown";
        }
        return it->second;
    }

    auto hasComponent(ComponentId id) const -> bool {
        return m_idToName.find(id) != m_idToName.end();
    }

    auto size() const -> ComponentId {
        return m_nextComponentId;
    }

   private:
    ComponentId m_nextComponentId = 0;
    std::unordered_map<std::type_index, ComponentId> m_typeToId;
    std::unordered_map<ComponentId, std::type_index> m_idToType;
    std::unordered_map<ComponentId, std::string> m_idToName;
};

}  // namespace Vengine