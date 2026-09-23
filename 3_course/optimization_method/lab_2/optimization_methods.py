"""Одномерная минимизация функции exp(x) + x**2 на отрезке [-1, 0]."""

from dataclasses import dataclass
from math import exp, sqrt
from typing import Callable


Function = Callable[[float], float]


@dataclass(frozen=True)
class OptimizationResult:
    """Результат работы метода одномерной минимизации."""

    x_min: float
    f_min: float
    iterations: int
    function_calls: int
    derivative_calls: int = 0


def _check_arguments(a: float, b: float, eps: float) -> None:
    if a >= b:
        raise ValueError("Левая граница должна быть меньше правой")
    if eps <= 0:
        raise ValueError("Точность eps должна быть положительной")


def dichotomy(
    function: Function,
    derivative: Function,
    a: float,
    b: float,
    eps: float,
) -> OptimizationResult:
    """Находит минимум дихотомией первого порядка.

    На каждой итерации отрезок делится пополам, после чего по знаку
    производной выбирается его половина, содержащая точку минимума.
    """

    _check_arguments(a, b, eps)
    iterations = 0
    derivative_calls = 0

    while b - a > 2 * eps:
        x_middle = (a + b) / 2
        derivative_value = derivative(x_middle)
        derivative_calls += 1

        if derivative_value > 0:
            b = x_middle
        else:
            a = x_middle
        iterations += 1

    x_min = (a + b) / 2
    f_min = function(x_min)
    return OptimizationResult(x_min, f_min, iterations, 1, derivative_calls)


def chord_method(
    function: Function,
    derivative: Function,
    a: float,
    b: float,
    eps: float,
    max_iterations: int = 1_000,
) -> OptimizationResult:
    """Находит минимум методом хорд, решая уравнение f'(x) = 0.

    Новая точка является пересечением оси Ox с хордой, проведённой через
    значения производной на концах текущего интервала. Останавливаемся,
    когда два последовательных приближения отличаются не более чем на eps.
    """

    _check_arguments(a, b, eps)
    derivative_left = derivative(a)
    derivative_right = derivative(b)
    derivative_calls = 2

    if derivative_left > 0 or derivative_right < 0:
        raise ValueError("Производная должна менять знак с минуса на плюс")

    previous_x: float | None = None

    for iteration in range(1, max_iterations + 1):
        denominator = derivative_right - derivative_left
        if denominator == 0:
            raise ArithmeticError("Невозможно построить хорду")

        x_new = (
            a * derivative_right - b * derivative_left
        ) / denominator
        derivative_new = derivative(x_new)
        derivative_calls += 1

        if previous_x is not None and abs(x_new - previous_x) <= eps:
            return OptimizationResult(
                x_new,
                function(x_new),
                iteration,
                1,
                derivative_calls,
            )

        if derivative_new > 0:
            b, derivative_right = x_new, derivative_new
        else:
            a, derivative_left = x_new, derivative_new
        previous_x = x_new

    raise RuntimeError(f"Метод хорд не сошёлся за {max_iterations} итераций")


def golden_section(
    function: Function,
    a: float,
    b: float,
    eps: float,
) -> OptimizationResult:
    """Находит минимум методом золотого сечения."""

    _check_arguments(a, b, eps)
    ratio = (sqrt(5) - 1) / 2
    x_left = b - ratio * (b - a)
    x_right = a + ratio * (b - a)
    f_left = function(x_left)
    f_right = function(x_right)
    function_calls = 2
    iterations = 0

    while (b - a) / 2 > eps:
        if f_left <= f_right:
            b = x_right
            x_right, f_right = x_left, f_left
            x_left = b - ratio * (b - a)
            f_left = function(x_left)
        else:
            a = x_left
            x_left, f_left = x_right, f_right
            x_right = a + ratio * (b - a)
            f_right = function(x_right)

        function_calls += 1
        iterations += 1

    x_min = (a + b) / 2
    f_min = function(x_min)
    return OptimizationResult(x_min, f_min, iterations, function_calls + 1)


def parabolic(
    function: Function,
    a: float,
    b: float,
    eps: float,
    max_iterations: int = 1_000,
) -> OptimizationResult:
    """Находит минимум методом параболической интерполяции.

    Начальные точки: границы отрезка и его середина. Функция должна быть
    унимодальной, а значение в средней точке не должно превышать значения
    на границах.
    """

    _check_arguments(a, b, eps)
    x_left, x_middle, x_right = a, (a + b) / 2, b
    f_left = function(x_left)
    f_middle = function(x_middle)
    f_right = function(x_right)
    function_calls = 3

    if f_middle > f_left or f_middle > f_right:
        raise ValueError("Начальные точки не заключают минимум")

    for iteration in range(1, max_iterations + 1):
        numerator = (
            (x_middle - x_left) ** 2 * (f_middle - f_right)
            - (x_middle - x_right) ** 2 * (f_middle - f_left)
        )
        denominator = 2 * (
            (x_middle - x_left) * (f_middle - f_right)
            - (x_middle - x_right) * (f_middle - f_left)
        )

        if denominator == 0:
            raise ArithmeticError("Невозможно построить параболу")

        x_new = x_middle - numerator / denominator
        if not x_left < x_new < x_right:
            raise ArithmeticError("Вершина параболы вышла за исходный интервал")

        f_new = function(x_new)
        function_calls += 1

        # Для метода парабол стандартный критерий остановки — близость
        # двух последовательных приближений к точке минимума.
        if abs(x_new - x_middle) <= eps:
            if f_new < f_middle:
                x_middle, f_middle = x_new, f_new
            return OptimizationResult(
                x_middle, f_middle, iteration, function_calls
            )

        # Сохраняем три точки, между крайними из которых заключён минимум.
        if x_new < x_middle:
            if f_new < f_middle:
                x_right, f_right = x_middle, f_middle
                x_middle, f_middle = x_new, f_new
            else:
                x_left, f_left = x_new, f_new
        else:
            if f_new < f_middle:
                x_left, f_left = x_middle, f_middle
                x_middle, f_middle = x_new, f_new
            else:
                x_right, f_right = x_new, f_new

    raise RuntimeError(f"Метод парабол не сошёлся за {max_iterations} итераций")


def objective(x: float) -> float:
    """Целевая функция f(x) = exp(x) + x**2."""

    return exp(x) + x**2


def objective_derivative(x: float) -> float:
    """Производная целевой функции: f'(x) = exp(x) + 2x."""

    return exp(x) + 2 * x


def main() -> None:
    methods = (
        ("Параболы", lambda eps: parabolic(objective, -1.0, 0.0, eps)),
        (
            "Золотое сечение",
            lambda eps: golden_section(objective, -1.0, 0.0, eps),
        ),
        (
            "Дихотомия 1-го порядка",
            lambda eps: dichotomy(
                objective, objective_derivative, -1.0, 0.0, eps
            ),
        ),
        (
            "Хорды",
            lambda eps: chord_method(
                objective, objective_derivative, -1.0, 0.0, eps
            ),
        ),
    )

    print("Минимизация f(x) = exp(x) + x^2 на [-1, 0]")
    for eps in (1e-3, 1e-5):
        print(f"\neps = {eps:g}")
        function_calls_label = "Вызовы f"
        derivative_calls_label = "Вызовы f'"
        print(
            f"{'Метод':<25} {'x_min':>14} {'f_min':>14} "
            f"{'Итерации':>10} {function_calls_label:>10} "
            f"{derivative_calls_label:>11} {'Всего':>7}"
        )
        for name, run_method in methods:
            result = run_method(eps)
            total_calls = result.function_calls + result.derivative_calls
            print(
                f"{name:<25} {result.x_min:>14.9f} "
                f"{result.f_min:>14.9f} {result.iterations:>10} "
                f"{result.function_calls:>10} "
                f"{result.derivative_calls:>11} {total_calls:>7}"
            )


if __name__ == "__main__":
    main()
