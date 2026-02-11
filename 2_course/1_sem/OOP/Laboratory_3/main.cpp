#include "Graph.hpp"
#include <sstream>
using namespace std;


vector<Node> readPoints(const string& filename) {
    vector<Node> points;
    ifstream file(filename);
    if (!file.is_open()) return points;

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        int id;
        double x, y;
        char comma1, comma2;
        if (ss >> id >> comma1 >> x >> comma2 >> y && comma1 == ',' && comma2 == ',') {
            points.emplace_back(id, x, y);
        }
    }
    file.close();
    return points;
}


void writeMST(const string& filename, const vector<Edge>& mst) {
    ofstream file(filename);
    if (!file.is_open()) return;

    for (const auto& e : mst) {
        file << e.u << " - " << e.v << "\n";
    }
    file.close();
}


int main() {
    vector<Node> points = readPoints("input.txt");
    if (points.empty()) {
        cerr << "No points read from input file." << endl;
        return 1;
    }
    Graph graph;
    for (const auto& point : points) {
        graph.addNode(point);
    }
    graph.buildEdges();
    vector<Edge> mst = graph.kruskalMST();
    writeMST("output.txt", mst);
    return 0;
}
