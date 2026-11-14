#ifndef MATHUTIL_H
#define MATHUTIL_H

#include <iostream>

namespace ECSEngine {

#include <cstdio>

struct Point2D {
    float x, y;
    // Constructs a point from floating-point coordinates.
    Point2D(float x = 0, float y = 0) : x(x), y(y) {}
    // Constructs a point from integer coordinates.
    Point2D(int x, int y) : x(x), y(y) {}
};

// Writes a point as "(x, y)" to os and returns os.
[[maybe_unused]] static std::ostream& operator<<(std::ostream& os, const Point2D& p) {
    os << "(" << p.x << ", " << p.y << ")";
    return os;
}

// Multiplies both coordinates by number.
[[maybe_unused]] static Point2D operator*(float number, const Point2D& rhs) {
    return Point2D(number * rhs.x, number * rhs.y);
}

struct Line {
    Point2D p1, p2;

    // Constructs a line from endpoints, defaulting both to the origin.
    Line(Point2D p1 = {0, 0}, Point2D p2 = {0, 0}) : p1(p1), p2(p2) {}
    // Constructs a line from endpoint coordinates.
    Line(float x1, float y1, float x2, float y2) : p1(x1, y1), p2(x2, y2) {}
};

// Writes a line and its endpoints to os and returns os.
[[maybe_unused]] static std::ostream& operator<<(std::ostream& os, const Line& l) {
    os << "L{" << l.p1 << ", " << l.p2 << "}";
    return os;
}

struct Circle {
    Point2D center;
    float radius;

    // Constructs a circle from a center point and radius, defaulting to the origin.
    Circle(Point2D c = {0, 0}, float r = 0) : center(c), radius(r) {}
    // Constructs a circle from center coordinates and radius.
    Circle(float x, float y, float r) : center(x, y), radius(r) {}
};

struct Rect {
    Point2D topLeft;
    float width, height;
};

// Writes a rectangle's origin and dimensions to os and returns os.
[[maybe_unused]] static std::ostream& operator<<(std::ostream& os, const Rect& r) {
    os << "R{" << r.topLeft << ", " << r.width << "x" << r.height << "}";
    return os;
}

}  // namespace ECSEngine

#endif  // MATHUTIL_H
