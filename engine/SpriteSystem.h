#pragma once

#include <memory>

#include "EntityManager.h"
#include "LocationComponent.h"
#include "SpriteComponent.h"
#include "SpriteManager.h"
#include "SystemManager.h"
#include "WindowManager.h"

namespace ECSEngine {

template <typename... Components>
class SpriteSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("SpriteSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        EntityManager<Components...>& entities = engine->GetEntityManager();
        SpriteManager& spriteManager = engine->GetSpriteManager();
        WindowManager& windowManager = engine->GetWindowManager();
        std::shared_ptr<sf::RenderWindow> window = windowManager.GetWindow();
        ComponentStorage<SpriteComponent>& sprites =
            entities.template GetComponentStorage<SpriteComponent>();

        // draw every sprite
        for (size_t i = 0; i < sprites.Size(); i++) {
            if (!sprites.IsValid(i)) {
                continue;
            }

            EntityID owner = sprites.GetOwner(i);
            if (!entities.template HasComponent<LocationComponent>(owner)) {
                continue;
            }

            // bounds are relative to the location
            SpriteComponent& spriteComponent = sprites.Get(i);
            Point2D location = entities.template GetComponent<LocationComponent>(owner).loc;
            Rect drawRect = spriteComponent.bounds;
            drawRect.topLeft.x += location.x;
            drawRect.topLeft.y += location.y;

            // world sprites follow the camera
            if (spriteComponent.worldSpace) {
                drawRect = windowManager.WorldToWindow(drawRect);
            }

            // stretch the sprite to fit
            sf::Sprite& sprite = spriteManager.GetSprite(spriteComponent.textureID);
            sf::IntRect source = sprite.getTextureRect();
            float scaleX = drawRect.width / static_cast<float>(source.size.x);
            float scaleY = drawRect.height / static_cast<float>(source.size.y);

            sprite.setPosition(sf::Vector2f(drawRect.topLeft.x, drawRect.topLeft.y));
            sprite.setScale(sf::Vector2f(scaleX, scaleY));
            window->draw(sprite);
        }
        return kContinue;
    }
};

}  // namespace ECSEngine
