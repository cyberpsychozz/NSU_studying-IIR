#pragma once
#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;


// Структура точек
struct Edge{
    string u, v;
    double w;
    Edge( u = 0, int v = 0, double w = 0.0) : u(u), v(v), w(w) {}
};

//Класс для объединения двух компонент графа
class UnionFind{
private:
    vector<int> parent, rank;
public:
    UnionFind(int size) : parent(size + 1), rank(size + 1, 0) {
        for (int i = 0; i <= size; ++i) {
            parent[i] = i;
        }
    }
    
    // Функция поиска инцидентных вершин ребра
    int find(int i){
        if(parent[i] != i){
            parent[i] = find(parent[i]);
        }
        return parent[i];
    }

    // Фкнкция объединения двух компонент
    void union_sets(int u, int v){
        u = find(u);
        v = find(v);
        if(u != v){
            if(rank[u] < rank[v]) swap(u, v);
            parent[v] = u;
            if(rank[u] == rank[v]) ++rank[u];
        }
    }

};

//Класс графа
class Graph{
private:
    vector<Node> nodes;
    vector<Edge> edges;
public:
    void addNode(const Node& node){
        nodes.push_back(node);
    }

    // Фнкция создания ребер
    void buildEdges(){
        edges.clear();
        for(size_t i = 0; i < nodes.size(); ++i){
            for(size_t j = i + 1; j < nodes.size(); ++j){
                double dist = nodes[i].distance(nodes[j]);
                edges.emplace_back(nodes[i].id, nodes[j].id, dist);
            }
        }
    }

    //Построение MST с помощью алгоритма Крускала
    vector<Edge> kruskalMST(){
        sort(edges.begin(), edges.end());
        UnionFind uf(nodes.size());
        vector<Edge> mst;
        for(const auto & e : edges){
            if(uf.find(e.u) != uf.find(e.v)){
                uf.union_sets(e.u, e.v);
                mst.push_back(e);
            }
        }
        return mst;
    }
};

//Объявление функций чтения и записи
vector<Node> readPoints(const string& filename);
void writeMST(const string& filename, const vector<Edge>& mst);