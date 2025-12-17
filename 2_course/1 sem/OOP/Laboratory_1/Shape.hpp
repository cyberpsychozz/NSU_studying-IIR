#pragma once
#include <iostream>

class Shape {
public:
    virtual void Draw() const = 0;
    virtual ~Shape() {}
};