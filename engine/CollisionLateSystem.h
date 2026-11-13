#pragma once

#include "CollisionComponent.h"
#include "Entity.h"
#include "EntityManager.h"
#include "LocationComponent.h"
#include "SystemManager.h"

namespace ECSEngine {

template <typename... Components>
class CollisionLateSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("CollisionLateSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto& entities = engine->GetEntityManager();

        for (const Entity& entity : entities) {
            const EntityID id = entity.GetID();
            if (!entities.IsEntityIDValid(id) ||
                !entities.template HasComponent<CollisionComponent>(id) ||
                !entities.template HasComponent<LocationComponent>(id)) {
                continue;
            }

            const CollisionComponent& collision =
                entities.template GetComponent<CollisionComponent>(id);
            const LocationComponent& location =
                entities.template GetComponent<LocationComponent>(id);
            Rect previousBox = collision.collisionBox;
            previousBox.topLeft.x += location.loc.x;
            previousBox.topLeft.y += location.loc.y;
            entities.template GetComponent<CollisionComponent>(id).previousCollisionBox =
                previousBox;
        }

        return kContinue;
    }
};

}  // namespace ECSEngine
