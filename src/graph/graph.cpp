#include "graph.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_set>
#include <queue>
#include <limits>

void Graph::loadLocations(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Erro ao abrir " << filename << std::endl;
        return;
    }

    std::string line;
    std::getline(file, line); // Ignorar cabeçalho

    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string location, id, code, parking;
        std::getline(ss, location, ',');
        std::getline(ss, id, ',');
        std::getline(ss, code, ',');
        std::getline(ss, parking, ',');

        nodes[code] = {code, {}, std::stoi(parking) != 0};
        idToCode[id] = code;
    }
}

void Graph::loadDistances(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Erro ao abrir " << filename << std::endl;
        return;
    }

    std::string line;
    std::getline(file, line); // Ignorar cabeçalho

    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string loc1, loc2, driving, walking;
        std::getline(ss, loc1, ',');
        std::getline(ss, loc2, ',');
        std::getline(ss, driving, ',');
        std::getline(ss, walking, ',');

        int drivingTime = (driving == "X") ? std::numeric_limits<int>::max() : std::stoi(driving);
        int walkingTime = (walking == "X") ? std::numeric_limits<int>::max() : std::stoi(walking);

        nodes[loc1].adj.push_back({loc2, drivingTime, walkingTime});
        nodes[loc2].adj.push_back({loc1, drivingTime, walkingTime});
    }
}

std::string Graph::getCodeFromId(const std::string& id) {
    if (idToCode.find(id) == idToCode.end()) {
        std::cerr << "ERRO: ID " << id << " não encontrado no mapa de codes." << std::endl;
        return "";
    }
    return idToCode[id];
}


std::vector<std::string> Graph::dijkstra(const std::string& source, const std::string& dest, const std::string& mode) {
    std::unordered_map<std::string, int> dist;
    std::unordered_map<std::string, std::string> parent;
    std::priority_queue<std::pair<int, std::string>, std::vector<std::pair<int, std::string>>, std::greater<>> pq;

    for (const auto& node : nodes) {
        dist[node.first] = std::numeric_limits<int>::max();
    }

    dist[source] = 0;
    pq.push({0, source});

    while (!pq.empty()) {
        auto [currentDist, currentNode] = pq.top();
        pq.pop();

        if (dist[currentNode] < currentDist) continue;

        if (currentNode == dest) break;

        for (const auto& edge : nodes[currentNode].adj) {
            int travelTime = (mode == "driving") ? edge.drivingTime : edge.walkingTime;

            if (travelTime == std::numeric_limits<int>::max()) continue;

            int newDist = currentDist + travelTime;

            if (newDist < dist[edge.dest]) {
                dist[edge.dest] = newDist;
                parent[edge.dest] = currentNode;
                pq.push({newDist, edge.dest});
            }
        }
    }

    if (dist[dest] == std::numeric_limits<int>::max()) return {};

    std::vector<std::string> path;
    for (std::string at = dest; at != source; at = parent[at]) {
        path.push_back(at);
    }
    path.push_back(source);
    std::reverse(path.begin(), path.end());

    return path;
}

// Melhor rota
std::vector<std::string> Graph::findBestRoute(const std::string& source, const std::string& dest, const std::string& mode) {
    return dijkstra(source, dest, mode);
}

// Rota alternativa independente
std::vector<std::string> Graph::findAlternativeRoute(const std::string& source, const std::string& dest, const std::string& mode) {
    auto bestRoute = findBestRoute(source, dest, mode);
    if (bestRoute.empty()) return {};

    std::unordered_set<std::string> blockedNodes(bestRoute.begin() + 1, bestRoute.end() - 1);

    for (auto& node : nodes) {
        if (blockedNodes.count(node.first)) continue;
        auto altRoute = dijkstra(source, dest, mode);
        if (!altRoute.empty() && altRoute != bestRoute) {
            return altRoute;
        }
    }
    return {};
}

int Graph::getRouteTime(const std::vector<std::string>& route, const std::string& mode) {
    int totalTime = 0;
    for (size_t i = 0; i < route.size() - 1; ++i) {
        for (const auto& edge : nodes[route[i]].adj) {
            if (edge.dest == route[i + 1]) {
                totalTime += (mode == "driving") ? edge.drivingTime : edge.walkingTime;
                break;
            }
        }
    }
    return totalTime;
}

void Graph::displayRoute(const std::vector<std::string>& route, int totalTime, const std::string& type, std::ostream& output) {
    if (route.empty()) {
        output << type << ":none\n";
    } else {
        output << type << ":";
        for (size_t i = 0; i < route.size(); ++i) {
            output << route[i];
            if (i < route.size() - 1) output << ",";
        }
        output << "(" << totalTime << ")\n";
    }
}
