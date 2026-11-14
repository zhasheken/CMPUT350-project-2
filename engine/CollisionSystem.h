#pragma once

#include <algorithm>
#include <cmath>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

#include "CameraComponent.h"
#include "CameraShakeComponent.h"
#include "CollisionComponent.h"
#include "Entity.h"
#include "EntityManager.h"
#include "InputComponent.h"
#include "LocationComponent.h"
#include "MovementComponent.h"
#include "RespawnComponent.h"
#include "ScoreComponent.h"
#include "SystemManager.h"

namespace ECSEngine {

// AI-assisted static-collision and star-interaction implementation for issues #23/#24;
// see ABOUT.TXT.
namespace CollisionSystemDetail {

constexpr float kPositionEpsilon = 0.001f;
constexpr float kLandingImpactThreshold = 1.0f;
constexpr float kWallImpactThreshold = 5.0f;
constexpr float kShakePerSpeed = 0.1f;
constexpr float kMaximumShake = 12.0f;
constexpr float kSpikeShake = 10.0f;
constexpr float kStarRestThreshold = 1.0f;
constexpr float kStarRestitution = 0.7f;
constexpr int kShakeFrameLimit = 5;

enum class Side { top, bottom, left, right };
enum class Axis { horizontal, vertical };

struct Resolution {
    Side side;
    Point2D correction;
};

struct PairResolution {
    Axis axis;
    Point2D correction;
};

inline Rect WorldBox(const CollisionComponent& collision, const LocationComponent& location) {
    Rect box = collision.collisionBox;
    box.topLeft.x += location.loc.x;
    box.topLeft.y += location.loc.y;
    return box;
}

inline bool Overlaps(const Rect& first, const Rect& second) {
    return first.topLeft.x < second.topLeft.x + second.width &&
           first.topLeft.x + first.width > second.topLeft.x &&
           first.topLeft.y < second.topLeft.y + second.height &&
           first.topLeft.y + first.height > second.topLeft.y;
}

inline Side FindCollisionSide(const Rect& previous, const Rect& current, const Rect& obstacle) {
    if (previous.topLeft.y + previous.height <= obstacle.topLeft.y + kPositionEpsilon) {
        return Side::top;
    }
    if (previous.topLeft.y >= obstacle.topLeft.y + obstacle.height - kPositionEpsilon) {
        return Side::bottom;
    }
    if (previous.topLeft.x + previous.width <= obstacle.topLeft.x + kPositionEpsilon) {
        return Side::left;
    }
    if (previous.topLeft.x >= obstacle.topLeft.x + obstacle.width - kPositionEpsilon) {
        return Side::right;
    }

    const float pushUp = current.topLeft.y + current.height - obstacle.topLeft.y;
    const float pushDown = obstacle.topLeft.y + obstacle.height - current.topLeft.y;
    const float pushLeft = current.topLeft.x + current.width - obstacle.topLeft.x;
    const float pushRight = obstacle.topLeft.x + obstacle.width - current.topLeft.x;
    const float verticalPush = std::min(pushUp, pushDown);
    const float horizontalPush = std::min(pushLeft, pushRight);

    if (verticalPush <= horizontalPush) {
        return pushUp <= pushDown ? Side::top : Side::bottom;
    }
    return pushLeft <= pushRight ? Side::left : Side::right;
}

inline Resolution ResolveOverlap(const Rect& previous, const Rect& current,
                                 const Rect& obstacle) {
    const Side side = FindCollisionSide(previous, current, obstacle);
    Point2D correction{};

    switch (side) {
        case Side::top:
            correction.y = obstacle.topLeft.y - (current.topLeft.y + current.height);
            break;
        case Side::bottom:
            correction.y = obstacle.topLeft.y + obstacle.height - current.topLeft.y;
            break;
        case Side::left:
            correction.x = obstacle.topLeft.x - (current.topLeft.x + current.width);
            break;
        case Side::right:
            correction.x = obstacle.topLeft.x + obstacle.width - current.topLeft.x;
            break;
    }

    return {side, correction};
}

inline float ShakeMagnitude(float impactSpeed) {
    return std::min(impactSpeed * kShakePerSpeed, kMaximumShake);
}

inline PairResolution ResolveStarOverlap(const Rect& first, const Rect& second,
                                         EntityID firstID, EntityID secondID) {
    const float overlapX = std::min(first.topLeft.x + first.width, second.topLeft.x + second.width) -
                           std::max(first.topLeft.x, second.topLeft.x);
    const float overlapY =
        std::min(first.topLeft.y + first.height, second.topLeft.y + second.height) -
        std::max(first.topLeft.y, second.topLeft.y);
    const bool separateHorizontally = overlapX < overlapY;
    const float firstCenter =
        separateHorizontally ? first.topLeft.x + first.width / 2.0f
                             : first.topLeft.y + first.height / 2.0f;
    const float secondCenter =
        separateHorizontally ? second.topLeft.x + second.width / 2.0f
                             : second.topLeft.y + second.height / 2.0f;
    const bool firstIsBefore = firstCenter == secondCenter ? firstID < secondID
                                                            : firstCenter < secondCenter;
    const float separation =
        (separateHorizontally ? overlapX : overlapY) / 2.0f + kPositionEpsilon / 2.0f;
    const float direction = firstIsBefore ? -1.0f : 1.0f;
    Point2D correction{};
    if (separateHorizontally) {
        correction.x = direction * separation;
    } else {
        correction.y = direction * separation;
    }

    return {separateHorizontally ? Axis::horizontal : Axis::vertical, correction};
}

inline float BounceVelocity(float velocity) {
    const float reflectedVelocity = -velocity * kStarRestitution;
    return std::abs(reflectedVelocity) < kStarRestThreshold ? 0.0f : reflectedVelocity;
}

}  // namespace CollisionSystemDetail

template <typename... Components>
class CollisionSystem : public System<Components...> {
public:
    // Returns this system's stable name.
    const std::string& GetName() const override {
        static std::string name("CollisionSystem");
        return name;
    }

    // Resolves dynamic collisions, star pairs, player collection, score, and feedback.
    tSystemReturn Run(EngineView<Components...>* engine) override {
        using namespace CollisionSystemDetail;

        auto& entities = engine->GetEntityManager();
        std::vector<EntityID> staticEntities;
        std::vector<EntityID> stars;
        std::vector<EntityID> players;
        std::optional<EntityID> cameraEntity;
        std::optional<EntityID> scoreEntity;

        for (const Entity& entity : entities) {
            const EntityID id = entity.GetID();
            if (!entities.IsEntityIDValid(id)) {
                continue;
            }

            if (entities.template HasComponent<CameraComponent>(id)) {
                cameraEntity = id;
            }

            if (!scoreEntity.has_value() &&
                entities.template HasComponent<ScoreComponent>(id)) {
                scoreEntity = id;
            }

            if (entities.template HasComponent<CollisionComponent>(id) &&
                entities.template HasComponent<LocationComponent>(id) &&
                entities.template GetComponent<CollisionComponent>(id).staticEntity) {
                staticEntities.push_back(id);
            }

            if (entities.template HasComponent<CollisionComponent>(id) &&
                entities.template HasComponent<LocationComponent>(id) &&
                entities.template HasComponent<MovementComponent>(id) &&
                !entities.template GetComponent<CollisionComponent>(id).staticEntity) {
                if (entities.template HasComponent<InputComponent>(id)) {
                    players.push_back(id);
                } else {
                    stars.push_back(id);
                }
            }
        }

        for (size_t firstIndex = 0; firstIndex < stars.size(); ++firstIndex) {
            const EntityID firstID = stars[firstIndex];
            if (!entities.IsEntityIDValid(firstID)) {
                continue;
            }

            for (size_t secondIndex = firstIndex + 1; secondIndex < stars.size();
                 ++secondIndex) {
                const EntityID secondID = stars[secondIndex];
                if (!entities.IsEntityIDValid(secondID)) {
                    continue;
                }

                CollisionComponent& firstCollision =
                    entities.template GetComponent<CollisionComponent>(firstID);
                LocationComponent& firstLocation =
                    entities.template GetComponent<LocationComponent>(firstID);
                MovementComponent& firstMovement =
                    entities.template GetComponent<MovementComponent>(firstID);
                CollisionComponent& secondCollision =
                    entities.template GetComponent<CollisionComponent>(secondID);
                LocationComponent& secondLocation =
                    entities.template GetComponent<LocationComponent>(secondID);
                MovementComponent& secondMovement =
                    entities.template GetComponent<MovementComponent>(secondID);

                const Rect firstBox = WorldBox(firstCollision, firstLocation);
                const Rect secondBox = WorldBox(secondCollision, secondLocation);
                if (!Overlaps(firstBox, secondBox)) {
                    continue;
                }

                const PairResolution resolution =
                    ResolveStarOverlap(firstBox, secondBox, firstID, secondID);
                firstLocation.loc.x += resolution.correction.x;
                firstLocation.loc.y += resolution.correction.y;
                secondLocation.loc.x -= resolution.correction.x;
                secondLocation.loc.y -= resolution.correction.y;
                if (resolution.axis == Axis::horizontal) {
                    std::swap(firstMovement.velocity.x, secondMovement.velocity.x);
                } else {
                    std::swap(firstMovement.velocity.y, secondMovement.velocity.y);
                }
            }
        }

        float shakeX = 0.0f;
        float shakeY = 0.0f;
        bool shouldShake = false;

        for (const Entity& entity : entities) {
            const EntityID id = entity.GetID();
            if (!entities.IsEntityIDValid(id) ||
                !entities.template HasComponent<CollisionComponent>(id) ||
                !entities.template HasComponent<LocationComponent>(id) ||
                !entities.template HasComponent<MovementComponent>(id)) {
                continue;
            }

            CollisionComponent& collision =
                entities.template GetComponent<CollisionComponent>(id);
            if (collision.staticEntity) {
                continue;
            }

            LocationComponent& location =
                entities.template GetComponent<LocationComponent>(id);
            MovementComponent& movement =
                entities.template GetComponent<MovementComponent>(id);
            const bool wasOnGround = collision.onGround;
            const bool wasOnLeftWall = collision.onLeftWall;
            const bool wasOnRightWall = collision.onRightWall;
            const bool isPlayer = entities.template HasComponent<InputComponent>(id);
            const Point2D impactVelocity = movement.velocity;

            collision.onGround = 0;
            collision.onCeiling = 0;
            collision.onLeftWall = 0;
            collision.onRightWall = 0;
            bool respawned = false;

            for (const EntityID staticID : staticEntities) {
                if (!entities.IsEntityIDValid(staticID)) {
                    continue;
                }

                const CollisionComponent& staticCollision =
                    entities.template GetComponent<CollisionComponent>(staticID);
                const LocationComponent& staticLocation =
                    entities.template GetComponent<LocationComponent>(staticID);
                const Rect obstacle = WorldBox(staticCollision, staticLocation);
                const Rect current = WorldBox(collision, location);
                if (!Overlaps(current, obstacle)) {
                    continue;
                }

                // spikes send the player back
                if (isPlayer && staticCollision.doesDamage &&
                    entities.template HasComponent<RespawnComponent>(id)) {
                    // sound and shake on hit
                    engine->GetSoundManager().GetSound("spike").play();
                    shakeX += kSpikeShake;
                    shakeY += kSpikeShake;
                    shouldShake = true;

                    location.loc = entities.template GetComponent<RespawnComponent>(id).loc;
                    movement.velocity = Point2D{};
                    collision.onGround = 0;
                    collision.onCeiling = 0;
                    collision.onLeftWall = 0;
                    collision.onRightWall = 0;
                    respawned = true;
                    break;
                }

                const Resolution resolution =
                    ResolveOverlap(collision.previousCollisionBox, current, obstacle);
                location.loc.x += resolution.correction.x;
                location.loc.y += resolution.correction.y;
                switch (resolution.side) {
                    case Side::top:
                        movement.velocity.y =
                            isPlayer ? 0.0f : BounceVelocity(movement.velocity.y);
                        collision.onGround = 1;
                        break;
                    case Side::bottom:
                        movement.velocity.y =
                            isPlayer ? 0.0f : BounceVelocity(movement.velocity.y);
                        collision.onCeiling = 1;
                        break;
                    case Side::left:
                        movement.velocity.x =
                            isPlayer ? 0.0f : BounceVelocity(movement.velocity.x);
                        collision.onRightWall = 1;
                        break;
                    case Side::right:
                        movement.velocity.x =
                            isPlayer ? 0.0f : BounceVelocity(movement.velocity.x);
                        collision.onLeftWall = 1;
                        break;
                }
            }

            // no landing effects after respawn
            if (respawned) {
                continue;
            }

            if (isPlayer && collision.onGround && !wasOnGround &&
                impactVelocity.y > kLandingImpactThreshold) {
                engine->GetSoundManager().GetSound("land").play();
                shakeY += ShakeMagnitude(impactVelocity.y);
                shouldShake = true;
            }

            if (isPlayer &&
                ((collision.onLeftWall && !wasOnLeftWall) ||
                 (collision.onRightWall && !wasOnRightWall)) &&
                std::abs(impactVelocity.x) > kWallImpactThreshold) {
                shakeX += std::copysign(ShakeMagnitude(std::abs(impactVelocity.x)),
                                        impactVelocity.x);
                shouldShake = true;
            }
        }

        std::vector<EntityID> toRemove;
        for (const EntityID playerID : players) {
            if (!entities.IsEntityIDValid(playerID)) {
                continue;
            }

            const CollisionComponent& playerCollision =
                entities.template GetComponent<CollisionComponent>(playerID);
            const LocationComponent& playerLocation =
                entities.template GetComponent<LocationComponent>(playerID);
            const Rect playerBox = WorldBox(playerCollision, playerLocation);
            for (const EntityID starID : stars) {
                if (!entities.IsEntityIDValid(starID) ||
                    std::find(toRemove.begin(), toRemove.end(), starID) != toRemove.end()) {
                    continue;
                }

                const CollisionComponent& starCollision =
                    entities.template GetComponent<CollisionComponent>(starID);
                const LocationComponent& starLocation =
                    entities.template GetComponent<LocationComponent>(starID);
                if (!Overlaps(playerBox, WorldBox(starCollision, starLocation))) {
                    continue;
                }

                if (!scoreEntity.has_value()) {
                    throw std::logic_error("cannot collect a star without a ScoreComponent");
                }
                auto& starSound = engine->GetSoundManager().GetSound("star");
                toRemove.push_back(starID);
                entities.template GetComponent<ScoreComponent>(*scoreEntity).score++;
                starSound.play();
            }
        }

        if (shouldShake && cameraEntity.has_value()) {
            if (!entities.template HasComponent<CameraShakeComponent>(*cameraEntity)) {
                entities.template AddComponent<CameraShakeComponent>(
                    *cameraEntity, CameraShakeComponent{});
            }

            CameraShakeComponent& shake =
                entities.template GetComponent<CameraShakeComponent>(*cameraEntity);
            shake.frameLimit = kShakeFrameLimit;
            shake.offset.x = shakeX;
            shake.offset.y = shakeY;
        }

        for (const EntityID id : toRemove) {
            if (entities.IsEntityIDValid(id)) {
                entities.RemoveEntity(id);
            }
        }

        return kContinue;
    }
};

}  // namespace ECSEngine
