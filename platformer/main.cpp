#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

#include "CameraComponent.h"
#include "CameraFollowerComponent.h"
#include "CameraShakeComponent.h"
#include "CameraSystem.h"
#include "CollisionComponent.h"
#include "CollisionLateSystem.h"
#include "CollisionSystem.h"
#include "ECSEngine.h"
#include "GravityComponent.h"
#include "GravitySystem.h"
#include "InputComponent.h"
#include "InputSystem.h"
#include "LocationComponent.h"
#include "MovementComponent.h"
#include "MovementSystem.h"
#include "RespawnComponent.h"
#include "ScoreComponent.h"
#include "ScoreSystem.h"
#include "SpawnComponent.h"
#include "SpawnSystem.h"
#include "SpriteComponent.h"
#include "SpriteSystem.h"

std::string gResourcePath = "../assets/";
#define COMPONENTS                                                                               \
    ECSEngine::LocationComponent, ECSEngine::MovementComponent, ECSEngine::CollisionComponent,   \
        ECSEngine::SpriteComponent, ECSEngine::SpawnComponent, ECSEngine::CameraComponent,       \
        ECSEngine::CameraFollowerComponent, ECSEngine::InputComponent,                           \
        ECSEngine::GravityComponent, ECSEngine::CameraShakeComponent, ECSEngine::ScoreComponent, \
        ECSEngine::RespawnComponent
using Engine = ECSEngine::ECSEngine<COMPONENTS>;

// place an entity with a sprite
void AddWorldLocationAndSprite(Engine& engine, ECSEngine::EntityID entity, float x, float y,
                               ECSEngine::SpriteID spriteID, float width, float height) {
    ECSEngine::EntityManager<COMPONENTS>& entities = engine.GetEntityManager();

    // top-left corner in the world
    ECSEngine::LocationComponent location;
    location.loc.x = x;
    location.loc.y = y;
    entities.AddComponent(entity, location);

    // sprite drawn in world space
    ECSEngine::SpriteComponent sprite;
    sprite.textureID = spriteID;
    sprite.bounds.width = width;
    sprite.bounds.height = height;
    sprite.worldSpace = true;
    entities.AddComponent(entity, sprite);
}

// make an entity solid
void AddSolidObstacleCollision(Engine& engine, ECSEngine::EntityID entity,
                               const ECSEngine::Rect& box) {
    ECSEngine::CollisionComponent collision;
    collision.staticEntity = 1;
    collision.collisionBox = box;
    engine.GetEntityManager().AddComponent(entity, collision);
}

// make an entity spawn stars
void AddStarSpawn(Engine& engine, ECSEngine::EntityID entity, const std::string& resourcePath) {
    // star sprite in the sheet
    ECSEngine::Rect starSource{ECSEngine::Point2D(640, 320), 64, 64};

    ECSEngine::SpawnComponent spawn;
    spawn.entityID = entity;
    spawn.spawnType = "star";
    spawn.spawnTextureID = engine.GetSpriteManager().RegisterTexture(
        resourcePath + "spritesheet-tiles-default.png", starSource);

    // spawn timing in frames
    spawn.timeToNextSpawn = 60;
    spawn.timeBetweenSpawns = 60;
    spawn.totalSpawns = 20;
    engine.GetEntityManager().AddComponent(entity, spawn);
}

// build entities from a map file
void LoadMap(Engine& engine, const std::string& resourcePath, const std::string& mapFile) {
    ECSEngine::SpriteManager& spriteManager = engine.GetSpriteManager();
    ECSEngine::EntityManager<COMPONENTS>& entities = engine.GetEntityManager();

    // open the map file
    std::ifstream file(resourcePath + mapFile);
    if (!file) {
        std::cerr << "Could not open map: " << resourcePath + mapFile << "\n";
        return;
    }

    // read the dictionary type
    std::string word;
    int dictionaryType = 0;
    file >> word >> dictionaryType;

    // read sprites until "map"
    std::unordered_map<char, ECSEngine::SpriteID> spriteIDs;
    std::unordered_map<char, ECSEngine::Rect> boundingBoxes;
    while (file >> word && word != "map") {
        char symbol = word[0];

        // sheet name and sprite rectangle
        std::string texturePath;
        ECSEngine::Rect source{};
        file >> texturePath;
        file >> source.topLeft.x;
        file >> source.topLeft.y;
        file >> source.width;
        file >> source.height;

        // remember the sprite for this character
        spriteIDs[symbol] = spriteManager.RegisterTexture(resourcePath + texturePath, source);

        // type 2 adds a bounding box
        if (dictionaryType == 2) {
            ECSEngine::Rect box{};
            file >> box.topLeft.x;
            file >> box.topLeft.y;
            file >> box.width;
            file >> box.height;
            boundingBoxes[symbol] = box;
        }
    }

    // read origin, tile and size
    float originX = 0;
    float originY = 0;
    float tileWidth = 0;
    float tileHeight = 0;
    int gridWidth = 0;
    int gridHeight = 0;
    file >> word >> originX >> originY;
    file >> word >> tileWidth >> tileHeight;
    file >> word >> gridWidth >> gridHeight;

    // stop if the header failed
    if (!file) {
        std::cerr << "Could not read map header: " << resourcePath + mapFile << "\n";
        return;
    }

    // one entity per tile
    for (int row = 0; row < gridHeight; row++) {
        // one row of characters
        std::string line;
        file >> line;

        for (int column = 0; column < gridWidth; column++) {
            char symbol = line[column];

            // skip characters without sprites
            if (spriteIDs.count(symbol) == 0) {
                continue;
            }

            // place the tile in the world
            ECSEngine::EntityID tile = entities.CreateEntity("tile");
            float x = originX + column * tileWidth;
            float y = originY + row * tileHeight;
            AddWorldLocationAndSprite(engine, tile, x, y, spriteIDs[symbol], tileWidth, tileHeight);

            // dictionary 2 tiles are solid
            if (dictionaryType == 2) {
                AddSolidObstacleCollision(engine, tile, boundingBoxes[symbol]);
            }

            // spikes hurt the player
            if (dictionaryType == 2 && symbol == 'x') {
                entities.GetComponent<ECSEngine::CollisionComponent>(tile).doesDamage = 1;
            }

            // spawners create stars
            if (symbol == 'S') {
                AddStarSpawn(engine, tile, resourcePath);
            }
        }
    }
}

// load the 0 to 9 sprites
void RegisterDigitSprites(Engine& engine, ECSEngine::ScoreComponent& score) {
    for (int digit = 0; digit < 10; digit++) {
        // digits run upward from 0
        ECSEngine::Rect digitSpriteRect{ECSEngine::Point2D(832, 832 - digit * 64), 64, 64};
        score.digitTextureID[digit] = engine.GetSpriteManager().RegisterTexture(
            gResourcePath + "spritesheet-tiles-default.png", digitSpriteRect);
    }
}

// one digit fixed on screen
ECSEngine::EntityID CreateDigitDisplay(Engine& engine, float x, float y,
                                       ECSEngine::SpriteID startingDigitSprite) {
    ECSEngine::EntityManager<COMPONENTS>& entities = engine.GetEntityManager();
    ECSEngine::EntityID digitDisplayID = entities.CreateEntity("digit");

    ECSEngine::LocationComponent digitLocation;
    digitLocation.loc.x = x;
    digitLocation.loc.y = y;
    entities.AddComponent(digitDisplayID, digitLocation);

    // screen space ignores the camera
    ECSEngine::SpriteComponent digitDisplaySprite;
    digitDisplaySprite.textureID = startingDigitSprite;
    digitDisplaySprite.bounds.width = 48;
    digitDisplaySprite.bounds.height = 48;
    digitDisplaySprite.worldSpace = false;
    entities.AddComponent(digitDisplayID, digitDisplaySprite);

    return digitDisplayID;
}

int main(int argc, char* argv[]) {
    if (argc >= 3 && argv[1] == std::string("-path")) {
        gResourcePath = argv[2];
    }
    std::cout << "Usage: " << argv[0] << " [-path resource_path]\n";
    std::cout << "Using resource path: " << gResourcePath << "\n";

    // Engine test code
    Engine engine(1024, 768, "Test Engine");

    // Load sounds
    engine.GetSoundManager().RegisterSound(gResourcePath + "footstep_grass_003.ogg", "land");
    engine.GetSoundManager().RegisterSound(gResourcePath + "sfx_gem.ogg", "star");
    engine.GetSoundManager().RegisterSound(gResourcePath + "sfx_jump.ogg", "jump");
    engine.GetSoundManager().RegisterSound(gResourcePath + "footstep_snow_001.ogg", "wallSlide");

    // Load maps
    LoadMap(engine, gResourcePath, "sky.map");
    LoadMap(engine, gResourcePath, "world.map");

    // Place player
    ECSEngine::EntityManager<COMPONENTS>& entities = engine.GetEntityManager();
    ECSEngine::EntityID player = entities.CreateEntity("player");

    // first character in the sheet
    ECSEngine::Rect playerSource{ECSEngine::Point2D(0, 0), 128, 128};
    ECSEngine::SpriteID playerSprite = engine.GetSpriteManager().RegisterTexture(
        gResourcePath + "spritesheet-characters-default.png", playerSource);
    AddWorldLocationAndSprite(engine, player, 13 * 64, 4 * 64, playerSprite, 64, 64);

    // box around the body only
    ECSEngine::CollisionComponent playerCollision;
    playerCollision.collisionBox = ECSEngine::Rect{ECSEngine::Point2D(12, 13), 42, 51};
    entities.AddComponent(player, playerCollision);

    ECSEngine::MovementComponent playerMovement;
    entities.AddComponent(player, playerMovement);

    ECSEngine::InputComponent playerInput(player);
    entities.AddComponent(player, playerInput);

    ECSEngine::GravityComponent playerGravity(ECSEngine::Point2D(0, 1));
    entities.AddComponent(player, playerGravity);

    // respawn where the player starts
    ECSEngine::RespawnComponent playerRespawn;
    playerRespawn.loc = entities.GetComponent<ECSEngine::LocationComponent>(player).loc;
    entities.AddComponent(player, playerRespawn);

    // Setup score component
    ECSEngine::ScoreComponent score;
    RegisterDigitSprites(engine, score);

    // three digits in the top-left
    for (int digitIndex = 0; digitIndex < 3; digitIndex++) {
        float digitX = 16 + digitIndex * 30;
        score.digitID[digitIndex] = CreateDigitDisplay(engine, digitX, 16, score.digitTextureID[0]);
    }

    ECSEngine::EntityID scoreEntity = entities.CreateEntity("score");
    entities.AddComponent(scoreEntity, score);

    // Setup Camera
    ECSEngine::EntityID camera = entities.CreateEntity("camera");

    // start on the current view
    ECSEngine::CameraComponent cameraComponent;
    cameraComponent.position = ECSEngine::Point2D(512, 384);
    cameraComponent.worldUnitsPerPixel = 1.0f;
    cameraComponent.speed = 0.1f;
    entities.AddComponent(camera, cameraComponent);

    // follow the player
    ECSEngine::CameraFollowerComponent follower;
    follower.entityID = player;
    entities.AddComponent(camera, follower);

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
