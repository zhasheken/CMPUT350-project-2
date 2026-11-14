#pragma once

#include <algorithm>

#include "CollisionComponent.h"
#include "Entity.h"
#include "EntityManager.h"
#include "InputComponent.h"
#include "Keyboard.h"
#include "MovementComponent.h"
#include "SoundManager.h"
#include "SystemManager.h"

namespace ECSEngine {

// AI-assisted control implementation for issue #22; see ABOUT.TXT.
inline constexpr float kMaxRunVelocity = 15.0f;
inline constexpr float kDashVelocity = 50.0f;
inline constexpr float kJumpVelocity = 25.0f;
inline constexpr float kGroundRunAcceleration = 5.0f;
inline constexpr float kAirRunAcceleration = 4.0f;
inline constexpr int kDashInterval = 60;

namespace InputSystemDetail {

inline float Approach(float value, float target, float amount) {
    if (value < target) {
        return std::min(value + amount, target);
    }
    return std::max(value - amount, target);
}

inline int HorizontalDirection(const InputComponent& input) {
    const bool left = input.keydown[Key::Left];
    const bool right = input.keydown[Key::Right];
    return static_cast<int>(right) - static_cast<int>(left);
}

inline void ApplyHorizontalInput(MovementComponent& movement, const InputComponent& input,
                                 bool onGround) {
    const float acceleration = onGround ? kGroundRunAcceleration : kAirRunAcceleration;
    const int direction = HorizontalDirection(input);
    const float targetVelocity =
        static_cast<float>(direction) * kMaxRunVelocity;
    movement.velocity.x = Approach(movement.velocity.x, targetVelocity, acceleration);
}

inline bool WasPressed(const InputComponent& input, unsigned int key) {
    return input.keydown[key] && !input.lastKeydown[key];
}

inline bool WasReleased(const InputComponent& input, unsigned int key) {
    return !input.keydown[key] && input.lastKeydown[key];
}

inline int DashDirection(const InputComponent& input, const MovementComponent& movement) {
    const int inputDirection = HorizontalDirection(input);
    if (inputDirection != 0) {
        return inputDirection;
    }
    return movement.velocity.x < 0.0f ? -1 : 1;
}

}  // namespace InputSystemDetail

template <typename... Components>
class InputSystem : public System<Components...> {
public:
    // Returns this system's stable name.
    const std::string& GetName() const override {
        static std::string name("InputSystem");
        return name;
    }

    // Applies movement, jump, wall-jump, dash, and wall-slide input for one frame.
    tSystemReturn Run(EngineView<Components...>* engine) override {
        using namespace InputSystemDetail;

        auto& entities = engine->GetEntityManager();
        for (const Entity& entity : entities) {
            const EntityID id = entity.GetID();
            if (!entities.IsEntityIDValid(id) ||
                !entities.template HasComponent<InputComponent>(id) ||
                !entities.template HasComponent<MovementComponent>(id)) {
                continue;
            }

            InputComponent& input = entities.template GetComponent<InputComponent>(id);
            MovementComponent& movement = entities.template GetComponent<MovementComponent>(id);
            CollisionComponent* collision = nullptr;
            if (entities.template HasComponent<CollisionComponent>(id)) {
                collision = &entities.template GetComponent<CollisionComponent>(id);
            }

            const bool onGround = collision != nullptr && collision->onGround;
            ApplyHorizontalInput(movement, input, onGround);

            if (input.lastDash > 0) {
                --input.lastDash;
            }

            if (WasReleased(input, Key::Z) && movement.velocity.y < 0.0f) {
                movement.velocity.y = 0.0f;
            }

            if (WasPressed(input, Key::Z) && collision != nullptr) {
                if (collision->onGround) {
                    movement.velocity.y = -kJumpVelocity;
                    engine->GetSoundManager().GetSound("jump").play();
                } else if (movement.velocity.y > 0.0f &&
                           (collision->onLeftWall || collision->onRightWall)) {
                    movement.velocity.y = -kJumpVelocity;
                    movement.velocity.x =
                        collision->onLeftWall ? kMaxRunVelocity : -kMaxRunVelocity;
                    engine->GetSoundManager().GetSound("jump").play();
                }
            }

            if (WasPressed(input, Key::C) && input.lastDash == 0) {
                movement.velocity.x =
                    static_cast<float>(DashDirection(input, movement)) * kDashVelocity;
                input.lastDash = kDashInterval;
            }

            const bool isWallSliding =
                collision != nullptr && movement.velocity.y > 0.0f &&
                (collision->onLeftWall || collision->onRightWall);
            if (isWallSliding && !input.wallSliding) {
                engine->GetSoundManager().GetSound("wallSlide").play();
            }
            input.wallSliding = isWallSliding;
        }

        return kContinue;
    }
};

}  // namespace ECSEngine
