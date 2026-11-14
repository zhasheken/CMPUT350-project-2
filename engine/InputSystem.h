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

inline constexpr float kMaxRunVelocity = 7.0f;
inline constexpr float kDashVelocity = 40.0f;
inline constexpr float kJumpVelocity = 18.0f;
inline constexpr float kGroundRunAcceleration = 4.0f;
inline constexpr float kAirRunAcceleration = 3.0f;
inline constexpr int kDashInterval = 60;
inline constexpr int kWallCoyoteFrames = 6;

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

    // both held, newest press wins
    if (left && right) {
        return input.lastHorizontalPress;
    }
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
    const std::string& GetName() const override {
        static std::string name("InputSystem");
        return name;
    }

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

            // remember the newest arrow press
            if (WasPressed(input, Key::Left)) {
                input.lastHorizontalPress = -1;
            }
            if (WasPressed(input, Key::Right)) {
                input.lastHorizontalPress = 1;
            }

            const bool onGround = collision != nullptr && collision->onGround;
            ApplyHorizontalInput(movement, input, onGround);

            // remember the wall briefly
            const bool onWall =
                collision != nullptr && (collision->onLeftWall || collision->onRightWall);
            if (onGround) {
                input.wallCoyote = 0;
            } else if (onWall && movement.velocity.y > 0.0f) {
                input.wallCoyote = kWallCoyoteFrames;
                if (collision->onLeftWall) {
                    input.wallSide = -1;
                } else {
                    input.wallSide = 1;
                }
            } else if (input.wallCoyote > 0) {
                --input.wallCoyote;
            }

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
                } else if (input.wallCoyote > 0) {
                    // push away from the wall
                    movement.velocity.y = -kJumpVelocity;
                    movement.velocity.x = static_cast<float>(-input.wallSide) * kMaxRunVelocity;
                    input.wallCoyote = 0;
                    engine->GetSoundManager().GetSound("jump").play();
                }
            }

            if (WasPressed(input, Key::C) && input.lastDash == 0) {
                movement.velocity.x =
                    static_cast<float>(DashDirection(input, movement)) * kDashVelocity;
                input.lastDash = kDashInterval;

                // stop falling when dashing
                if (movement.velocity.y > 0.0f) {
                    movement.velocity.y = 0.0f;
                }
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
