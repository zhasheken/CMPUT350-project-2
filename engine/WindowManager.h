#pragma once

#include <memory>
#include <string>

#include "MathUtil.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

namespace ECSEngine {

inline constexpr unsigned int kFrameRateLimit = 60;


class WindowManager {
public:
    // Creates the render window and initializes a centered camera at 1 world unit per pixel.
    WindowManager(unsigned int width, unsigned int height, const std::string& title);
    // Releases the manager's shared window reference.
    ~WindowManager() = default;

    // Returns a shared owner of the window; the window remains alive while any copy is held.
    std::shared_ptr<sf::RenderWindow> GetWindow() const;

    // Returns the screen-space window bounds, with origin (0, 0).
    Rect GetWindowBounds() const;

    // Sets the camera so worldPt maps to screenPt.
    void SetCamera(const Point2D& worldPt, const Point2D& screenPt);

    // Sets the world-space point mapped to the window center.
    void SetWorldCenter(const Point2D& worldCenter);

    // Sets a world-space view rectangle; the rectangle fits in view and extra space may appear
    // along one axis to preserve the window's aspect ratio.
    void SetWorldRect(const Rect& worldCameraRect);

    // Sets world units per screen pixel; requires a positive value.
    void SetWorldScale(float worldUnitsPerPixel);

    // Returns the world-space point mapped to the window center.
    Point2D GetWorldCenter() const;

    // Returns the current world-units-per-pixel scale.
    float GetWorldScale() const;

    // Converts a scalar distance (not a position) between screen and world units.
    float WindowToWorld(float x) const;
    float WorldToWindow(float x) const;

    // Converts a screen-space rectangle to world coordinates, including its position and size.
    Rect WindowToWorld(const Rect& rect) const;

    // Converts a world-space rectangle to screen coordinates, including its position and size.
    Rect WorldToWindow(const Rect& rect) const;

    // Converts positions using both the camera center and scale.
    Point2D WorldToWindow(const Point2D& pt) const;
    Point2D WindowToWorld(const Point2D& pt) const;

    // Draws a filled world-space rectangle; color channels are clamped to [0, 255].
    void DebugDrawRect(const Rect& r, int red, int green, int blue, int alpha = 255);

    // Draws a framed world-space rectangle; color channels are clamped to [0, 255].
    void DebugFrameRect(const Rect& r, int red, int green, int blue, int alpha = 255);

private:
    Point2D WindowCenter() const;
    std::shared_ptr<sf::RenderWindow> mWindow;
    Point2D mWorldCenter;
    float mWorldUnitsPerPixel;
};

}  // namespace ECSEngine
