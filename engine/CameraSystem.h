#pragma once

#include <algorithm>

#include "CameraComponent.h"
#include "CameraFollowerComponent.h"
#include "CameraShakeComponent.h"
#include "CollisionComponent.h"
#include "Entity.h"
#include "EntityManager.h"
#include "LocationComponent.h"
#include "SystemManager.h"

namespace ECSEngine {

namespace CameraSystemDetail {

constexpr float kHardMargin = 0.1f;
constexpr float kSoftMargin = 0.3f;
constexpr float kDefaultFollowSpeed = 0.1f;

inline float TrackHorizontalPosition(float cameraX, float targetX, float worldUnitsPerPixel,
                                     float screenWidth, float followSpeed) {
    if (worldUnitsPerPixel <= 0.0f || screenWidth <= 0.0f) {
        return cameraX;
    }

    const float screenCenter = screenWidth / 2.0f;
    const float targetScreenX =
        screenCenter + (targetX - cameraX) / worldUnitsPerPixel;
    const float targetScreenRatio = targetScreenX / screenWidth;

    float desiredRatio = targetScreenRatio;
    bool hardCorrection = false;
    if (targetScreenRatio < kHardMargin) {
        desiredRatio = kHardMargin;
        hardCorrection = true;
    } else if (targetScreenRatio > 1.0f - kHardMargin) {
        desiredRatio = 1.0f - kHardMargin;
        hardCorrection = true;
    } else if (targetScreenRatio < kSoftMargin) {
        desiredRatio = kSoftMargin;
    } else if (targetScreenRatio > 1.0f - kSoftMargin) {
        desiredRatio = 1.0f - kSoftMargin;
    } else {
        return cameraX;
    }

    const float desiredCameraX =
        targetX - (desiredRatio * screenWidth - screenCenter) * worldUnitsPerPixel;
    if (hardCorrection) {
        return desiredCameraX;
    }

    const float followFactor =
        std::clamp(followSpeed > 0.0f ? followSpeed : kDefaultFollowSpeed, 0.0f, 1.0f);
    return cameraX + (desiredCameraX - cameraX) * followFactor;
}

inline Point2D AdvanceShake(CameraShakeComponent& shake) {
    if (shake.frameLimit <= 0) {
        return Point2D{};
    }
    const float direction = shake.frameLimit % 2 == 0 ? 1.0f : -1.0f;
    --shake.frameLimit;
    return Point2D(direction * shake.offset.x, direction * shake.offset.y);
}

}  // namespace CameraSystemDetail

template <typename... Components>
class CameraSystem : public System<Components...> {
public:
    // Returns this system's stable name.
    const std::string& GetName() const override {
        static std::string name = "CameraSystem";
        return name;
    }

    // Tracks camera targets and advances any active shake for one frame.
    tSystemReturn Run(EngineView<Components...>* engine) override {
        using namespace CameraSystemDetail;

        auto& entities = engine->GetEntityManager();
        auto& windowManager = engine->GetWindowManager();
        const Rect windowBounds = windowManager.GetWindowBounds();
        const float screenWidth = windowBounds.width;

        for (const Entity& entity : entities) {
            const EntityID cameraID = entity.GetID();
            if (!entities.IsEntityIDValid(cameraID) ||
                !entities.template HasComponent<CameraComponent>(cameraID) ||
                !entities.template HasComponent<CameraFollowerComponent>(cameraID)) {
                continue;
            }

            CameraComponent& camera =
                entities.template GetComponent<CameraComponent>(cameraID);
            const EntityID targetID =
                entities.template GetComponent<CameraFollowerComponent>(cameraID).entityID;
            if (!entities.IsEntityIDValid(targetID) ||
                !entities.template HasComponent<LocationComponent>(targetID)) {
                continue;
            }

            if (camera.worldUnitsPerPixel > 0.0f) {
                windowManager.SetWorldScale(camera.worldUnitsPerPixel);
            } else {
                camera.worldUnitsPerPixel = windowManager.GetWorldScale();
            }

            const LocationComponent& targetLocation =
                entities.template GetComponent<LocationComponent>(targetID);
            float targetX = targetLocation.loc.x;
            if (entities.template HasComponent<CollisionComponent>(targetID)) {
                const Rect& targetBounds =
                    entities.template GetComponent<CollisionComponent>(targetID).collisionBox;
                targetX += targetBounds.topLeft.x + targetBounds.width / 2.0f;
            }

            camera.position.x =
                TrackHorizontalPosition(camera.position.x, targetX,
                                        camera.worldUnitsPerPixel, screenWidth, camera.speed);

            Point2D shakeOffset{};
            if (entities.template HasComponent<CameraShakeComponent>(cameraID)) {
                CameraShakeComponent& shake =
                    entities.template GetComponent<CameraShakeComponent>(cameraID);
                shakeOffset = AdvanceShake(shake);
            }

            windowManager.SetWorldCenter(
                Point2D(camera.position.x + shakeOffset.x, camera.position.y + shakeOffset.y));
        }

        return kContinue;
    }
};

}  // namespace ECSEngine
