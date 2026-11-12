#include <iostream>
#include <string>

#include "CameraComponent.h"
#include "CameraFollowerComponent.h"
#include "CameraShakeComponent.h"
#include "CollisionComponent.h"
#include "Entity.h"
#include "EntityManager.h"
#include "GravityComponent.h"
#include "InputComponent.h"
#include "LocationComponent.h"
#include "MovementComponent.h"
#include "RespawnComponent.h"
#include "ScoreComponent.h"
#include "SpawnComponent.h"
#include "SpriteComponent.h"

using namespace ECSEngine;

// same components as the platformer
using TestManager =
    EntityManager<LocationComponent, MovementComponent, CollisionComponent, SpriteComponent,
                  SpawnComponent, CameraComponent, CameraFollowerComponent, InputComponent,
                  GravityComponent, CameraShakeComponent, ScoreComponent, RespawnComponent>;

int gFailures = 0;

void Check(bool condition, const std::string& description) {
    if (!condition) {
        std::cout << "FAILED: " << description << "\n";
        gFailures++;
    }
}

int CountValidEntities(TestManager& manager) {
    int count = 0;
    for (const Entity& entity : manager) {
        if (entity.IsValid()) {
            count++;
        }
    }
    return count;
}

void TestCreateAndRemove() {
    TestManager manager;
    EntityID first = manager.CreateEntity("first");
    EntityID second = manager.CreateEntity("second");
    Check(first == 0 && second == 1, "IDs start at 0");

    manager.RemoveEntity(first);
    Check(!manager.IsEntityIDValid(first), "removed entity is invalid");
    Check(manager.IsEntityIDValid(second), "other entity still valid");

    // removed IDs get reused
    EntityID third = manager.CreateEntity("third");
    Check(third == first, "removed ID is reused");
    Check(!manager.IsEntityIDValid(999), "out-of-range ID is invalid");
}

void TestIteration() {
    TestManager manager;
    Check(CountValidEntities(manager) == 0, "empty manager has no entities");

    EntityID a = manager.CreateEntity("a");
    EntityID b = manager.CreateEntity("b");
    EntityID c = manager.CreateEntity("c");
    EntityID d = manager.CreateEntity("d");

    // gaps at start, middle and end
    manager.RemoveEntity(a);
    manager.RemoveEntity(c);
    manager.RemoveEntity(d);
    Check(CountValidEntities(manager) == 1, "iteration skips removed entities");
    Check(manager.IsEntityIDValid(b), "remaining entity is valid");
}

void TestComponents() {
    TestManager manager;
    EntityID player = manager.CreateEntity("player");
    EntityID star = manager.CreateEntity("star");

    manager.AddComponent(player, LocationComponent{Point2D(1.0f, 2.0f)});
    manager.AddComponent(player, InputComponent(player));
    manager.AddComponent(star, LocationComponent{Point2D(5.0f, 6.0f)});
    Check(manager.HasComponent<InputComponent>(player), "player has input");
    Check(!manager.HasComponent<InputComponent>(star), "star has no input");

    // changes go through the reference
    manager.GetComponent<LocationComponent>(star).loc.x = 9.0f;
    Check(manager.GetComponent<LocationComponent>(star).loc.x == 9.0f, "edit through reference");

    // adding twice replaces the old one
    manager.AddComponent(player, LocationComponent{Point2D(3.0f, 3.0f)});
    Check(manager.GetComponent<LocationComponent>(player).loc.x == 3.0f, "second add replaces");
    Check(manager.GetComponentStorage<LocationComponent>().Size() == 2, "replace uses no new slot");

    manager.RemoveComponent<InputComponent>(player);
    Check(!manager.HasComponent<InputComponent>(player), "component removed");

    // removing player leaves star alone
    manager.RemoveEntity(player);
    Check(!manager.HasComponent<LocationComponent>(player), "removed entity loses components");
    Check(manager.GetComponent<LocationComponent>(star).loc.x == 9.0f, "other entity untouched");

    // freed slot gets reused
    EntityID newPlayer = manager.CreateEntity("new player");
    manager.AddComponent(newPlayer, LocationComponent{Point2D(0.0f, 0.0f)});
    Check(manager.GetComponentStorage<LocationComponent>().Size() == 2, "freed slot is reused");
}

void TestEveryComponentType() {
    TestManager manager;
    EntityID entity = manager.CreateEntity("everything");

    manager.AddComponent(entity, LocationComponent{});
    manager.AddComponent(entity, MovementComponent{});
    manager.AddComponent(entity, CollisionComponent{});
    manager.AddComponent(entity, SpriteComponent{});
    manager.AddComponent(entity, SpawnComponent{});
    manager.AddComponent(entity, CameraComponent{});
    manager.AddComponent(entity, CameraFollowerComponent{});
    manager.AddComponent(entity, InputComponent(entity));
    manager.AddComponent(entity, GravityComponent(Point2D(0.0f, 1.0f)));
    manager.AddComponent(entity, CameraShakeComponent{});
    manager.AddComponent(entity, ScoreComponent{});
    manager.AddComponent(entity, RespawnComponent{});
    Check(manager.HasComponent<InputComponent>(entity), "input added");
    Check(manager.HasComponent<GravityComponent>(entity), "gravity added");

    // check the storage slots were freed
    manager.RemoveEntity(entity);
    Check(!manager.GetComponentStorage<InputComponent>().IsValid(0), "input freed with entity");
    Check(!manager.GetComponentStorage<GravityComponent>().IsValid(0), "gravity freed with entity");
}

int main() {
    TestCreateAndRemove();
    TestIteration();
    TestComponents();
    TestEveryComponentType();

    if (gFailures > 0) {
        std::cout << gFailures << " EntityManager test(s) failed\n";
        return 1;
    }
    std::cout << "all EntityManager tests passed\n";
    return 0;
}
