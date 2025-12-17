#include "Graph.hpp"
#include <gtest/gtest.h>
#include <sstream>

// Основная программа

// Тесты
class MSTTest : public ::testing::Test {
protected:
    const double epsilon = 1e-10; // Погрешность для сравнения double
};

// Тест чтения точек из строки
TEST_F(MSTTest, ReadPointsValid) {
    std::stringstream ss("1, 7, 5\n2, 3, -11\ninvalid line\n3, 2, -2\n\n4, -1, -8\n5, 14, -14");
    std::vector<Node> points;
    std::string line;
    while (std::getline(ss, line)) {
        if (line.empty()) continue;
        std::stringstream line_ss(line);
        int id;
        double x, y;
        char comma1, comma2;
        if (line_ss >> id >> comma1 >> x >> comma2 >> y && comma1 == ',' && comma2 == ',') {
            points.emplace_back(id, x, y);
        }
    }
    ASSERT_EQ(points.size(), 5);
    ASSERT_EQ(points[0].id, 1);
    ASSERT_DOUBLE_EQ(points[0].x, 7.0);
    ASSERT_DOUBLE_EQ(points[0].y, 5.0);
    ASSERT_EQ(points[4].id, 5);
    ASSERT_DOUBLE_EQ(points[4].x, 14.0);
    ASSERT_DOUBLE_EQ(points[4].y, -14.0);
}

// Тест чтения пустого или невалидного ввода
TEST_F(MSTTest, ReadPointsEmpty) {
    std::stringstream ss("\ninvalid\n");
    std::vector<Node> points;
    std::string line;
    while (std::getline(ss, line)) {
        if (line.empty()) continue;
        std::stringstream line_ss(line);
        int id;
        double x, y;
        char comma1, comma2;
        if (line_ss >> id >> comma1 >> x >> comma2 >> y && comma1 == ',' && comma2 == ',') {
            points.emplace_back(id, x, y);
        }
    }
    ASSERT_TRUE(points.empty());
}

// Тест вычисления расстояния между точками
TEST_F(MSTTest, DistanceCalculation) {
    Node p1(1, 0.0, 0.0);
    Node p2(2, 3.0, 4.0);
    double dist = p1.distance(p2);
    ASSERT_DOUBLE_EQ(dist, 5.0);
}

// Тест алгоритма Крускала
TEST_F(MSTTest, BuildMST) {
    Graph graph;
    // Добавляем 10 узлов, расположенных на прямой x = 0..9, y = 0
    graph.addNode(Node(1, 0.0, 0.0));
    graph.addNode(Node(2, 1.0, 0.0));
    graph.addNode(Node(3, 2.0, 0.0));
    graph.addNode(Node(4, 3.0, 0.0));
    graph.addNode(Node(5, 4.0, 0.0));
    graph.addNode(Node(6, 5.0, 0.0));
    graph.addNode(Node(7, 6.0, 0.0));
    graph.addNode(Node(8, 7.0, 0.0));
    graph.addNode(Node(9, 8.0, 0.0));
    graph.addNode(Node(10, 9.0, 0.0));
    
    graph.buildEdges();
    std::vector<Edge> mst = graph.kruskalMST();
    
    // Проверяем, что MST содержит 9 рёбер для 10 вершин
    ASSERT_EQ(mst.size(), 9);
    
    // Проверяем наличие ожидаемых рёбер
    bool has1_2 = false, has2_3 = false, has3_4 = false, has4_5 = false;
    bool has5_6 = false, has6_7 = false, has7_8 = false, has8_9 = false, has9_10 = false;
    for (const auto& e : mst) {
        if ((e.u == 1 && e.v == 2) || (e.u == 2 && e.v == 1)) has1_2 = true;
        if ((e.u == 2 && e.v == 3) || (e.u == 3 && e.v == 2)) has2_3 = true;
        if ((e.u == 3 && e.v == 4) || (e.u == 4 && e.v == 3)) has3_4 = true;
        if ((e.u == 4 && e.v == 5) || (e.u == 5 && e.v == 4)) has4_5 = true;
        if ((e.u == 5 && e.v == 6) || (e.u == 6 && e.v == 5)) has5_6 = true;
        if ((e.u == 6 && e.v == 7) || (e.u == 7 && e.v == 6)) has6_7 = true;
        if ((e.u == 7 && e.v == 8) || (e.u == 8 && e.v == 7)) has7_8 = true;
        if ((e.u == 8 && e.v == 9) || (e.u == 9 && e.v == 8)) has8_9 = true;
        if ((e.u == 9 && e.v == 10) || (e.u == 10 && e.v == 9)) has9_10 = true;
    }
    
    // Проверяем, что все ожидаемые рёбра присутствуют
    ASSERT_TRUE(has1_2);
    ASSERT_TRUE(has2_3);
    ASSERT_TRUE(has3_4);
    ASSERT_TRUE(has4_5);
    ASSERT_TRUE(has5_6);
    ASSERT_TRUE(has6_7);
    ASSERT_TRUE(has7_8);
    ASSERT_TRUE(has8_9);
    ASSERT_TRUE(has9_10);
}

// Тест записи MST
TEST_F(MSTTest, WriteMST) {
    std::vector<Edge> mst = {{1, 3, 0.0}, {2, 4, 0.0}, {2, 5, 0.0}, {3, 4, 0.0}};
    std::stringstream ss;
    for (const auto& e : mst) {
        ss << e.u << " - " << e.v << "\n";
    }
    std::string output = ss.str();
    std::string expected = "1 - 3\n2 - 4\n2 - 5\n3 - 4\n";
    ASSERT_EQ(output, expected);
}

// Тест Union-Find
TEST_F(MSTTest, UnionFind) {
    UnionFind uf(5);
    ASSERT_EQ(uf.find(1), 1);
    uf.union_sets(1, 2);
    ASSERT_EQ(uf.find(1), uf.find(2));
    uf.union_sets(2, 3);
    ASSERT_EQ(uf.find(1), uf.find(3));
    ASSERT_NE(uf.find(1), uf.find(4));
}

// Запуск тестов
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}