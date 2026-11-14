#pragma once

#include <cstddef>
#include <deque>
#include <string>
#include <unordered_map>

#include "MathUtil.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace ECSEngine {

using SpriteID = size_t;

// Owns textures and sprites; sprites are destroyed first because they reference those textures.
class SpriteManager {
public:
    // Starts with empty texture and sprite stores.
    SpriteManager() = default;
    // Releases sprites before their referenced textures.
    ~SpriteManager() = default;
    // Copying is disabled to keep sprite/texture references owned by one manager.
    SpriteManager(const SpriteManager&) = delete;
    // Copy assignment is disabled for the same resource-lifetime reason.
    SpriteManager& operator=(const SpriteManager&) = delete;

    // Reuses a texture already loaded from texturePath, but creates a new sprite/ID per call.
    // Throws std::runtime_error if the texture cannot be loaded.
    [[nodiscard]] SpriteID RegisterTexture(const std::string& texturePath, const Rect& sourceRect);

    // Returns a manager-owned sprite; its reference lasts until manager destruction.
    // Throws std::out_of_range if id is not registered.
    sf::Sprite& GetSprite(SpriteID id);

private:
    std::unordered_map<std::string, sf::Texture> textures;
    std::deque<sf::Sprite> sprites;
};

}  // namespace ECSEngine
