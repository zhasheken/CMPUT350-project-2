#pragma once

#include "CollisionComponent.h"
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
        EntityManager<Components...>& entities = engine->GetEntityManager();
        ComponentStorage<CollisionComponent>& collisions =
            entities.template GetComponentStorage<CollisionComponent>();

        // save boxes before anything moves
        for (size_t i = 0; i < collisions.Size(); i++) {
            if (!collisions.IsValid(i)) {
                continue;
            }

            EntityID owner = collisions.GetOwner(i);
            Point2D location = entities.template GetComponent<LocationComponent>(owner).loc;
            CollisionComponent& collision = collisions.Get(i);

            // store it in world coordinates
            collision.previousCollisionBox = collision.collisionBox;
            collision.previousCollisionBox.topLeft.x += location.x;
            collision.previousCollisionBox.topLeft.y += location.y;
        }
        return kContinue;
    }
};

}  // namespace ECSEngine
