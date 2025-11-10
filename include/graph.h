#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <queue>
#include <limits>
#include "traffic.h"
#include <algorithm>


class Graph {
    int V;
    std::vector<std::vector<std::pair<int, float>>> adj;
    Traffic* traffic;

public:
    Graph(int v, Traffic* t) : V(v), traffic(t) {
        adj.resize(V);
    }

    void addEdge(int u, int v, float w) {
        adj[u].push_back({v, w});
        adj[v].push_back({u, w});
    }

    const std::vector<std::vector<std::pair<int, float>>>& getAdj() const { return adj; }

    std::vector<int> dijkstra(int src, int dest) {
        std::vector<float> dist(V, std::numeric_limits<float>::infinity());
        std::vector<int> parent(V, -1);
        using P = std::pair<float, int>;
        std::priority_queue<P, std::vector<P>, std::greater<P>> pq;

        dist[src] = 0;
        pq.push({0, src});

        while (!pq.empty()) {
            int u = pq.top().second;
            pq.pop();

            for (auto [v, w] : adj[u]) {
                float cost = w * traffic->getCongestion(u, v);
                if (dist[v] > dist[u] + cost) {
                    dist[v] = dist[u] + cost;
                    parent[v] = u;
                    pq.push({dist[v], v});
                }
            }
        }

        std::vector<int> path;
        for (int v = dest; v != -1; v = parent[v])
            path.push_back(v);
        std::reverse(path.begin(), path.end());
        return path;
    }

    float calculatePathCost(const std::vector<int>& path) {
        float total = 0.0f;
        for (size_t i = 0; i + 1 < path.size(); ++i) {
            int u = path[i], v = path[i + 1];
            for (auto [to, w] : adj[u]) {
                if (to == v) {
                    total += w * traffic->getCongestion(u, v);
                    break;
                }
            }
        }
        return total;
    }
};

#endif
