#include "entities.hpp"

#include "vengine/ecs/entity.hpp"

namespace Vengine {

auto Entities::getEntity(EntityId entity) -> Entity {
    return {entity, this};
}

auto Entities::getEntityByTag(const std::string& tag) -> Entity {
    auto taggedEntities = getEntitiesWith<TagComponent>();

    for (auto entityId : taggedEntities) {
        auto tagComponent = getEntityComponent<TagComponent>(entityId);
        if (tagComponent && tagComponent->tag == tag) {
            return {entityId, this};
        }
    }

    return {};
}

}  // namespace Vengine