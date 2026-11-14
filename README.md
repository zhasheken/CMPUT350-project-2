# CMPUT 350 Project 2: ECS Platformer

## Implementation Notes

### Entities and components

`EntityManager` stores entities in a vector and reuses removed entity IDs from a free list. Each entity has a component-index table; `kNullComponent` means the entity does not have that component type.

`ComponentStorage` keeps logical slots in vectors, tracks live slots and owners separately, and reuses freed slots instead of compacting storage.

> **Invalidation rules:** a vector reallocation can invalidate component references, and an entity ID can identify a different entity after reuse.

### Collision

`CollisionComponent.collisionBox` is relative to `LocationComponent.loc`. `CollisionLateSystem` records the translated world-space box before movement; `CollisionSystem` compares it with the post-movement box to resolve contacts.

### Movement and gravity

Movement and gravity are discrete per-update values. Player input accelerates toward a run speed limit and supports variable-height jumps, wall jumps, and a press-triggered dash.

| Constant                  | Value      |
| ------------------------- | ---------- |
| Run speed                 | 7          |
| Jump velocity             | 18         |
| Dash velocity             | 40         |
| Ground / air acceleration | 4 / 3      |
| Dash cooldown             | 60 updates |

`GravitySystem` doubles gravity while falling, triples it while fast falling, caps fall speed, and limits wall sliding to 1 pixel per update.

Wall jumps use coyote time: a wall jump is still allowed for 6 updates after leaving a wall.

When left and right are both held, the most recently pressed arrow wins.

Dashing while falling cancels downward velocity.

### Stars

Stars are dynamic entities without an `InputComponent`. `SpawnSystem` gives them a small random upward launch (5 pixels per update) and a downward `GravityComponent`. `CollisionSystem` handles star bounces, star-star collisions, and deferred collection/removal. One extra star is placed outside the last wall. It does not move and has no gravity.

- Bounce restitution: **0.7**
- Stars settle when reflected speed is below **1 pixel per update**

### Sound and sprites

- **`SoundManager`** uses one map for `sf::SoundBuffer` ownership and a second map for `sf::Sound` objects that refer to those buffers.
- **`SpriteManager`** caches textures by path and stores `sf::Sprite` objects in a `deque`, so adding sprites preserves references to existing sprites. Re-registering a texture path reuses its texture but returns a new sprite ID.

### Map loading and assets

The map loader loads `sky.map` before `world.map` so background sprites are added first. CMake copies the whole `assets/` directory into the build directory. The map files and sprite/audio assets are in `assets/`.

---

## System Access and Update Order

Systems take `EngineView` rather than `ECSEngine` and obtain managers through its manager getters. The systems in `main.cpp` are registered and run in this order:

| #   | System                | Purpose                                                    |
| --- | --------------------- | ---------------------------------------------------------- |
| 1   | `CollisionLateSystem` | Snapshots collision boxes before anything moves            |
| 2   | `InputSystem`         | Updates player movement intent                             |
| 3   | `GravitySystem`       | Applies acceleration and fall limits                       |
| 4   | `MovementSystem`      | Changes entity locations                                   |
| 5   | `CollisionSystem`     | Resolves obstacles, handles stars, collects stars          |
| 6   | `ScoreSystem`         | Updates the score display after collection                 |
| 7   | `CameraSystem`        | Follows the player and applies camera shake                |
| 8   | `SpriteSystem`        | Draws world-space and screen-space sprites                 |
| 9   | `SpawnSystem`         | Counts down spawners and creates stars for the next update |

Boxes are saved before anything moves and resolved after movement. Spawning runs last, so new stars appear on the next frame.

`SystemManager` owns its systems, runs them in insertion order, and stops immediately when one returns `kExit`.

### Engine input and debug controls

`ECSEngine` snapshots `keydown` into `lastKeydown` at the start of each frame, then polls events and updates `keydown` on every `InputComponent`. Losing window focus clears held key state.

| Key | Action                         |
| --- | ------------------------------ |
| `D` | Toggle collision debug drawing |
| `P` | Pause / resume                 |
| `N` | Advance one frame while paused |

---

## Team Contributions

The following primary areas are based on the Git commit history:

- **Harveer Deol:** sound, sprite, and window managers; component-header separation; input, collision, camera, and spawn systems; and related tests and integration.
- **Zhasheken de Guia:** entity manager, system manager, component storage, `ECSEngine` view, map loader, movement, gravity, sprite, score, collision systems, movement tuning and related tests and integration.

These are primary contributions, not exclusive ownership. Both team members shared responsibility.

---

## Build and Run

From the repository root:

```sh
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
(cd build/platformer && ./platformer)
```
