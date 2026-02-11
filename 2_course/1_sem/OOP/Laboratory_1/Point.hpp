#pragma once
#include "Shape.hpp"
class Point : public Shape{
private:
    int _x, _y;
public:   Point(int xVal = 0, int yVal = 0) : _x(xVal), _y(yVal){}

    int GetX() const{ return _x; }
    int GetY() const{ return _y; }

    void SetX(int xVal) { _x = xVal; }
    void SetY(int yVal) { _y = yVal; }

    void Draw() const override{
        std::cout << "point at (" << _x << ", " << _y <<")\n";
    }
};