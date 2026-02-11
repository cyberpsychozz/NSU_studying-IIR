// #include <iostream>
// #include <fstream>
// #include <vector>
// #include <sstream>
// #include "Point.hpp"
// #include "Rect.hpp"
// #include "Line.hpp"
// #include "Circle.hpp"

// using namespace std;

// enum class myFigure {
//     POINT = 1,
//     RECT = 2,
//     LINE = 3,
//     CIRCLE = 4,
//     ERROR = 0
// };

// struct ParsedResult {
//     myFigure typeFig;
//     vector<Point> points;
//     int radius;
// };

// ParsedResult Parse_line(string input) {
//     ParsedResult result;
//     stringstream ss(input);
//     string token;
//     if (!(ss >> token)) {
//         result.typeFig = myFigure::ERROR;
//         return result;
//     }
//     if (token == "point") {
//         result.typeFig = myFigure::POINT;
//         string strX, strY;
//         if (!(ss >> strX >> strY)) {
//             result.typeFig = myFigure::ERROR;
//             return result;
//         }
//         Point point;
//         point.SetX(stoi(strX));
//         point.SetY(stoi(strY));
//         result.points.push_back(point);
//         return result;
//     } else if (token == "rect") {
//         result.typeFig = myFigure::RECT;
//         string p1X, p1Y, p2X, p2Y;
//         if (!(ss >> p1X >> p1Y >> p2X >> p2Y)) {
//             result.typeFig = myFigure::ERROR;
//             return result;
//         }
//         Point p1, p2;
//         p1.SetX(stoi(p1X));
//         p1.SetY(stoi(p1Y));
//         p2.SetX(stoi(p2X));
//         p2.SetY(stoi(p2Y));
//         result.points.push_back(p1);
//         result.points.push_back(p2);
//         return result;
//     } else if (token == "line") {
//         result.typeFig = myFigure::LINE;
//         string p1X, p1Y, p2X, p2Y;
//         if (!(ss >> p1X >> p1Y >> p2X >> p2Y)) {
//             result.typeFig = myFigure::ERROR;
//             return result;
//         }
//         Point p1, p2;
//         p1.SetX(stoi(p1X));
//         p1.SetY(stoi(p1Y));
//         p2.SetX(stoi(p2X));
//         p2.SetY(stoi(p2Y));
//         result.points.push_back(p1);
//         result.points.push_back(p2);
//         return result;
//     } else if (token == "circle") {
//         result.typeFig = myFigure::CIRCLE;
//         string strX, strY, strRad;
//         if (!(ss >> strX >> strY >> strRad)) {
//             result.typeFig = myFigure::ERROR;
//             return result;
//         }
//         Point center;
//         center.SetX(stoi(strX));
//         center.SetY(stoi(strY));
//         result.radius = stoi(strRad);
//         result.points.push_back(center);
//         return result;
//     } else {
//         result.typeFig = myFigure::ERROR;
//         return result;
//     }
// }

// int main() {
//     ifstream in("input.txt");
//     if (in.is_open()) {
//         string str;
//         while (getline(in, str)) {
//             ParsedResult result = Parse_line(str);
//             switch (result.typeFig) {
//                 case myFigure::POINT:
//                     result.points[0].Draw();
//                     break;
//                 case myFigure::RECT:
//                     {
//                         Rect rect(result.points[0], result.points[1]);
//                         rect.Draw();
//                     }
//                     break;
//                 case myFigure::LINE:
//                     {
//                         Line line(result.points[0], result.points[1]);
//                         line.Draw();
//                     }
//                     break;
//                 case myFigure::CIRCLE:
//                     {
//                         Circle circle;
//                         circle.SetCenter(result.points[0]);
//                         circle.SetRadius(result.radius);
//                         circle.Draw();
//                     }
//                     break;
//                 case myFigure::ERROR:
//                     if (!str.empty()) {
//                         cout << "Invalid figure type or parameters in line: \"" << str << "\"" << endl;
//                     }
//                     break;
//             }
//         }
//     }
//     in.close();
//     return 0;
// }


#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include "Shape.hpp"
#include "Point.hpp"
#include "Rect.hpp"
#include "Line.hpp"
#include "Circle.hpp"

using namespace std;

enum class myFigure {
    POINT = 1,
    RECT = 2,
    LINE = 3,
    CIRCLE = 4,
    ERROR = 0
};

Shape* Parse_line(string input) {
    stringstream ss(input);
    string token;
    if (!(ss >> token)) {
        return nullptr;
    }
    if (token == "point") {
        string strX, strY;
        if (!(ss >> strX >> strY)) {
            return nullptr;
        }
        char* endptr;
        long x = strtol(strX.c_str(), &endptr, 10);
        if (*endptr != '\0') return nullptr;
        long y = strtol(strY.c_str(), &endptr, 10);
        if (*endptr != '\0') return nullptr;
        return new Point(x, y);
    } else if (token == "rect") {
        string p1X, p1Y, p2X, p2Y;
        if (!(ss >> p1X >> p1Y >> p2X >> p2Y)) {
            return nullptr;
        }
        char* endptr;
        long x1 = strtol(p1X.c_str(), &endptr, 10);
        if (*endptr != '\0') return nullptr;
        long y1 = strtol(p1Y.c_str(), &endptr, 10);
        if (*endptr != '\0') return nullptr;
        long x2 = strtol(p2X.c_str(), &endptr, 10);
        if (*endptr != '\0') return nullptr;
        long y2 = strtol(p2Y.c_str(), &endptr, 10);
        if (*endptr != '\0') return nullptr;
        return new Rect(Point(x1, y1), Point(x2, y2));
    } else if (token == "line") {
        string p1X, p1Y, p2X, p2Y;
        if (!(ss >> p1X >> p1Y >> p2X >> p2Y)) {
            return nullptr;
        }
        char* endptr;
        long x1 = strtol(p1X.c_str(), &endptr, 10);
        if (*endptr != '\0') return nullptr;
        long y1 = strtol(p1Y.c_str(), &endptr, 10);
        if (*endptr != '\0') return nullptr;
        long x2 = strtol(p2X.c_str(), &endptr, 10);
        if (*endptr != '\0') return nullptr;
        long y2 = strtol(p2Y.c_str(), &endptr, 10);
        if (*endptr != '\0') return nullptr;
        return new Line(Point(x1, y1), Point(x2, y2));
    } else if (token == "circle") {
        string strX, strY, strRad;
        if (!(ss >> strX >> strY >> strRad)) {
            return nullptr;
        }
        char* endptr;
        long x = strtol(strX.c_str(), &endptr, 10);
        if (*endptr != '\0') return nullptr;
        long y = strtol(strY.c_str(), &endptr, 10);
        if (*endptr != '\0') return nullptr;
        long rad = strtol(strRad.c_str(), &endptr, 10);
        if (*endptr != '\0') return nullptr;
        return new Circle(Point(x, y), rad);
    }
    return nullptr;
}

int main() {
    ifstream in("input.txt");
    vector<Shape*> shapes;
    if (in.is_open()) {
        string str;
        while (getline(in, str)) {
            Shape* shape = Parse_line(str);
            if (shape) {
                shapes.push_back(shape);
            } else if (!str.empty()) {
                cout << "Invalid figure type or parameters in line: \"" << str << "\"" << endl;
            }
        }
    }
    in.close();
    for (const auto& shape : shapes) {
        shape->Draw();
    }
    for (auto& shape : shapes) {
        delete shape;
    }
    return 0;
}