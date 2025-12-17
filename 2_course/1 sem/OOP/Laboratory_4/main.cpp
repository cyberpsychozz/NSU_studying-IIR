#include "Graph.hpp"
#include <iostream>
using namespace std;

int main() {
    vector<Edge> edges = readGraph("input.txt");
    if (edges.empty()) {
        cerr << "No valid edges read from input file." << endl;
        return 1;
    }

    Graph graph;
    for (const auto& edge : edges) {
        graph.addEdge(edge.u, edge.v, edge.w);
    }

    PathResult result = graph.dijkstra("Moscow", "Krasnoyarsk");
    writePath("output.txt", result);
    return 0;
}