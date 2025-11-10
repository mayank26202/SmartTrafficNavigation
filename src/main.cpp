#include <GL/freeglut.h>
#include <vector>
#include <cmath>
#include <iostream>
#include <string>
#include "../include/graph.h"
#include "../include/traffic.h"

Traffic traffic(6);
Graph g(6, &traffic);

std::vector<std::pair<float, float>> positions = {
    {-0.7f, -0.4f},
    {0.0f, -0.4f},
    {0.7f, -0.4f},
    {-0.7f, 0.4f},
    {0.0f, 0.4f},
    {0.7f, 0.4f}
};

std::vector<int> path, path2;
int pathIndex = 0, pathIndex2 = 0;
float carX, carY, car2X, car2Y;
float carSpeed = 0.02f, car2Speed = 0.02f;
int frameCount = 0;

void drawCircle(float cx, float cy, float r, float R, float G, float B) {
    glColor3f(R, G, B);
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i <= 360; ++i) {
        float angle = i * M_PI / 180;
        glVertex2f(cx + cos(angle) * r, cy + sin(angle) * r);
    }
    glEnd();
}

void drawText(float x, float y, const std::string &text, void* font = GLUT_BITMAP_HELVETICA_18) {
    glRasterPos2f(x, y);
    for (char c : text) glutBitmapCharacter(font, c);
}

void drawBanner() {
    glColor3f(0, 0, 0);
    drawText(-0.4f, 0.92f, "Smart Traffic Navigation System");
    drawText(-0.45f, 0.86f, "Dijkstra’s Shortest Path with Dynamic Traffic");
}

void drawLegend() {
    glColor3f(0.95f, 0.95f, 0.95f);
    glBegin(GL_QUADS);
    glVertex2f(-0.98f, 0.8f);
    glVertex2f(-0.55f, 0.8f);
    glVertex2f(-0.55f, 0.55f);
    glVertex2f(-0.98f, 0.55f);
    glEnd();

    glColor3f(0, 0, 0);
    drawText(-0.95f, 0.75f, "Legend");
    struct L { float r, g, b; const char* t; };
    std::vector<L> items = {
        {0, 1, 0, "Low Traffic"},
        {1, 1, 0, "Medium"},
        {1, 0, 0, "Heavy"},
        {0.1f, 0.1f, 0.9f, "Node"},
        {1, 0, 0, "Car 1"},
        {0, 0, 0, "Car 2"}
    };
    float y = 0.70f;
    for (auto &it : items) {
        glColor3f(it.r, it.g, it.b);
        glBegin(GL_QUADS);
        glVertex2f(-0.95f, y);
        glVertex2f(-0.90f, y);
        glVertex2f(-0.90f, y - 0.05f);
        glVertex2f(-0.95f, y - 0.05f);
        glEnd();
        glColor3f(0, 0, 0);
        drawText(-0.88f, y - 0.04f, it.t);
        y -= 0.08f;
    }
}

void drawRoads() {
    glLineWidth(8);
    glBegin(GL_LINES);
    const auto &adj = g.getAdj();
    for (int i = 0; i < adj.size(); ++i) {
        for (auto &e : adj[i]) {
            int j = e.first;
            if (i < j) {
                float cong = traffic.getCongestion(i, j);
                if (cong < 1.2f) glColor3f(0, 1, 0);
                else if (cong < 2.0f) glColor3f(1, 1, 0);
                else glColor3f(1, 0, 0);
                glVertex2f(positions[i].first, positions[i].second);
                glVertex2f(positions[j].first, positions[j].second);
            }
        }
    }
    glEnd();
}

void drawNodes() {
    for (int i = 0; i < positions.size(); ++i) {
        drawCircle(positions[i].first, positions[i].second, 0.05f, 0.1f, 0.1f, 0.9f);
        glColor3f(0, 0, 0);
        drawText(positions[i].first - 0.02f, positions[i].second - 0.09f, "N" + std::to_string(i));
    }
    drawCircle(positions[0].first, positions[0].second, 0.07f, 0, 0.8f, 0);
    drawCircle(positions[5].first, positions[5].second, 0.07f, 0.8f, 0, 0);
    drawText(positions[0].first - 0.05f, positions[0].second + 0.09f, "Source");
    drawText(positions[5].first - 0.07f, positions[5].second + 0.09f, "Destination");
}

void drawInfoPanel() {
    float cost = g.calculatePathCost(path);
    glColor3f(0.9f, 0.9f, 0.9f);
    glBegin(GL_QUADS);
    glVertex2f(0.55f, -0.6f);
    glVertex2f(0.98f, -0.6f);
    glVertex2f(0.98f, -0.9f);
    glVertex2f(0.55f, -0.9f);
    glEnd();

    glColor3f(0, 0, 0);
    drawText(0.57f, -0.65f, "Current Shortest Path:");
    std::string p = "";
    for (int i = 0; i < path.size(); ++i) {
        p += std::to_string(path[i]);
        if (i != path.size() - 1) p += " -> ";
    }
    drawText(0.57f, -0.72f, p);
    drawText(0.57f, -0.82f, "Total Travel Time: " + std::to_string(cost).substr(0, 5));
}

bool detectCollision() {
    float d = std::hypot(carX - car2X, carY - car2Y);
    return d < 0.12f;
}

void update(int) {
    frameCount++;

    // 🔁 Recalculate every few seconds
    if (frameCount % 200 == 0) {
        traffic.updateCongestion(positions.size());
        path = g.dijkstra(0, 5);
        path2 = g.dijkstra(3, 2);
        pathIndex = pathIndex2 = 0;

        // Reset car positions when path updates
        carX = positions[path[0]].first;
        carY = positions[path[0]].second;
        car2X = positions[path2[0]].first;
        car2Y = positions[path2[0]].second;

        std::cout << "Updated traffic and recalculated paths.\n";
    }

    // 🏎️ Movement logic with boundary fix
    auto move = [&](float &x, float &y, const std::vector<int>& p, int &idx, float speed) {
        if (p.empty() || idx >= p.size() - 1) return;

        int from = p[idx], to = p[idx + 1];
        float dx = positions[to].first - positions[from].first;
        float dy = positions[to].second - positions[from].second;

        // Normalize
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist > 0) {
            x += (dx / dist) * speed;
            y += (dy / dist) * speed;
        }

        // Check if reached next node
        if (std::fabs(x - positions[to].first) < 0.02 && std::fabs(y - positions[to].second) < 0.02) {
            idx++;
            // Snap to node position
            x = positions[to].first;
            y = positions[to].second;

            // Stop when reaching destination
            if (idx >= p.size() - 1) {
                idx = p.size() - 1;
            }
        }
    };

    move(carX, carY, path, pathIndex, carSpeed);
    move(car2X, car2Y, path2, pathIndex2, car2Speed);

    // 🚗 Collision avoidance
    if (detectCollision()) {
        carSpeed = 0.008f;
        car2Speed = 0.01f;
    } else {
        carSpeed = car2Speed = 0.02f;
    }

    glutPostRedisplay();
    glutTimerFunc(50, update, 0);
}


void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    drawBanner();
    drawLegend();
    drawRoads();
    drawNodes();
    drawInfoPanel();
    drawCircle(carX, carY, 0.06f, 1, 0, 0);
    drawCircle(car2X, car2Y, 0.06f, 0, 0, 0);
    glutSwapBuffers();
}

void initGraph() {
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(3, 4, 1);
    g.addEdge(4, 5, 1);
    g.addEdge(0, 3, 1.2);
    g.addEdge(1, 4, 1);
    g.addEdge(2, 5, 1.1);
    traffic.updateCongestion(6);
    path = g.dijkstra(0, 5);
    path2 = g.dijkstra(3, 2);
    carX = positions[path[0]].first; carY = positions[path[0]].second;
    car2X = positions[path2[0]].first; car2Y = positions[path2[0]].second;
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1000, 800);
    glutCreateWindow("Smart Traffic Navigation System - Dijkstra’s Algorithm");
    glClearColor(1, 1, 1, 1);
    initGraph();
    glutDisplayFunc(display);
    glutTimerFunc(50, update, 0);
    glutMainLoop();
    return 0;
}
