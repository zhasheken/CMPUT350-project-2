#pragma once

#include "CollisionComponent.h"
#include "EntityManager.h"
#include "GravityComponent.h"
#include "InputComponent.h"
#include "Keyboard.h"
#include "MovementComponent.h"
#include "SystemManager.h"

namespace ECSEngine {

const float kFallGravityMultiplier = 2.0f;
const float kFastFallGravityMultiplier = 3.0f;
const float kMaxFallVelocity = 30.0f;
const float kMaxFastFallVelocity = 45.0f;
const float kWallSlideVelocity = 1.0f;

template <typename... Components>
class GravitySystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("GravitySystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        EntityManager<Components...>& entities = engine->GetEntityManager();
        ComponentStorage<GravityComponent>& gravities =
            entities.template GetComponentStorage<GravityComponent>();

        // accelerate every entity with gravity
        for (size_t i = 0; i < gravities.Size(); i++) {
            if (!gravities.IsValid(i)) {
                continue;
            }

            EntityID owner = gravities.GetOwner(i);
            if (!entities.template HasComponent<MovementComponent>(owner)) {
                continue;
            }

            Point2D acceleration = gravities.Get(i).acceleration;
            MovementComponent& movement = entities.template GetComponent<MovementComponent>(owner);
            bool isFalling = movement.velocity.y > 0;
            bool hasInput = entities.template HasComponent<InputComponent>(owner);

            // fall faster than rising
            float multiplier = 1.0f;
            float maxFallVelocity = kMaxFallVelocity;
            if (isFalling) {
                multiplier = kFallGravityMultiplier;
            }

            // holding down falls even faster
            if (isFalling && hasInput) {
                InputComponent& input = entities.template GetComponent<InputComponent>(owner);
                if (input.keydown[Key::Down]) {
                    multiplier = kFastFallGravityMultiplier;
                    maxFallVelocity = kMaxFastFallVelocity;
                }
            }

            movement.velocity.x += acceleration.x;
            movement.velocity.y += acceleration.y * multiplier;

            // cap the falling speed
            if (movement.velocity.y > maxFallVelocity) {
                movement.velocity.y = maxFallVelocity;
            }

            // slide slowly down walls
            if (hasInput && entities.template HasComponent<CollisionComponent>(owner)) {
                CollisionComponent& collision =
                    entities.template GetComponent<CollisionComponent>(owner);
                bool onWall = collision.onLeftWall || collision.onRightWall;
                if (onWall && movement.velocity.y > kWallSlideVelocity) {
                    movement.velocity.y = kWallSlideVelocity;
                }
            }
        }
        return kContinue;
    }
};

}  // namespace ECSEngine
