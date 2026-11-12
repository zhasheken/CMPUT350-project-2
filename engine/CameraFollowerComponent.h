#pragma once

#include "Entity.h"

namespace ECSEngine {

struct CameraFollowerComponent {
    EntityID entityID{};  // The entity that this camera follows
};

}  // namespace ECSEngine
