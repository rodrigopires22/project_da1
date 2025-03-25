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
        codeToId[code] = id;
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
        if (drivingTime != std::numeric_limits<int>::max()) {
            nodes[loc1].adj.push_back({loc2, drivingTime, walkingTime});
            nodes[loc2].adj.push_back({loc1, drivingTime, walkingTime});
        }        
    }
}
void Graph::displayGraph() {
    std::cout << "=== Grafo Carregado ===" << std::endl;
    for (const auto& [code, node] : nodes) {
        std::cout << "Nó: " << code << " (Parking: " << (node.hasParking ? "Sim" : "Não") << ")\n";
        for (const auto& edge : node.adj) {
            std::cout << " -> " << edge.dest << " (Driving: " << edge.drivingTime 
                      << ", Walking: " << edge.walkingTime << ")\n";
        }
        std::cout << "----------------------------\n";
    }
}

std::vector<std::string> Graph::convertCodesToIds(const std::vector<std::string>& route) {
    std::vector<std::string> convertedRoute;
    for (const auto& code : route) {
        convertedRoute.push_back(codeToId[code]);
    }
    return convertedRoute;
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

        if (currentDist > dist[currentNode]) continue;

        for (const auto& edge : nodes[currentNode].adj) {
            int weight = (mode == "driving") ? edge.drivingTime : edge.walkingTime;
            if (weight == std::numeric_limits<int>::max()) continue;

            int newDist = currentDist + weight;
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

std::tuple<std::vector<std::string>, std::vector<std::string>, int, int>
Graph::findRoutes(const std::string& source, const std::string& dest, const std::string& mode) {
    auto bestRoute = dijkstra(source, dest, mode);
    std::vector<std::string> altRoute;

    if (!bestRoute.empty()) {
        for (size_t i = 0; i < bestRoute.size() - 1; ++i) {
            std::string from = bestRoute[i];
            std::string to = bestRoute[i + 1];

            auto originalAdj = nodes[from].adj;

            nodes[from].adj.erase(
                std::remove_if(
                    nodes[from].adj.begin(),
                    nodes[from].adj.end(),
                    [&](const Edge& edge) { return edge.dest == to; }
                ),
                nodes[from].adj.end()
            );

            auto tempRoute = dijkstra(source, dest, mode);
            nodes[from].adj = originalAdj;

            if (!tempRoute.empty() && tempRoute != bestRoute) {
                altRoute = tempRoute;
                break;
            }
        }
    }

    return {convertCodesToIds(bestRoute), convertCodesToIds(altRoute),
            getRouteTime(bestRoute, mode), getRouteTime(altRoute, mode)};
}

void Graph::displayRoutes(const std::vector<std::string>& route, int totalTime, const std::string& type, std::ostream& output) {
    if (route.empty()) {
        output << type << ":none\n";
    } else {
        output << type << ":";
        for (size_t i = 0; i < route.size(); ++i) {
            output << route[i];
            if (i < route.size() - 1) output << ",";
        }
        if (totalTime == std::numeric_limits<int>::max()) {
            output << "none\n";
        } else {
            output << "(" << totalTime << ")\n";
        }        
    }
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

void Graph::printEdgesFrom(const std::string& id) {
    std::string code = getCodeFromId(id);
    if (nodes.find(code) == nodes.end()) {
        std::cout << "Nó com ID " << id << " não encontrado.\n";
        return;
    }

    std::cout << "Ligações a partir do nó " << id << " (" << code << "):\n";
    for (const auto& edge : nodes[code].adj) {
        std::cout << " -> " << edge.dest
                  << " (Driving: " << edge.drivingTime
                  << ", Walking: " << edge.walkingTime << ")\n";
    }
}
