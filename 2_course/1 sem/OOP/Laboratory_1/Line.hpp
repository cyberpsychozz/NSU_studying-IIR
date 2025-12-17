#pragma once
#include "Shape.hpp"


class Line : public Shape{
private:
    Point _point1, _point2;
public:
    Line(const Point& p1 = (0,0), const Point& p2 = (0,0)) : _point1(p1), _point2(p2) {}

    Point GetPoint1() const { return _point1; }
    Point GetPoint2() const { return _point2; }

    void SetPoint1(const Point& p1) { _point1 = p1; }
    void SetPoint2(const Point& p2) { _point2 = p2; }

    void Draw() const override{
        std::cout << "line at (" << _point1.GetX() << ", " << _point1.GetY() <<"), " <<"(" << _point2.GetX() << ", " << _point2.GetY() <<")\n";
    }
};