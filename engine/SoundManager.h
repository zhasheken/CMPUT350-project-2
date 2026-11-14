#pragma once

#include <string>
#include <unordered_map>

#include <SFML/Audio.hpp>

namespace ECSEngine {

class SoundManager {
public:
    // Owns buffers and sounds; each sound references its buffer, and sounds are destroyed first.
    SoundManager() = default;
    // Releases sounds before the buffers they reference.
    ~SoundManager() = default;
    // Copying is disabled because sounds refer to manager-owned buffers.
    SoundManager(const SoundManager&) = delete;
    // Copy assignment is disabled for the same resource-lifetime reason.
    SoundManager& operator=(const SoundManager&) = delete;

    // Loads a named sound. Duplicate names are ignored; load failures are logged and not stored.
    void RegisterSound(const std::string& soundPath, const std::string& soundName);

    // Returns a sound owned by this manager; the reference lasts until manager destruction.
    // Throws std::runtime_error if soundName was not successfully registered.
    sf::Sound& GetSound(const std::string& soundName);

private:
    std::unordered_map<std::string, sf::SoundBuffer> soundBuffer;
    std::unordered_map<std::string, sf::Sound> sound;
};

}  // namespace ECSEngine
