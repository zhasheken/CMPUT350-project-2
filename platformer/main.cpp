#include <iostream>
#include <memory>
#include <string>

#include "CameraComponent.h"
#include "CameraFollowerComponent.h"
#include "CameraShakeComponent.h"
#include "CollisionComponent.h"
#include "ECSEngine.h"
#include "GravityComponent.h"
#include "InputComponent.h"
#include "LocationComponent.h"
#include "MovementComponent.h"
#include "RespawnComponent.h"
#include "ScoreComponent.h"
#include "SpawnComponent.h"
#include "SpriteComponent.h"

#include "CameraSystem.h"
#include "CollisionLateSystem.h"
#include "CollisionSystem.h"
#include "GravitySystem.h"
#include "InputSystem.h"
#include "MovementSystem.h"
#include "ScoreSystem.h"
#include "SpawnSystem.h"
#include "SpriteSystem.h"

std::string gResourcePath = "../assets/";
#define COMPONENTS                                                                             \
    ECSEngine::LocationComponent, ECSEngine::MovementComponent, ECSEngine::CollisionComponent, \
        ECSEngine::SpriteComponent, ECSEngine::SpawnComponent, ECSEngine::CameraComponent,     \
        ECSEngine::CameraFollowerComponent, ECSEngine::InputComponent,                         \
        ECSEngine::GravityComponent, ECSEngine::CameraShakeComponent, ECSEngine::ScoreComponent, \
        ECSEngine::RespawnComponent
using Engine = ECSEngine::ECSEngine<COMPONENTS>;

void LoadMap(Engine&, const std::string&, const std::string&) {}

int main(int argc, char* argv[]) {
    if (argc >= 3 && argv[1] == std::string("-path")) {
        gResourcePath = argv[2];
    }
    std::cout << "Usage: " << argv[0] << " [-path resource_path]\n";
    std::cout << "Using resource path: " << gResourcePath << "\n";

    // Engine test code
    Engine engine(1024, 768, "Test Engine");

    // Load sounds

    // Load maps

    // Place player

    // Setup score component

    // Setup Camera

    // Add all systems
    auto& sm = engine.GetSystemManager();
    sm.AddSystem(std::make_unique<ECSEngine::CollisionLateSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::InputSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::GravitySystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::MovementSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::CollisionSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::ScoreSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::CameraSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::SpriteSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::SpawnSystem<COMPONENTS>>());

    engine.Run();
    return 0;
}
