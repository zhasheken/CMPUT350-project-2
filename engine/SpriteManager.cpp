#include "SpriteManager.h"

#include <stdexcept>

namespace ECSEngine {

SpriteID SpriteManager::RegisterTexture(const std::string& texturePath, const Rect& sourceRect) {
    auto [iterator, inserted] = textures.try_emplace(texturePath);

    if (inserted) {
        if (!iterator->second.loadFromFile(texturePath)) {
            textures.erase(iterator);
            throw std::runtime_error("SpriteManager: failed to load texture from '" + texturePath + "'");
        }
    }

    sf::IntRect intRect{
        {
            static_cast<int>(sourceRect.topLeft.x), 
            static_cast<int>(sourceRect.topLeft.y)},

        {
            static_cast<int>(sourceRect.width), 
            static_cast<int>(sourceRect.height)
        }
    };

    sprites.emplace_back(iterator->second, intRect);

    return sprites.size() - 1;
}

sf::Sprite& SpriteManager::GetSprite(SpriteID id) {
    if (id >= sprites.size()) {
        throw std::out_of_range("SpriteManager: sprite id " + std::to_string(id) + " is out of range");
    }
    return sprites[id];
}

}  // namespace ECSEngine