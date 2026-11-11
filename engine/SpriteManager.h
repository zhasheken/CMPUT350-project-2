#pragma once

#include <unordered_map>
#include <deque>
#include <cstddef>
#include <string>
 
#include "MathUtil.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace ECSEngine {

using SpriteID = size_t;

class SpriteManager {
public:
    SpriteManager() = default;
    ~SpriteManager() = default;
    SpriteManager(const SpriteManager&) = delete;
    SpriteManager& operator=(const SpriteManager&) = delete;
    [[nodiscard]] SpriteID RegisterTexture(const std::string& texturePath, const Rect& sourceRect);
    sf::Sprite& GetSprite(SpriteID id);
    

private:
    std::unordered_map<std::string, sf::Texture> textures;
    std::deque<sf::Sprite> sprites;
};

}  // namespace ECSEngine
