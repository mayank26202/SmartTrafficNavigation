#ifndef TRAFFIC_H
#define TRAFFIC_H

#include <map>
#include <utility>
#include <cstdlib>
#include <ctime>

class Traffic {
    std::map<std::pair<int, int>, float> congestion;

public:
    Traffic(int n = 0) { std::srand((unsigned)std::time(nullptr)); }

    void updateCongestion(int n) {
        congestion.clear();
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                // Generate random congestion between 0.5 and 2.5
                float value = 0.5f + static_cast<float>(rand()) / RAND_MAX * 2.0f;
                congestion[{i, j}] = congestion[{j, i}] = value;
            }
        }
    }

    float getCongestion(int u, int v) const {
        auto it = congestion.find({u, v});
        if (it != congestion.end()) return it->second;
        return 1.0f;
    }
};

#endif
