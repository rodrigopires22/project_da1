#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <limits>

struct Edge {
    std::string dest;
    int drivingTime;
    int walkingTime;

    bool operator==(const Edge& other) const {
        return dest == other.dest &&
               drivingTime == other.drivingTime &&
               walkingTime == other.walkingTime;
    }
};

struct Node {
    std::string code;
    std::vector<Edge> adj;
    bool hasParking;
};

class Graph {
private:
    std::unordered_map<std::string, Node> nodes;
    std::unordered_map<std::string, std::string> idToCode;
    std::unordered_map<std::string, std::string> codeToId;

    std::vector<std::string> dijkstra(const std::string& source, const std::string& dest, const std::string& mode);
    
public:
    void loadLocations(const std::string& filename);
    void loadDistances(const std::string& filename);
    void displayGraph();
    std::string getCodeFromId(const std::string& id);
    std::vector<std::string> convertCodesToIds(const std::vector<std::string>& route);
    std::tuple<std::vector<std::string>, std::vector<std::string>, int, int> findRoutes(const std::string& source, const std::string& dest, const std::string& mode);
    void displayRoutes(const std::vector<std::string>& route, int totalTime, const std::string& type, std::ostream& output);
    int getRouteTime(const std::vector<std::string>& route, const std::string& mode);
    void printEdgesFrom(const std::string& id);
};

#endif // GRAPH_H
