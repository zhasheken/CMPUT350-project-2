#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace ECSEngine {

// A system can continue the pipeline or request it to stop.
enum tSystemReturn { kContinue, kExit };

template <typename... Components>
class EngineView;

template <typename... Components>
class System {
public:
    // Virtual destruction lets SystemManager safely own derived systems.
    virtual ~System() = default;

    // Runs one update using only services exposed through EngineView.
    virtual tSystemReturn Run(EngineView<Components...>* scene) = 0;

    // Returns a stable display name for this system.
    virtual const std::string& GetName() const = 0;
};

template <typename... Components>
class SystemManager {
public:
    // Runs systems in insertion order; returns false immediately when one returns kExit.
    bool Run(EngineView<Components...>* scene) {
        for (auto& system : mSystems) {
            if (system->Run(scene) == kExit)  // system wants to exit
            {
                std::cout << system->GetName() << " requested exit from system manager\n";
                return false;
            }
        }
        return true;
    }
    // Takes ownership of system and appends it to the update order.
    void AddSystem(std::unique_ptr<System<Components...>>&& system) {
        mSystems.push_back(std::move(system));
    }

    // Destroys all owned systems and clears the update order.
    void ClearSystems() { mSystems.clear(); }

private:
    std::vector<std::unique_ptr<System<Components...>>> mSystems;
};

}  // namespace ECSEngine
