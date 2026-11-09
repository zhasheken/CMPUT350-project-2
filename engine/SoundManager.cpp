#include "SoundManager.h"

#include <iostream>
#include <stdexcept>

namespace ECSEngine {


void SoundManager::RegisterSound(const std::string& soundPath, const std::string& soundName) {
    auto [iterator, inserted] = soundBuffer.try_emplace(soundName);

    if (!inserted) {
        std::cerr << soundName << "' is already registered, ignoring\n";
        return;
    }

    if (!iterator->second.loadFromFile(soundPath)) {
        std::cerr << "failed to load audio from '" << soundPath << "'\n";
        soundBuffer.erase(iterator);
        return;
    }

    sound.emplace(soundName, iterator->second);
}


sf::Sound& SoundManager::GetSound(const std::string& soundName) {
    auto iterator = SoundManager::sound.find(soundName);

    if (iterator == SoundManager::sound.end()) {

    throw std::runtime_error("unknown sound: " + soundName);

    }

    return iterator->second;
}

}  // namespace ECSEngine
