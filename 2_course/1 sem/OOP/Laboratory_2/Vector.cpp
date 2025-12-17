
#include "Vector.hpp"
#include <gtest/gtest.h>
#include <cmath>

using namespace std;

// Тестовый класс
class VectorTest : public ::testing::Test {
protected:
    const double epsilon = 1e-10; // Погрешность для сравнения double
};

// Тесты конструкторов
TEST_F(VectorTest, DefaultConstructor) {
    Vector v;
    ASSERT_DOUBLE_EQ(v.x(), 0.0);
    ASSERT_DOUBLE_EQ(v.y(), 0.0);
}

TEST_F(VectorTest, ParameterizedConstructor) {
    Vector v(3.0, 4.0);
    ASSERT_DOUBLE_EQ(v.x(), 3.0);
    ASSERT_DOUBLE_EQ(v.y(), 4.0);
}

TEST_F(VectorTest, CopyConstructor) {
    Vector v1(3.0, 4.0);
    Vector v2(v1);
    ASSERT_DOUBLE_EQ(v2.x(), v1.x());
    ASSERT_DOUBLE_EQ(v2.y(), v1.y());
}

TEST_F(VectorTest, AssignmentOperator) {
    Vector v1(3.0, 4.0);
    Vector v2;
    v2 = v1;
    ASSERT_DOUBLE_EQ(v2.x(), v1.x());
    ASSERT_DOUBLE_EQ(v2.y(), v1.y());
}

TEST_F(VectorTest, MakePolar) {
    Vector v = Vector::makePolar(5.0, M_PI / 4);
    ASSERT_NEAR(v.x(), 5.0 * cos(M_PI / 4), epsilon);
    ASSERT_NEAR(v.y(), 5.0 * sin(M_PI / 4), epsilon);
}

// Тесты арифметических операторов
TEST_F(VectorTest, Addition) {
    Vector v1(3.0, 4.0);
    Vector v2(1.0, 1.0);
    Vector result = v1 + v2;
    ASSERT_DOUBLE_EQ(result.x(), 4.0);
    ASSERT_DOUBLE_EQ(result.y(), 5.0);
}

TEST_F(VectorTest, Subtraction) {
    Vector v1(3.0, 4.0);
    Vector v2(1.0, 1.0);
    Vector result = v1 - v2;
    ASSERT_DOUBLE_EQ(result.x(), 2.0);
    ASSERT_DOUBLE_EQ(result.y(), 3.0);
}

TEST_F(VectorTest, DotProduct) {
    Vector v1(3.0, 4.0);
    Vector v2(1.0, 0.0);
    double result = v1 * v2;
    ASSERT_DOUBLE_EQ(result, 3.0);
}

TEST_F(VectorTest, ScalarMultiplication) {
    Vector v(3.0, 4.0);
    Vector result = v * 2.0;
    ASSERT_DOUBLE_EQ(result.x(), 6.0);
    ASSERT_DOUBLE_EQ(result.y(), 8.0);
}

TEST_F(VectorTest, FreeScalarMultiplication) {
    Vector v(3.0, 4.0);
    Vector result = 2.0 * v;
    ASSERT_DOUBLE_EQ(result.x(), 6.0);
    ASSERT_DOUBLE_EQ(result.y(), 8.0);
}

TEST_F(VectorTest, Division) {
    Vector v(6.0, 8.0);
    Vector result = v / 2.0;
    ASSERT_DOUBLE_EQ(result.x(), 3.0);
    ASSERT_DOUBLE_EQ(result.y(), 4.0);
}

// Тесты операторов присваивания
TEST_F(VectorTest, AdditionAssignment) {
    Vector v(3.0, 4.0);
    v += Vector(1.0, 1.0);
    ASSERT_DOUBLE_EQ(v.x(), 4.0);
    ASSERT_DOUBLE_EQ(v.y(), 5.0);
}

TEST_F(VectorTest, SubtractionAssignment) {
    Vector v(3.0, 4.0);
    v -= Vector(1.0, 1.0);
    ASSERT_DOUBLE_EQ(v.x(), 2.0);
    ASSERT_DOUBLE_EQ(v.y(), 3.0);
}

TEST_F(VectorTest, MultiplicationAssignment) {
    Vector v(3.0, 4.0);
    v *= 2.0;
    ASSERT_DOUBLE_EQ(v.x(), 6.0);
    ASSERT_DOUBLE_EQ(v.y(), 8.0);
}

TEST_F(VectorTest, DivisionAssignment) {
    Vector v(6.0, 8.0);
    v /= 2.0;
    ASSERT_DOUBLE_EQ(v.x(), 3.0);
    ASSERT_DOUBLE_EQ(v.y(), 4.0);
}

// Тест унарного минуса
TEST_F(VectorTest, UnaryMinus) {
    Vector v(3.0, 4.0);
    Vector result = -v;
    ASSERT_DOUBLE_EQ(result.x(), -3.0);
    ASSERT_DOUBLE_EQ(result.y(), -4.0);
}

// Тесты сравнения
TEST_F(VectorTest, Equality) {
    Vector v1(3.0, 4.0);
    Vector v2(3.0, 4.0);
    ASSERT_TRUE(v1 == v2);
}

TEST_F(VectorTest, Inequality) {
    Vector v1(3.0, 4.0);
    Vector v2(1.0, 1.0);
    ASSERT_TRUE(v1 != v2);
}

// Тесты методов работы с вектором
TEST_F(VectorTest, RotateDouble) {
    Vector v(1.0, 0.0);
    v.rotate(M_PI / 2);
    ASSERT_NEAR(v.x(), 0.0, epsilon);
    ASSERT_NEAR(v.y(), 1.0, epsilon);
}

TEST_F(VectorTest, RotateQuad) {
    Vector v(1.0, 0.0);
    v.rotate(1); // 90° по часовой стрелке
    ASSERT_DOUBLE_EQ(v.x(), 0.0);
    ASSERT_DOUBLE_EQ(v.y(), -1.0);
    
    v = Vector(1.0, 0.0);
    v.rotate(2); // 180°
    ASSERT_DOUBLE_EQ(v.x(), -1.0);
    ASSERT_DOUBLE_EQ(v.y(), 0.0);
}

TEST_F(VectorTest, Module2) {
    Vector v(3.0, 4.0);
    ASSERT_DOUBLE_EQ(v.module2(), 25.0);
}

TEST_F(VectorTest, Angle) {
    Vector v(1.0, 1.0);
    ASSERT_NEAR(v.angle(), M_PI / 4, epsilon);
}

TEST_F(VectorTest, AngleBetweenVectors) {
    Vector v1(1.0, 0.0);
    Vector v2(0.0, 1.0);
    ASSERT_NEAR(v1.angle(v2), M_PI / 2, epsilon);
    
    Vector zero(0.0, 0.0);
    ASSERT_DOUBLE_EQ(v1.angle(zero), 0.0); // Нулевой вектор
}

TEST_F(VectorTest, Length) {
    Vector v(3.0, 4.0);
    ASSERT_DOUBLE_EQ(v.length(), 5.0);
}

TEST_F(VectorTest, Projection) {
    Vector v(3.0, 4.0);
    Vector base(1.0, 0.0);
    ASSERT_DOUBLE_EQ(v.projection(base), 3.0);
    
    Vector zero(0.0, 0.0);
    ASSERT_DOUBLE_EQ(v.projection(zero), 0.0); // Нулевой базис
}

TEST_F(VectorTest, Normalize) {
    Vector v(3.0, 4.0);
    v.normalize();
    ASSERT_NEAR(v.x(), 0.6, epsilon);
    ASSERT_NEAR(v.y(), 0.8, epsilon);
    ASSERT_NEAR(v.length(), 1.0, epsilon);
    
    Vector zero(0.0, 0.0);
    zero.normalize();
    ASSERT_DOUBLE_EQ(zero.x(), 0.0);
    ASSERT_DOUBLE_EQ(zero.y(), 0.0);
}

TEST_F(VectorTest, TransformTo) {
    Vector v(3.0, 4.0);
    Vector e1(2.0, 0.0);
    Vector e2(0.0, 2.0);
    v.transformTo(e1, e2);
    ASSERT_DOUBLE_EQ(v.x(), 1.5);
    ASSERT_DOUBLE_EQ(v.y(), 2.0);
    
    Vector zero(0.0, 0.0);
    Vector original = v;
    v.transformTo(zero, e2);
    ASSERT_DOUBLE_EQ(v.x(), original.x());
    ASSERT_DOUBLE_EQ(v.y(), original.y());
}

TEST_F(VectorTest, TransformFrom) {
    Vector v(1.5, 2.0);
    Vector e1(2.0, 0.0);
    Vector e2(0.0, 2.0);
    v.transformFrom(e1, e2);
    ASSERT_DOUBLE_EQ(v.x(), 3.0);
    ASSERT_DOUBLE_EQ(v.y(), 4.0);
}

// Тесты свободных функций
TEST_F(VectorTest, FreeRotateDouble) {
    Vector v(1.0, 0.0);
    Vector result = rotate(v, M_PI / 2);
    ASSERT_NEAR(result.x(), 0.0, epsilon);
    ASSERT_NEAR(result.y(), 1.0, epsilon);
    ASSERT_DOUBLE_EQ(v.x(), 1.0); // Исходный вектор не изменился
}

TEST_F(VectorTest, FreeRotateQuad) {
    Vector v(1.0, 0.0);
    Vector result = rotate(v, 1);
    ASSERT_DOUBLE_EQ(result.x(), 0.0);
    ASSERT_DOUBLE_EQ(result.y(), -1.0);
}

TEST_F(VectorTest, FreeModule2) {
    Vector v(3.0, 4.0);
    ASSERT_DOUBLE_EQ(module2(v), 25.0);
}

TEST_F(VectorTest, FreeLength) {
    Vector v(3.0, 4.0);
    ASSERT_DOUBLE_EQ(length(v), 5.0);
}

TEST_F(VectorTest, FreeAngle) {
    Vector v(1.0, 1.0);
    ASSERT_NEAR(angle(v), M_PI / 4, epsilon);
}

TEST_F(VectorTest, FreeAngleBetweenVectors) {
    Vector v1(1.0, 0.0);
    Vector v2(0.0, 1.0);
    ASSERT_NEAR(angle(v1, v2), M_PI / 2, epsilon);
}

TEST_F(VectorTest, FreeProjection) {
    Vector v(3.0, 4.0);
    Vector base(1.0, 0.0);
    ASSERT_DOUBLE_EQ(projection(v, base), 3.0);
}

TEST_F(VectorTest, FreeNormalize) {
    Vector v(3.0, 4.0);
    Vector result = normalize(v);
    ASSERT_NEAR(result.x(), 0.6, epsilon);
    ASSERT_NEAR(result.y(), 0.8, epsilon);
    ASSERT_NEAR(length(result), 1.0, epsilon);
}

TEST_F(VectorTest, FreeTransformTo) {
    Vector v(3.0, 4.0);
    Vector e1(2.0, 0.0);
    Vector e2(0.0, 2.0);
    Vector result = transformTo(v, e1, e2);
    ASSERT_DOUBLE_EQ(result.x(), 1.5);
    ASSERT_DOUBLE_EQ(result.y(), 2.0);
}

TEST_F(VectorTest, FreeTransformFrom) {
    Vector v(1.5, 2.0);
    Vector e1(2.0, 0.0);
    Vector e2(0.0, 2.0);
    Vector result = transformFrom(v, e1, e2);
    ASSERT_DOUBLE_EQ(result.x(), 3.0);
    ASSERT_DOUBLE_EQ(result.y(), 4.0);
}

// Запуск тестов
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
