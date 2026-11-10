#include "WindowManager.h"

#include <algorithm>
#include <cassert>
#include <cstdint>

namespace ECSEngine {

namespace {
std::uint8_t ToByte(int value) { return static_cast<std::uint8_t>(std::clamp(value, 0, 255)); }
}  // namespace

WindowManager::WindowManager(unsigned int width, unsigned int height, const std::string& title)
    : mWindow(std::make_shared<sf::RenderWindow>(sf::VideoMode({width, height}), title)),
      mWorldCenter(static_cast<float>(width) / 2.0f, static_cast<float>(height) / 2.0f),
      mWorldUnitsPerPixel(1.0f) {
    mWindow->setFramerateLimit(kFrameRateLimit);
}

std::shared_ptr<sf::RenderWindow> WindowManager::GetWindow() const { return mWindow; }

Point2D WindowManager::WindowCenter() const {
    const sf::Vector2u size = mWindow->getSize();
    return Point2D(static_cast<float>(size.x) / 2.0f, static_cast<float>(size.y) / 2.0f);
}

Rect WindowManager::GetWindowBounds() const {
    const sf::Vector2u size = mWindow->getSize();
    return Rect{Point2D(0.0f, 0.0f), static_cast<float>(size.x), static_cast<float>(size.y)};
}

void WindowManager::SetWorldCenter(const Point2D& worldCenter) { mWorldCenter = worldCenter; }

Point2D WindowManager::GetWorldCenter() const { return mWorldCenter; }

void WindowManager::SetWorldScale(float worldUnitsPerPixel) {
    assert(worldUnitsPerPixel > 0.0f);
    mWorldUnitsPerPixel = worldUnitsPerPixel;
}

float WindowManager::GetWorldScale() const { return mWorldUnitsPerPixel; }

void WindowManager::SetCamera(const Point2D& worldPt, const Point2D& screenPt) {
    const Point2D c = WindowCenter();
    mWorldCenter = Point2D(worldPt.x - (screenPt.x - c.x) * mWorldUnitsPerPixel,
                           worldPt.y - (screenPt.y - c.y) * mWorldUnitsPerPixel);
}

void WindowManager::SetWorldRect(const Rect& worldCameraRect) {
    const Rect bounds = GetWindowBounds();
    assert(bounds.width > 0.0f && bounds.height > 0.0f);
    assert(worldCameraRect.width > 0.0f && worldCameraRect.height > 0.0f);

    mWorldCenter = Point2D(worldCameraRect.topLeft.x + worldCameraRect.width / 2.0f,
                           worldCameraRect.topLeft.y + worldCameraRect.height / 2.0f);
    mWorldUnitsPerPixel = std::max(worldCameraRect.width / bounds.width,
                                   worldCameraRect.height / bounds.height);
}

float WindowManager::WindowToWorld(float x) const { return x * mWorldUnitsPerPixel; }

float WindowManager::WorldToWindow(float x) const { return x / mWorldUnitsPerPixel; }

Point2D WindowManager::WindowToWorld(const Point2D& pt) const {
    const Point2D c = WindowCenter();
    return Point2D(mWorldCenter.x + WindowToWorld(pt.x - c.x),
                   mWorldCenter.y + WindowToWorld(pt.y - c.y));
}

Point2D WindowManager::WorldToWindow(const Point2D& pt) const {
    const Point2D c = WindowCenter();
    return Point2D(c.x + WorldToWindow(pt.x - mWorldCenter.x),
                   c.y + WorldToWindow(pt.y - mWorldCenter.y));
}

Rect WindowManager::WindowToWorld(const Rect& rect) const {
    return Rect{WindowToWorld(rect.topLeft), WindowToWorld(rect.width), WindowToWorld(rect.height)};
}

Rect WindowManager::WorldToWindow(const Rect& rect) const {
    return Rect{WorldToWindow(rect.topLeft), WorldToWindow(rect.width), WorldToWindow(rect.height)};
}

void WindowManager::DebugDrawRect(const Rect& r, int red, int green, int blue, int alpha) {
    const Rect w = WorldToWindow(r);
    sf::RectangleShape shape(sf::Vector2f(w.width, w.height));
    shape.setPosition(sf::Vector2f(w.topLeft.x, w.topLeft.y));
    shape.setFillColor(sf::Color(ToByte(red), ToByte(green), ToByte(blue), ToByte(alpha)));
    mWindow->draw(shape);
}

void WindowManager::DebugFrameRect(const Rect& r, int red, int green, int blue, int alpha) {
    const Rect w = WorldToWindow(r);
    sf::RectangleShape shape(sf::Vector2f(w.width, w.height));
    shape.setPosition(sf::Vector2f(w.topLeft.x, w.topLeft.y));
    shape.setFillColor(sf::Color::Transparent);
    shape.setOutlineColor(sf::Color(ToByte(red), ToByte(green), ToByte(blue), ToByte(alpha)));
    shape.setOutlineThickness(-1.0f); 
    mWindow->draw(shape);
}

}  // namespace ECSEngine