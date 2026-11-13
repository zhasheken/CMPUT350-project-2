#pragma once

#include "EntityManager.h"
#include "ScoreComponent.h"
#include "SpriteComponent.h"
#include "SystemManager.h"

namespace ECSEngine {

template <typename... Components>
class ScoreSystem : public System<Components...> {
private:
    // swap a display to a digit
    void ShowDigitSprite(EntityManager<Components...>& entities, EntityID digitDisplayID,
                         SpriteID digitSpriteID) {
        if (!entities.template HasComponent<SpriteComponent>(digitDisplayID)) {
            return;
        }

        SpriteComponent& digitDisplaySprite =
            entities.template GetComponent<SpriteComponent>(digitDisplayID);
        digitDisplaySprite.textureID = digitSpriteID;
    }

public:
    const std::string& GetName() const override {
        static std::string name("ScoreSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        EntityManager<Components...>& entities = engine->GetEntityManager();
        ComponentStorage<ScoreComponent>& scores =
            entities.template GetComponentStorage<ScoreComponent>();

        // update every score display
        for (size_t i = 0; i < scores.Size(); i++) {
            if (!scores.IsValid(i)) {
                continue;
            }

            ScoreComponent& score = scores.Get(i);

            // never show below zero
            int shownScore = score.score;
            if (shownScore < 0) {
                shownScore = 0;
            }

            // split the score into digits
            int hundredsDigit = (shownScore / 100) % 10;
            int tensDigit = (shownScore / 10) % 10;
            int onesDigit = shownScore % 10;

            ShowDigitSprite(entities, score.digitID[0], score.digitTextureID[hundredsDigit]);
            ShowDigitSprite(entities, score.digitID[1], score.digitTextureID[tensDigit]);
            ShowDigitSprite(entities, score.digitID[2], score.digitTextureID[onesDigit]);
        }
        return kContinue;
    }
};

}  // namespace ECSEngine
