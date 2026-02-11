#include <iostream>
#include <cmath>

using namespace std;

void solution(double a, double b, double c) {
    // Проверка на a == 0
    if (a == 0) {
        if (b == 0) {
            if (c == 0) {
                cout << "Бесконечное множество решений" << endl;
            } else {
                cout << "Нет решений" << endl;
            }
        } else {
            // Линейное уравнение: bx + c = 0
            cout << "Это линейное уравнение, корень: " << -c / b << endl;
        }
        return;
    }

    // Вычисление дискриминанта
    double D = b * b - 4 * a * c;
    if (D < 0) {
        cout << "Нет вещественных решений" << endl;
    } else if (D == 0) {
        double x = -b / (2 * a);
        cout << x << endl;
    } else {
        double x1 = (-b - sqrt(D)) / (2 * a);
        double x2 = (-b + sqrt(D)) / (2 * a);
        cout << x1 << " " << x2 << endl;
    }
}

int main() {
    double a, b, c;
    cout << "Введите коэффициенты A, B, C уравнения в формате (Ax^2 + Bx + C):" << endl;
    
    
    if (!(cin >> a >> b >> c)) {
        cerr << "Ошибка ввода" << endl;
        return 1;
    }

    solution(a, b, c);
    return 0;
}