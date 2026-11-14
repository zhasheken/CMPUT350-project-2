#pragma once

#include <cmath>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "CollisionComponent.h"
#include "Entity.h"
#include "EntityManager.h"
#include "GravityComponent.h"
#include "LocationComponent.h"
#include "MovementComponent.h"
#include "SpawnComponent.h"
#include "SpriteComponent.h"
#include "SystemManager.h"

namespace ECSEngine {

namespace SpawnSystemDetail {

inline constexpr const char* kStarSpawnType = "star";
inline constexpr float kStarSpeed = 2.0f;
inline constexpr float kPi = 3.14159265358979323846f;

struct SpawnRequest {
    EntityID spawnerID;
    SpriteID textureID;
};

template <typename... Components>
bool SpawnStar(EntityManager<Components...>& entities, const SpawnRequest& request,
               std::mt19937& randomGenerator, float starWidth, float starHeight) {
    if (!entities.IsEntityIDValid(request.spawnerID) ||
        !entities.template HasComponent<LocationComponent>(request.spawnerID)) {
        std::cerr << "Cannot spawn star: spawner has no valid location\n";
        return false;
    }
    if (starWidth <= 0.0f || starHeight <= 0.0f) {
        std::cerr << "Cannot spawn star: star texture has empty bounds\n";
        return false;
    }

    const LocationComponent& spawnerLocation =
        entities.template GetComponent<LocationComponent>(request.spawnerID);
    Rect spawnBounds{};
    if (entities.template HasComponent<CollisionComponent>(request.spawnerID)) {
        const CollisionComponent& collision =
            entities.template GetComponent<CollisionComponent>(request.spawnerID);
        spawnBounds = collision.collisionBox;
    } else if (entities.template HasComponent<SpriteComponent>(request.spawnerID)) {
        spawnBounds = entities.template GetComponent<SpriteComponent>(request.spawnerID).bounds;
    } else {
        std::cerr << "Cannot spawn star: spawner has no bounds\n";
        return false;
    }

    spawnBounds.topLeft.x += spawnerLocation.loc.x;
    spawnBounds.topLeft.y += spawnerLocation.loc.y;
    if (spawnBounds.width <= 0.0f || spawnBounds.height <= 0.0f) {
        std::cerr << "Cannot spawn star: spawner bounds are empty\n";
        return false;
    }

    const float spawnX = spawnBounds.topLeft.x + (spawnBounds.width - starWidth) / 2.0f;
    const float spawnY = spawnBounds.topLeft.y - starHeight;
    const float angle = std::uniform_real_distribution<float>(kPi, 2.0f * kPi)(randomGenerator);
    const Point2D velocity(std::cos(angle) * kStarSpeed, std::sin(angle) * kStarSpeed);
    Rect starCollisionBox{Point2D(0.0f, 0.0f), starWidth, starHeight};
    Rect previousCollisionBox = starCollisionBox;
    previousCollisionBox.topLeft.x += spawnX;
    previousCollisionBox.topLeft.y += spawnY;

    const EntityID starID = entities.CreateEntity("star");
    entities.AddComponent(starID, LocationComponent{Point2D(spawnX, spawnY)});

    MovementComponent movement{};
    movement.velocity = velocity;
    entities.AddComponent(starID, movement);
    entities.AddComponent(starID, GravityComponent(Point2D(0.0f, 1.0f)));

    CollisionComponent collision{};
    collision.collisionBox = starCollisionBox;
    collision.previousCollisionBox = previousCollisionBox;
    entities.AddComponent(starID, collision);

    SpriteComponent sprite{};
    sprite.textureID = request.textureID;
    sprite.bounds = Rect{Point2D(0.0f, 0.0f), starWidth, starHeight};
    sprite.worldSpace = true;
    entities.AddComponent(starID, sprite);

    return true;
}

}  // namespace SpawnSystemDetail

template <typename... Components>
class SpawnSystem : public System<Components...> {
public:
    // Seeds the random direction generator for this spawner system.
    SpawnSystem() : mRandomGenerator(std::random_device{}()) {}

    // Returns this system's stable name.
    const std::string& GetName() const override {
        static std::string name("SpawnSystem");
        return name;
    }

    // Counts down active spawners and creates queued stars for one frame.
    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto& entities = engine->GetEntityManager();
        auto& spriteManager = engine->GetSpriteManager();
        auto& spawnStorage = entities.template GetComponentStorage<SpawnComponent>();
        std::vector<SpawnSystemDetail::SpawnRequest> pending;

        for (size_t i = 0; i < spawnStorage.Size(); ++i) {
            if (!spawnStorage.IsValid(i)) {
                continue;
            }

            SpawnComponent& spawner = spawnStorage.Get(i);
            const EntityID owner = spawnStorage.GetOwner(i);
            if (spawner.totalSpawns <= 0) {
                continue;
            }

            if (spawner.entityID != owner) {
                std::cerr << "Spawner component owner does not match its configured entity; "
                             "disabling it\n";
                spawner.totalSpawns = 0;
                continue;
            }

            if (spawner.timeBetweenSpawns <= 0) {
                std::cerr << "Spawner " << owner
                          << " has a non-positive spawn interval; disabling it\n";
                spawner.totalSpawns = 0;
                continue;
            }

            if (spawner.timeToNextSpawn > 0) {
                --spawner.timeToNextSpawn;
            }
            if (spawner.timeToNextSpawn > 0) {
                continue;
            }

            if (spawner.spawnType != SpawnSystemDetail::kStarSpawnType) {
                std::cerr << "Spawner " << owner << " has unsupported type '"
                          << spawner.spawnType << "'; disabling it\n";
                spawner.totalSpawns = 0;
                continue;
            }

            pending.push_back({owner, spawner.spawnTextureID});
            spawner.timeToNextSpawn = spawner.timeBetweenSpawns;
        }

        for (const SpawnSystemDetail::SpawnRequest& request : pending) {
            sf::IntRect textureBounds{};
            try {
                textureBounds = spriteManager.GetSprite(request.textureID).getTextureRect();
            } catch (const std::out_of_range& error) {
                std::cerr << "Spawner " << request.spawnerID << " has an invalid star texture: "
                          << error.what() << "; disabling it\n";
                if (entities.IsEntityIDValid(request.spawnerID) &&
                    entities.template HasComponent<SpawnComponent>(request.spawnerID)) {
                    entities.template GetComponent<SpawnComponent>(request.spawnerID).totalSpawns =
                        0;
                }
                continue;
            }

            if (SpawnSystemDetail::SpawnStar(
                    entities, request, mRandomGenerator,
                    static_cast<float>(textureBounds.size.x),
                    static_cast<float>(textureBounds.size.y))) {
                --entities.template GetComponent<SpawnComponent>(request.spawnerID).totalSpawns;
            } else if (entities.IsEntityIDValid(request.spawnerID) &&
                       entities.template HasComponent<SpawnComponent>(request.spawnerID)) {
                entities.template GetComponent<SpawnComponent>(request.spawnerID).totalSpawns = 0;
            }
        }

        return kContinue;
    }

private:
    std::mt19937 mRandomGenerator;
};

}  // namespace ECSEngine
