#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <limits>
#include <fstream>
#include <sstream>
#include <algorithm>
using namespace std;

// Структура ребер
struct Edge{
    string u, v;
    unsigned int w;
    Edge(const string& u, const string& v, unsigned int w) : u(u), v(v), w(w) {}
};

struct PathResult{
    vector<string> path;
    unsigned int weight;
    PathResult() : weight(numeric_limits<unsigned int> ::max()) {}
};

//Класс графа
class Graph{
private:
    unordered_map<string, vector<Edge>> adjList;
public:
    void addEdge(const string& u, const string& v, unsigned int w){
        adjList[u].emplace_back(u, v, w);
        adjList[v].emplace_back(v, u, w);
    }

    PathResult dijkstra(const string& start, const string& end){
        PathResult result;
        if (adjList.find(start) == adjList.end() || adjList.find(end) == adjList.end()){
            return result;
        }

        unordered_map <string, unsigned int> dist;
        unordered_map <string, string> prev;
        using P = pair<unsigned int, string>;
        priority_queue<P, vector<P>, greater<P>> pq;

        for(const auto& vertex: adjList){
            dist[vertex.first] = numeric_limits<unsigned int> :: max();
        }
        dist[start] = 0;
        pq.emplace(0, start);

        while (!pq.empty())
        {
            unsigned int d = pq.top().first;
            string u = pq.top().second;
            pq.pop();

            if (u == end) {
                vector<string> path;
                string current = end;
                while (current != start) {
                    path.push_back(current);
                    if (prev.find(current) == prev.end()) break;
                    current = prev[current];
                }
                path.push_back(start);
                reverse(path.begin(), path.end());
                result.path = path;
                result.weight = d;
                return result;
            }

            for (const auto& edge : adjList.at(u)) {
                string v = edge.v;
                unsigned int w = edge.w;
                if (dist[u] + w < dist[v]) {
                    dist[v] = dist[u] + w;
                    prev[v] = u;
                    pq.emplace(dist[v], v);
                }
            }
        }
        return result;
    }
    
};

// Чтение графа из файла
vector<Edge> readGraph(const string& filename) {
    vector<Edge> edges;
    ifstream file(filename);
    if (!file.is_open()) return edges;

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string u, v;
        unsigned int w;
        
        if (ss >> u >>v >> w ) {
            edges.emplace_back(u, v, w);
        }
    }
    file.close();
    return edges;
};

// Запись результата в файл
void writePath(const string& filename, const PathResult& result) {
    ofstream file(filename);
    if (!file.is_open()) return;

    if (result.path.empty()) {
        file << "No path found\n";
    } else {
        file << "{";
        for (size_t i = 0; i < result.path.size(); ++i) {
            file << result.path[i];
            if (i < result.path.size() - 1) file << ", ";
        }
        file << "} - " << result.weight << "\n";
    }
    file.close();
};
