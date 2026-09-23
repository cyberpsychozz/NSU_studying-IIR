#include <stdio.h>
#include <math.h>

int calls = 0;

double func(double x) {
    calls++;
    return (exp(x) + x * x);
}

int main() {
    double epsilon;
    double L;
    double a;
    double b;

    printf("Введите epsilon: ");
    scanf("%lf", &epsilon);

    printf("Введите константу Липшица L: ");
    scanf("%lf", &L);

    printf("Введите левую границу a: ");
    scanf("%lf", &a);

    printf("Введите правую границу b: ");
    scanf("%lf", &b);

    if (epsilon <= 0 || L <= 0 || a >= b) {
        printf("Ошибка: нужно epsilon > 0, L > 0 и a < b\n");
        return 1;
    }

    int N = (int)ceil(L * (b - a) / epsilon);
    double delta = (b - a) / N;

    double res_x = a;
    double res = func(res_x);
    double f = 0;

    for (double x = 0; x > -2; x -= delta) {
        f = func(x);
        if (f < res) {
            res = f;
            res_x = x;
        }
        else break; 
    }

    printf ("Количество разбиений: %d\n", N);
    printf ("Шаг delta: %f\n", delta);
    printf ("Минимальное значение функции: %f\n", res);
    printf ("Аргумент в точке минимума: %f\n", res_x);
    printf ("Количество вызовов функции: %d\n", calls);

    return 0;
}
