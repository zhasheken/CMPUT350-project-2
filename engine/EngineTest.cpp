#include <cmath>
#include <iostream>
#include <string>

#include "CameraComponent.h"
#include "CameraFollowerComponent.h"
#include "CameraSystem.h"
#include "CameraShakeComponent.h"
#include "CollisionComponent.h"
#include "CollisionSystem.h"
#include "Entity.h"
#include "EntityManager.h"
#include "GravityComponent.h"
#include "InputComponent.h"
#include "InputSystem.h"
#include "LocationComponent.h"
#include "MovementComponent.h"
#include "RespawnComponent.h"
#include "ScoreComponent.h"
#include "SpawnComponent.h"
#include "SpawnSystem.h"
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

void TestCollisionGeometry() {
    CollisionComponent collision{};
    collision.collisionBox = {{2.0f, 3.0f}, 4.0f, 5.0f};
    LocationComponent location{Point2D(10.0f, 20.0f)};
    Rect worldBox = CollisionSystemDetail::WorldBox(collision, location);
    Check(worldBox.topLeft.x == 12.0f && worldBox.topLeft.y == 23.0f,
          "collision box is translated into world space");
    Check(CollisionSystemDetail::Overlaps(worldBox, Rect{{15.0f, 27.0f}, 2.0f, 2.0f}),
          "overlapping rectangles are detected");
    Check(!CollisionSystemDetail::Overlaps(worldBox, Rect{{16.0f, 23.0f}, 2.0f, 2.0f}),
          "touching rectangle edges are not overlap");

    const Rect floor{{0.0f, 10.0f}, 20.0f, 10.0f};
    const Rect playerAbove{{2.0f, 8.0f}, 4.0f, 4.0f};
    const Rect previousAbove{{2.0f, 5.0f}, 4.0f, 4.0f};
    auto landing =
        CollisionSystemDetail::ResolveOverlap(previousAbove, playerAbove, floor);
    Check(landing.side == CollisionSystemDetail::Side::top &&
              landing.correction.y == -2.0f,
          "landing resolves upward from the obstacle top");

    const Rect wall{{10.0f, 0.0f}, 10.0f, 20.0f};
    const Rect playerAtWall{{8.0f, 2.0f}, 4.0f, 4.0f};
    const Rect previousAtWall{{5.0f, 2.0f}, 4.0f, 4.0f};
    auto wallHit =
        CollisionSystemDetail::ResolveOverlap(previousAtWall, playerAtWall, wall);
    Check(wallHit.side == CollisionSystemDetail::Side::left &&
              wallHit.correction.x == -2.0f,
          "wall collision resolves toward the previous side");

    const Rect playerBelow{{2.0f, 18.0f}, 4.0f, 4.0f};
    const Rect previousBelow{{2.0f, 21.0f}, 4.0f, 4.0f};
    auto ceiling =
        CollisionSystemDetail::ResolveOverlap(previousBelow, playerBelow, floor);
    Check(ceiling.side == CollisionSystemDetail::Side::bottom &&
              ceiling.correction.y == 2.0f,
          "ceiling collision resolves downward");

    const Rect playerRight{{8.0f, 2.0f}, 4.0f, 4.0f};
    const Rect previousRight{{21.0f, 2.0f}, 4.0f, 4.0f};
    auto oppositeWall =
        CollisionSystemDetail::ResolveOverlap(previousRight, playerRight, wall);
    Check(oppositeWall.side == CollisionSystemDetail::Side::right &&
              oppositeWall.correction.x == 12.0f,
          "opposite wall collision resolves away from the obstacle");

    auto initialOverlap = CollisionSystemDetail::ResolveOverlap(
        Rect{{5.0f, 5.0f}, 1.0f, 1.0f}, Rect{{8.0f, 3.0f}, 4.0f, 4.0f},
        Rect{{0.0f, 0.0f}, 10.0f, 10.0f});
    Check(initialOverlap.side == CollisionSystemDetail::Side::right &&
              initialOverlap.correction.x == 2.0f,
          "initial overlap is resolved along the shallowest axis");

    const Rect firstStar{{0.0f, 0.0f}, 4.0f, 4.0f};
    const Rect secondStar{{3.0f, 1.0f}, 4.0f, 4.0f};
    const auto starHit =
        CollisionSystemDetail::ResolveStarOverlap(firstStar, secondStar, 1, 2);
    Rect separatedFirst = firstStar;
    Rect separatedSecond = secondStar;
    separatedFirst.topLeft.x += starHit.correction.x;
    separatedFirst.topLeft.y += starHit.correction.y;
    separatedSecond.topLeft.x -= starHit.correction.x;
    separatedSecond.topLeft.y -= starHit.correction.y;
    Check(starHit.axis == CollisionSystemDetail::Axis::horizontal &&
              !CollisionSystemDetail::Overlaps(separatedFirst, separatedSecond),
          "star collision separates along the shallowest axis");
    Check(CollisionSystemDetail::BounceVelocity(10.0f) == -7.0f,
          "star bounce reflects and reduces speed");
    Check(CollisionSystemDetail::BounceVelocity(1.0f) == 0.0f,
          "small star bounce velocity settles to zero");
}

void TestInputHelpers() {
    InputComponent input(0);
    MovementComponent movement{};
    input.keydown[Key::Right] = true;
    InputSystemDetail::ApplyHorizontalInput(movement, input, true);
    Check(movement.velocity.x == kGroundRunAcceleration,
          "ground movement accelerates at the ground rate");

    movement.velocity.x = 0.0f;
    InputSystemDetail::ApplyHorizontalInput(movement, input, false);
    Check(movement.velocity.x == kAirRunAcceleration,
          "air movement accelerates at the air rate");

    movement.velocity.x = kMaxRunVelocity - 1.0f;
    InputSystemDetail::ApplyHorizontalInput(movement, input, true);
    Check(movement.velocity.x == kMaxRunVelocity, "run speed is capped");

    input.keydown.reset();
    movement.velocity.x = kGroundRunAcceleration;
    InputSystemDetail::ApplyHorizontalInput(movement, input, true);
    Check(movement.velocity.x == 0.0f, "horizontal speed decelerates without input");

    input.lastKeydown[Key::Z] = false;
    input.keydown[Key::Z] = true;
    Check(InputSystemDetail::WasPressed(input, Key::Z), "jump press edge is detected");
    input.lastKeydown[Key::Z] = true;
    input.keydown[Key::Z] = false;
    Check(InputSystemDetail::WasReleased(input, Key::Z), "jump release edge is detected");

    input.keydown[Key::Left] = true;
    movement.velocity.x = 0.0f;
    Check(InputSystemDetail::DashDirection(input, movement) == -1,
          "dash follows the held horizontal direction");
    input.keydown.reset();
    movement.velocity.x = -1.0f;
    Check(InputSystemDetail::DashDirection(input, movement) == -1,
          "dash uses the current direction when no direction key is held");
    Check(kDashInterval == 60, "dash cooldown lasts one second at 60 frames per second");
}

void TestCameraHelpers() {
    using namespace CameraSystemDetail;

    Check(TrackHorizontalPosition(0.0f, 0.0f, 1.0f, 1000.0f, 0.1f) == 0.0f,
          "camera remains still while the target is in the center 40 percent");

    const float softFollow = TrackHorizontalPosition(0.0f, -300.0f, 1.0f, 1000.0f, 0.1f);
    Check(softFollow < 0.0f && softFollow > -200.0f,
          "camera smoothly follows in the soft margin");

    const float hardFollow = TrackHorizontalPosition(0.0f, -450.0f, 1.0f, 1000.0f, 0.1f);
    const float hardTargetScreenX = 500.0f + (-450.0f - hardFollow);
    Check(hardTargetScreenX == 100.0f,
          "hard tracking keeps the target out of the outer ten percent");

    CameraShakeComponent shake{2, Point2D(3.0f, 4.0f)};
    const Point2D firstOffset = AdvanceShake(shake);
    const Point2D secondOffset = AdvanceShake(shake);
    const Point2D stoppedOffset = AdvanceShake(shake);
    Check(firstOffset.x == 3.0f && firstOffset.y == 4.0f &&
              secondOffset.x == -3.0f && secondOffset.y == -4.0f,
          "camera shake alternates its offset");
    Check(shake.frameLimit == 0 && stoppedOffset.x == 0.0f && stoppedOffset.y == 0.0f,
          "camera shake expires after its frame limit");
}

void TestSpawnStar() {
    TestManager entities;
    const EntityID spawnerID = entities.CreateEntity("spawner");
    entities.AddComponent(spawnerID, LocationComponent{Point2D(100.0f, 200.0f)});

    CollisionComponent spawnerCollision{};
    spawnerCollision.staticEntity = 1;
    spawnerCollision.collisionBox = Rect{Point2D(0.0f, 0.0f), 64.0f, 64.0f};
    entities.AddComponent(spawnerID, spawnerCollision);

    std::mt19937 randomGenerator(42);
    const SpawnSystemDetail::SpawnRequest request{spawnerID, 17};
    Check(SpawnSystemDetail::SpawnStar(entities, request, randomGenerator, 64.0f, 64.0f),
          "star is created for a valid spawner");

    const EntityID starID = 1;
    Check(entities.IsEntityIDValid(starID), "spawned star entity is valid");
    Check(entities.GetComponent<LocationComponent>(starID).loc.x == 100.0f &&
              entities.GetComponent<LocationComponent>(starID).loc.y == 136.0f,
          "star starts immediately above the spawner collision");
    Check(!entities.GetComponent<CollisionComponent>(starID).staticEntity,
          "spawned star is dynamic");
    Check(entities.GetComponent<CollisionComponent>(starID).previousCollisionBox.topLeft.y ==
              136.0f,
          "spawned star starts with a world-space previous collision box");
    Check(entities.GetComponent<SpriteComponent>(starID).textureID == 17 &&
              entities.GetComponent<SpriteComponent>(starID).worldSpace,
          "spawned star uses its configured world space sprite");
    const Point2D velocity = entities.GetComponent<MovementComponent>(starID).velocity;
    Check(std::abs(std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y) - 15.0f) <
              0.001f,
          "spawned star receives a fixed-speed random direction");
    Check(velocity.y <= 0.0f, "spawned star initially launches upward");
}

int main() {
    TestCreateAndRemove();
    TestIteration();
    TestComponents();
    TestEveryComponentType();
    TestCollisionGeometry();
    TestInputHelpers();
    TestCameraHelpers();
    TestSpawnStar();

    if (gFailures > 0) {
        std::cout << gFailures << " EntityManager test(s) failed\n";
        return 1;
    }
    std::cout << "all EntityManager tests passed\n";
    return 0;
}
