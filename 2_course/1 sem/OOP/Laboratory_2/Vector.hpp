#pragma once

#include <iostream>
#include <cmath>
using namespace std;

class Vector{
private:
    double _x, _y;
public: 
// Конструкторы
    Vector() : _x(0.0), _y(0.0) {}
    explicit Vector(double x, double y) : _x(x), _y(y){}
    Vector (const Vector &that) : _x(that._x), _y(that._y){}

// Оператор присваивания
    Vector & operator = (const Vector &that){
        if(this != &that){
            _x = that._x;
            _y = that._y;
        }
        return *this;
    }

// Создание вектора в полярных координатах
    static Vector makePolar(double rad, double alpha) {
    return Vector(rad * cos(alpha), rad * sin(alpha));
}
// Сеттеры и геттеры
    double x(void) const {return _x;}
    double y(void) const {return _y;}
    void x(double newX){_x = newX;}
    void y(double newY){_y = newY;}

// Арифметические операторы
    Vector operator+(const Vector &that) const {
        return Vector(_x + that._x, _y + that._y);
    }
    Vector operator-(const Vector &that) const {
        return Vector(_x - that._x, _y - that._y);
    }
    double operator*(const Vector &that) const {
        return (_x * that._x + _y * that._y);
    }
    Vector operator*(const double &that) const {
        return Vector(_x * that, _y * that);
    }
    Vector operator/(const double &that) const {
        if (that == 0.0) {
            return Vector(0.0, 0.0);
        }
        return Vector(_x / that, _y / that);
    }


// Операторы присваивания
    Vector & operator += (const Vector & that){
        _x += that._x;
        _y += that._y;
        return *this;
    }

    Vector & operator -= (const Vector & that){
        _x -= that._x;
        _y -= that._y;
        return *this;
    }

    Vector & operator *= (const double & that){
        _x *= that;
        _y *= that;
        return *this;
    }

    Vector & operator /= (const double & that){
        _x /= that;
        _y /= that;
        return *this;
    }

// Операция отрицания вектора
    Vector operator-() const {
        return Vector(-_x, -_y);
    }

// Операция проверки на равенство
    bool operator == (const Vector & that){
        return _x == that._x && _y == that._y;
    }
// Оператор проверки на неравенство
    bool operator != (const Vector & that){
        return _x != that._x || _y != that._y;
    }
    
// Методы работы с вектором
    Vector &rotate (double angle){
        double temp_x = _x;
        double temp_y = _y;
        _x = temp_x * cos(angle) - temp_y * sin(angle);
        _y = temp_y * cos(angle) + temp_x * sin(angle);
        return *this;
    }

    Vector &rotate(int quad){
        int k = quad % 4;
        if (k < 0) k += 4;

        double temp_x = _x;
        double temp_y = _y;
        
        switch (k) {
            case 0: // 0° 
                break;
            case 1: // 90° 
                _x = temp_y;
                _y = -temp_x;
                break;
            case 2: // 180°
                _x = -temp_x;
                _y = -temp_y;
                break;
            case 3: // 270° 
                _x = -temp_y;
                _y = temp_x;
                break;
        }
        
        return *this;
    }

    double module2(void) const {
        return _x * _x + _y * _y;
    }

    double angle(void) const {
        return atan2(_y, _x);
    }

     double angle(const Vector &that) const {
        double dot_product = x() * that.x() + y() * that.y();
        double len1 = length();
        double len2 = that.length();
        
        if (len1 == 0.0 || len2 == 0.0) {
            return 0.0; 
        }
        
        double cos_theta = dot_product / (len1 * len2);
        if (cos_theta > 1.0) cos_theta = 1.0;
        if (cos_theta < -1.0) cos_theta = -1.0;
        
        return acos(cos_theta);
    }
    
    
    double length() const {
        return sqrt(_x * _x + _y * _y);
    }

    double projection(const Vector &base) const {
        double base_length = base.length();
        if (base_length == 0.0) {
            return 0.0; 
        }
        double dot_product = x() * base.x() + y() * base.y();
        return dot_product / base_length;
    }


    Vector &normalize() {
        double len = length();
        if (len == 0.0) {
            return *this; 
        }
        x(_x / len);
        y(_y / len);
        return *this;
    }

    Vector &transformTo(const Vector &e1, const Vector &e2) {
        double e1_len2 = e1.module2();
        double e2_len2 = e2.module2();
        
        if (e1_len2 == 0.0 || e2_len2 == 0.0){ 
            return *this; 
        }
        
        double a1 = (x() * e1.x() + y() * e1.y()) / e1_len2;
        double a2 = (x() * e2.x() + y() * e2.y()) / e2_len2;
        
        x(a1);
        y(a2);
        return *this;
    }

    
    Vector &transformFrom(const Vector &e1, const Vector &e2) {
        double new_x = x() * e1.x() + y() * e2.x();
        double new_y = x() * e1.y() + y() * e2.y();
        
        x(new_x);
        y(new_y);
        return *this;
    }

};



Vector operator*(const double &lhs, const Vector &rhs) {
    return Vector(lhs * rhs.x(), lhs * rhs.y());
}

Vector rotate(const Vector &v, double angle) {
    double new_x = v.x() * std::cos(angle) - v.y() * std::sin(angle);
    double new_y = v.x() * std::sin(angle) + v.y() * std::cos(angle);
    return Vector(new_x, new_y);
}

Vector rotate(const Vector &v, int quad) {
    int k = quad % 4;
    if (k < 0) k += 4;
    switch (k) {
        case 0: // 0°
            return v;
        case 1: // 90° 
            return Vector(v.y(), -v.x());
        case 2: // 180°
            return Vector(-v.x(), -v.y());
        case 3: // 270° 
            return Vector(-v.y(), v.x());
    }
    return v; 
}

double module2(const Vector &v) {
    return v.x() * v.x() + v.y() * v.y();
}

double length(const Vector &v) {
    return std::sqrt(v.x() * v.x() + v.y() * v.y());
}

double angle(const Vector &v) {
    return std::atan2(v.y(), v.x());
}

double angle(const Vector &v1, const Vector &v2) {
    double dot_product = v1.x() * v2.x() + v1.y() * v2.y();
    double len1 = v1.length();
    double len2 = v2.length();
    if (len1 == 0.0 || len2 == 0.0) {
        return 0.0;
    }
    double cos_theta = dot_product / (len1 * len2);
    if (cos_theta > 1.0) cos_theta = 1.0;
    if (cos_theta < -1.0) cos_theta = -1.0;
    return std::acos(cos_theta);
}

double projection(const Vector &v, const Vector &base) {
    double base_length = base.length();
    if (base_length == 0.0) {
        return 0.0;
    }
    double dot_product = v.x() * base.x() + v.y() * base.y();
    return dot_product / base_length;
}

Vector normalize(const Vector &v) {
    double len = v.length();
    if (len == 0.0) {
        return v;
    }
    return Vector(v.x() / len, v.y() / len);
}

Vector transformTo(const Vector &v, const Vector &e1, const Vector &e2) {
    double e1_len2 = e1.x() * e1.x() + e1.y() * e1.y();
    double e2_len2 = e2.x() * e2.x() + e2.y() * e2.y();
    if (e1_len2 == 0.0 || e2_len2 == 0.0) {
        return v;
    }
    double a1 = (v.x() * e1.x() + v.y() * e1.y()) / e1_len2;
    double a2 = (v.x() * e2.x() + v.y() * e2.y()) / e2_len2;
    return Vector(a1, a2);
}

Vector transformFrom(const Vector &v, const Vector &e1, const Vector &e2) {
    double new_x = v.x() * e1.x() + v.y() * e2.x();
    double new_y = v.x() * e1.y() + v.y() * e2.y();
    return Vector(new_x, new_y);
}