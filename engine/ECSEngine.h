#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "CameraComponent.h"
#include "CollisionComponent.h"
#include "EntityManager.h"
#include "InputComponent.h"
#include "Keyboard.h"
#include "LocationComponent.h"
#include "MathUtil.h"
#include "ScoreComponent.h"
#include "SoundManager.h"
#include "SpawnComponent.h"
#include "SpriteComponent.h"
#include "SpriteManager.h"
#include "SystemManager.h"
#include "WindowManager.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

namespace ECSEngine {

template <typename... Components>
class EngineView {
public:
    virtual SoundManager& GetSoundManager() = 0;
    virtual SpriteManager& GetSpriteManager() = 0;
    virtual EntityManager<Components...>& GetEntityManager() = 0;
    virtual WindowManager& GetWindowManager() = 0;
};

template <typename... Components>
class ECSEngine : public EngineView<Components...> {
public:
    ECSEngine(unsigned int width, unsigned int height, const std::string& name);
    ~ECSEngine();

    void Run();

    SoundManager& GetSoundManager() override { return mSoundManager; }

    SpriteManager& GetSpriteManager() override { return mSpriteManager; }

    EntityManager<Components...>& GetEntityManager() override { return mEntityManager; }

    WindowManager& GetWindowManager() override { return mWindowManager; }

    SystemManager<Components...>& GetSystemManager() { return mSystemManager; }

private:
    bool ProcessEvents();
    void DebugDraw();

    EntityManager<Components...> mEntityManager;
    SystemManager<Components...> mSystemManager;
    SpriteManager mSpriteManager;
    SoundManager mSoundManager;
    WindowManager mWindowManager;

    // For debugging - pause and step the engine
    bool running, step;
    int debug;
};

template <typename... Components>
ECSEngine<Components...>::ECSEngine(unsigned int width, unsigned int height,
                                    const std::string& name)
    : mWindowManager(width, height, name), running(true), step(false), debug(0) {}

template <typename... Components>
ECSEngine<Components...>::~ECSEngine() {}

template <typename... Components>
void ECSEngine<Components...>::Run() {
    std::shared_ptr<sf::RenderWindow> window = mWindowManager.GetWindow();

    while (window->isOpen()) {
        // false means the window closed
        if (!ProcessEvents()) {
            break;
        }

        // paused, unless stepping one frame
        if (!running && !step) {
            sf::sleep(sf::milliseconds(10));
            continue;
        }
        step = false;

        window->clear();

        // false means a system returned kExit
        if (!mSystemManager.Run(this)) {
            break;
        }

        if (debug) {
            DebugDraw();
        }
        window->display();
    }
}

template <typename... Components>
bool ECSEngine<Components...>::ProcessEvents() {
    std::shared_ptr<sf::RenderWindow> window = mWindowManager.GetWindow();
    ComponentStorage<InputComponent>& inputs =
        mEntityManager.template GetComponentStorage<InputComponent>();

    // remember last frame's keys
    for (size_t i = 0; i < inputs.Size(); i++) {
        if (inputs.IsValid(i)) {
            inputs.Get(i).lastKeydown = inputs.Get(i).keydown;
        }
    }

    while (const std::optional<sf::Event> event = window->pollEvent()) {
        // window closed
        if (event->is<sf::Event::Closed>()) {
            window->close();
            return false;
        }

        // forget held keys on focus loss
        if (event->is<sf::Event::FocusLost>()) {
            for (size_t i = 0; i < inputs.Size(); i++) {
                if (inputs.IsValid(i)) {
                    inputs.Get(i).keydown.reset();
                }
            }
        }

        // a key went down
        const sf::Event::KeyPressed* pressed = event->getIf<sf::Event::KeyPressed>();
        if (pressed != nullptr && pressed->scancode != sf::Keyboard::Scan::Unknown) {
            unsigned int key = static_cast<unsigned int>(pressed->scancode);

            for (size_t i = 0; i < inputs.Size(); i++) {
                if (inputs.IsValid(i)) {
                    inputs.Get(i).keydown[key] = true;
                }
            }

            // engine debug keys
            if (key == Key::D) {
                debug = !debug;
            }
            if (key == Key::P) {
                running = !running;
            }
            if (key == Key::N) {
                step = true;
            }
        }

        // a key went up
        const sf::Event::KeyReleased* released = event->getIf<sf::Event::KeyReleased>();
        if (released != nullptr && released->scancode != sf::Keyboard::Scan::Unknown) {
            unsigned int key = static_cast<unsigned int>(released->scancode);

            for (size_t i = 0; i < inputs.Size(); i++) {
                if (inputs.IsValid(i)) {
                    inputs.Get(i).keydown[key] = false;
                }
            }
        }
    }
    return true;
}

template <typename... Components>
void ECSEngine<Components...>::DebugDraw() {
    ComponentStorage<CollisionComponent>& collisions =
        mEntityManager.template GetComponentStorage<CollisionComponent>();

    // outline every collision box
    for (size_t i = 0; i < collisions.Size(); i++) {
        if (!collisions.IsValid(i)) {
            continue;
        }

        EntityID owner = collisions.GetOwner(i);
        Point2D location = mEntityManager.template GetComponent<LocationComponent>(owner).loc;
        Rect box = collisions.Get(i).collisionBox;

        // boxes are relative to the location
        box.topLeft.x += location.x;
        box.topLeft.y += location.y;

        mWindowManager.DebugFrameRect(box, 255, 0, 0);
    }
}

}  // namespace ECSEngine
