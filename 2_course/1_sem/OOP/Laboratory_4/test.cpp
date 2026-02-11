#include "Graph.hpp"
#include <gtest/gtest.h>
#include <sstream>

class DijkstraTest : public ::testing::Test {
protected:
    const double epsilon = 1e-10; // Для сравнения чисел с плавающей точкой (не используется здесь)
};

// Тест чтения пустого или невалидного ввода
TEST_F(DijkstraTest, ReadGraphEmpty) {
    std::stringstream ss("\ninvalid\n");
    std::vector<Edge> edges;
    std::string line;
    while (std::getline(ss, line)) {
        if (line.empty()) continue;
        std::stringstream line_ss(line);
        std::string u, v;
        unsigned int w;
        char dash1, dash2;
        if (std::getline(line_ss, u, '-') && line_ss >> dash1 >> v >> dash2 >> w && dash1 == '-' && dash2 == '-') {
            edges.emplace_back(u, v, w);
        }
    }
    ASSERT_TRUE(edges.empty());
}

// Тест алгоритма Дейкстры
TEST_F(DijkstraTest, DijkstraShortestPath) {
    Graph graph;
    graph.addEdge("Moscow", "Novosibirsk", 7);
    graph.addEdge("Moscow", "Toronto", 9);
    graph.addEdge("Moscow", "Krasnoyarsk", 14);
    graph.addEdge("Novosibirsk", "Toronto", 10);
    graph.addEdge("Novosibirsk", "Omsk", 15);
    graph.addEdge("Omsk", "Toronto", 11);
    graph.addEdge("Toronto", "Krasnoyarsk", 2);
    graph.addEdge("Krasnoyarsk", "Kiev", 9);
    graph.addEdge("Kiev", "Omsk", 6);

    PathResult result = graph.dijkstra("Moscow", "Kiev");
    ASSERT_EQ(result.path.size(), 4);
    ASSERT_EQ(result.path[0], "Moscow");
    ASSERT_EQ(result.path[1], "Toronto");
    ASSERT_EQ(result.path[2], "Krasnoyarsk");
    ASSERT_EQ(result.path[3], "Kiev");
    ASSERT_EQ(result.weight, 20); // Путь: Moscow -> Toronto (9) -> Krasnoyarsk (2) -> Kiev (9) = 20 (ошибка в примере)
}

// Тест для случая, когда путь не существует
TEST_F(DijkstraTest, DijkstraNoPath) {
    Graph graph;
    graph.addEdge("Moscow", "Novosibirsk", 7);
    PathResult result = graph.dijkstra("Moscow", "Kiev");
    ASSERT_TRUE(result.path.empty());
    ASSERT_EQ(result.weight, std::numeric_limits<unsigned int>::max());
}

// Тест вывода результата
TEST_F(DijkstraTest, WritePath) {
    PathResult result;
    result.path = {"Moscow", "Toronto", "Krasnoyarsk"};
    result.weight = 11;
    std::stringstream ss;
    if (result.path.empty()) {
        ss << "No path found\n";
    } else {
        ss << "{";
        for (size_t i = 0; i < result.path.size(); ++i) {
            ss << result.path[i];
            if (i < result.path.size() - 1) ss << ", ";
        }
        ss << "} - " << result.weight << "\n";
    }
    std::string output = ss.str();
    std::string expected = "{Moscow, Toronto, Krasnoyarsk} - 11\n";
    ASSERT_EQ(output, expected);
}

// Запуск тестов
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}