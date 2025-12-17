#pragma once
#include "Shape.hpp"


class Circle : public Shape{
private:
    Point _center;
    int _radius;
public:
    Circle(const Point& cent = (0,0), const int rad = 0) : _center(cent), _radius(rad) {}

    Point Getcenter() const { return _center; }
    int Getradius() const { return _radius; }

    void SetCenter(const Point& cent) { _center = cent; }
    void SetRadius(const int rad) { _radius = rad; }

    void Draw() const override{
        std::cout << "circle at (" << _center.GetX() << ", " << _center.GetY() <<"), rad = " << _radius << "\n";
    }
};