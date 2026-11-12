#pragma once

#include "EntityManager.h"
#include "LocationComponent.h"
#include "MovementComponent.h"
#include "SystemManager.h"

namespace ECSEngine {

template <typename... Components>
class MovementSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("MovementSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        EntityManager<Components...>& entities = engine->GetEntityManager();
        ComponentStorage<MovementComponent>& movements =
            entities.template GetComponentStorage<MovementComponent>();

        // move entities by their velocity
        for (size_t i = 0; i < movements.Size(); i++) {
            if (!movements.IsValid(i)) {
                continue;
            }

            EntityID owner = movements.GetOwner(i);
            if (!entities.template HasComponent<LocationComponent>(owner)) {
                continue;
            }

            // velocity is per frame
            Point2D velocity = movements.Get(i).velocity;
            LocationComponent& location = entities.template GetComponent<LocationComponent>(owner);
            location.loc.x += velocity.x;
            location.loc.y += velocity.y;
        }
        return kContinue;
    }
};

}  // namespace ECSEngine
