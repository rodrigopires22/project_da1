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

    std::vector<std::string> dijkstra(const std::string& source, const std::string& dest, const std::string& mode);
    
public:
    void loadLocations(const std::string& filename);
    void loadDistances(const std::string& filename);
    std::string getCodeFromId(const std::string& id);

    std::vector<std::string> findBestRoute(const std::string& source, const std::string& dest, const std::string& mode);
    std::vector<std::string> findAlternativeRoute(const std::string& source, const std::string& dest, const std::string& mode);

    int getRouteTime(const std::vector<std::string>& route, const std::string& mode);
    void displayRoute(const std::vector<std::string>& route, int totalTime, const std::string& type, std::ostream& output);
};

#endif // GRAPH_H
