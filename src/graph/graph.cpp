#include "graph.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_set>
#include <queue>
#include <limits>

/**
 * @brief Loads locations from a CSV file and updates the graph.
 *
 * This function reads a CSV file containing locations, IDs, codes, and parking
 * information. The extracted data is then stored in the internal data structures
 * of the Graph class.
 *
 * @param filename The name of the CSV file containing the locations to be loaded.
 *
 * @details The function skips the first line of the file (header) and processes each
 * subsequent line to extract data. It maps codes as keys and associates them with
 * IDs and parking attributes.
 *
 * @exception std::ifstream::failure If the file cannot be opened, an error message
 * is printed, and the function returns without modifying the graph's state.
 *
 * The time complexity is O(n), where n is the number of lines in the
 * CSV file, as each line is processed once.
 */
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

/**
 * @brief Loads distances between locations from a CSV file and updates the graph.
 *
 * This function reads a CSV file containing pairs of locations along with driving
 * and walking times. The extracted data is used to update the adjacency list of
 * the graph with distance information.
 *
 * @param filename The name of the CSV file containing the distances to be loaded.
 *
 * @details The function skips the first line of the file (header) and processes
 * each subsequent line to extract data. Driving and walking times are read, and
 * if the driving time is valid, the adjacency list of the graph is updated for
 * both locations.
 *
 * @exception std::ifstream::failure If the file cannot be opened, an error message
 * is printed, and the function returns without modifying the graph's state.
 *
 * The time complexity is O(n), where n is the number of lines in the
 * CSV file, as each line is processed once.
 */
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

/**
 * @brief Displays the graph's nodes and their connections.
 *
 * This function prints the details of all nodes in the graph, including their
 * parking availability and adjacency list connections. For each edge, the driving
 * and walking times are displayed.
 *
 * @details Each node's code and parking availability are printed, followed by its
 * connections to other nodes with the respective driving and walking times. The
 * graph structure is displayed in a human-readable format.
 *
 * The time complexity is O(n + m), where n is the number of nodes
 * and m is the number of edges in the graph, as every node and its connections
 * are iterated over once.
 */
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


/**
 * @brief Converts location codes to their corresponding IDs.
 *
 * This function takes a vector of location codes and converts them into their
 * corresponding IDs using the `codeToId` mapping.
 *
 * @param route A vector of location codes representing a route.
 * @return A vector of corresponding location IDs for the given codes.
 *
 * @details The function iterates over each location code in the input vector,
 * looks up the corresponding ID in the `codeToId` map, and appends it to the
 * resulting vector.
 *
 * The time complexity is O(n), where n is the size of the input
 * vector, as each code is processed exactly once.
 */
std::vector<std::string> Graph::convertCodesToIds(const std::vector<std::string>& route) {
    std::vector<std::string> convertedRoute;
    for (const auto& code : route) {
        convertedRoute.push_back(codeToId[code]);
    }
    return convertedRoute;
}

/**
 * @brief Retrieves the location code corresponding to a given ID.
 *
 * This function searches for the location code associated with a specific ID
 * using the `idToCode` mapping. If the ID is not found, an error message is
 * displayed, and an empty string is returned.
 *
 * @param id The ID for which the corresponding location code is to be retrieved.
 * @return The location code corresponding to the given ID, or an empty string
 * if the ID is not found.
 *
 * @details The function checks if the ID exists in the `idToCode` map. If not,
 * it logs an error message and returns an empty value. Otherwise, it retrieves
 * and returns the corresponding code.
 *
 * The time complexity is O(1) on average for map lookups.
 */

std::string Graph::getCodeFromId(const std::string& id) {
    if (idToCode.find(id) == idToCode.end()) {
        std::cerr << "ERRO: ID " << id << " não encontrado no mapa de codes." << std::endl;
        return "";
    }
    return idToCode[id];
}

/**
 * @brief Computes the shortest path between two locations using Dijkstra's algorithm.
 *
 * This function implements Dijkstra's algorithm to find the shortest path between
 * a source and destination node in the graph, based on the selected mode of transportation.
 *
 * @param source The starting location code.
 * @param dest The destination location code.
 * @param mode The mode of transportation ("driving" or "walking").
 * @return A vector containing the codes of nodes representing the shortest path from
 * the source to the destination. Returns an empty vector if no path exists.
 *
 * @details The function initializes the distance map and priority queue, processes
 * each node's adjacency list, and updates the distances accordingly. It constructs
 * the path by backtracking from the destination to the source using the parent map.
 *
 * The time complexity is O((V + E) log V), where V is the number of nodes
 * and E is the number of edges. This arises from processing each edge and maintaining
 * the priority queue.
 */
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

/**
 * @brief Finds the best and alternative routes between two locations.
 *
 * This function calculates the shortest path (best route) using Dijkstra's algorithm
 * and attempts to find an alternative route by temporarily removing edges from the
 * best route and re-running Dijkstra's algorithm.
 *
 * @param source The starting location code.
 * @param dest The destination location code.
 * @param mode The mode of transportation ("driving" or "walking").
 * @return A tuple containing:
 * - A vector of IDs for the best route.
 * - A vector of IDs for the alternative route (if found).
 * - The total time for the best route.
 * - The total time for the alternative route.
 *
 * @details The function uses the adjacency list to remove edges temporarily and
 * tests alternative routes. It also converts location codes to IDs and calculates
 * the total time for each route.
 *
 * The time complexity is approximately O(k * (V + E) log V), where k
 * is the number of edges in the best route, V is the number of nodes, and E is the
 * number of edges in the graph. This accounts for multiple calls to Dijkstra's algorithm.
 */
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

/**
 * @brief Displays a route and its total travel time in a formatted manner.
 *
 * This function prints the details of a given route to the specified output stream.
 * It includes the type of route (e.g., best or alternative), the sequence of
 * locations in the route, and the total travel time if available.
 *
 * @param route A vector of location IDs representing the route.
 * @param totalTime The total travel time for the route, or std::numeric_limits<int>::max()
 * if no time is applicable.
 * @param type The type of route (e.g., "best" or "alternative").
 * @param output The output stream where the formatted route will be printed.
 *
 * @details The function handles cases where the route is empty by printing "none".
 * For valid routes, it formats the location IDs with commas between them, followed
 * by the total travel time enclosed in parentheses. If the total time is set to
 * std::numeric_limits<int>::max(), "none" is printed for the time.
 *
 * The time complexity is O(n), where n is the number of locations in the
 * route, as each location ID is processed once.
 */
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

/**
 * @brief Calculates the total travel time for a given route.
 *
 * This function computes the total time required to traverse a route based on
 * the specified mode of transportation ("driving" or "walking").
 *
 * @param route A vector of location codes representing the route.
 * @param mode The mode of transportation ("driving" or "walking").
 * @return The total travel time for the route.
 *
 * @details The function iterates through the edges connecting consecutive nodes
 * in the route, accumulating the travel time based on the selected mode. If no
 * valid edge is found between two nodes, the total time remains unaffected.
 *
 * The time complexity is O(n * m), where n is the size of the route
 * and m is the average number of adjacent edges per node, as each node's adjacency
 * list is searched for the next destination.
 */

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

/**
 * @brief Prints all edges originating from a specific node.
 *
 * This function displays the connections (edges) from a node identified by its
 * ID. It includes the destination nodes and the respective driving and walking times.
 *
 * @param id The ID of the node whose edges are to be printed.
 *
 * @details The function retrieves the corresponding code for the given ID using
 * the `getCodeFromId` method. If the node is not found, an error message is printed.
 * Otherwise, the edges originating from the node are iterated and displayed in
 * a formatted manner, showing destination, driving time, and walking time.
 *
 * The time complexity is O(m), where m is the number of edges originating
 * from the node, as each edge in the adjacency list is processed once.
 */

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

/**
 * @brief Finds a route between two locations with restrictions.
 *
 * This function calculates the shortest path between a source and destination
 * while considering various restrictions such as nodes to avoid, segments to
 * avoid, and an optional mandatory node to include in the route.
 *
 * @param source The starting location code.
 * @param dest The destination location code.
 * @param mode The mode of transportation ("driving" or "walking").
 * @param avoidNodes A set of nodes that should be excluded from the route.
 * @param avoidSegments A set of segments (pairs of nodes) that should be excluded
 * from the route.
 * @param includeNode An optional node that must be included in the route.
 * @return A pair containing:
 * - A vector of IDs representing the restricted route.
 * - The total travel time for the route.
 *
 * @details The function uses a helper lambda to execute a modified version of
 * Dijkstra's algorithm with the given restrictions. If a mandatory node is
 * specified, the route is split into two parts: one from the source to the
 * mandatory node and another from the mandatory node to the destination.
 * The two paths are concatenated to form the complete route.
 *
 * The time complexity is O(k * (V + E) log V), where k is the number
 * of Dijkstra calls required (typically 1 or 2), V is the number of nodes, and E
 * is the number of edges in the graph.
 */

std::pair<std::vector<std::string>, int> Graph::findRestrictedRoute(
    const std::string& source,
    const std::string& dest,
    const std::string& mode,
    const std::unordered_set<std::string>& avoidNodes,
    const std::set<std::pair<std::string, std::string>>& avoidSegments,
    const std::string& includeNode) {

    auto runDijkstra = [&](const std::string& src, const std::string& dst) -> std::pair<std::vector<std::string>, int> {
        std::unordered_map<std::string, int> dist;
        std::unordered_map<std::string, std::string> parent;
        std::priority_queue<std::pair<int, std::string>, std::vector<std::pair<int, std::string>>, std::greater<>> pq;

        for (const auto& node : nodes)
            dist[node.first] = std::numeric_limits<int>::max();

        if (avoidNodes.count(src) || avoidNodes.count(dst)) return {{}, std::numeric_limits<int>::max()};

        dist[src] = 0;
        pq.push({0, src});

        while (!pq.empty()) {
            auto [currentDist, currentNode] = pq.top();
            pq.pop();

            if (avoidNodes.count(currentNode)) continue;
            if (currentDist > dist[currentNode]) continue;

            for (const auto& edge : nodes[currentNode].adj) {
                if (avoidNodes.count(edge.dest)) continue;
                if (avoidSegments.count({currentNode, edge.dest}) || avoidSegments.count({edge.dest, currentNode})) continue;

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

        if (dist[dst] == std::numeric_limits<int>::max()) return {{}, std::numeric_limits<int>::max()};

        std::vector<std::string> path;
        for (std::string at = dst; at != src; at = parent[at]) {
            path.push_back(at);
        }
        path.push_back(src);
        std::reverse(path.begin(), path.end());

        return {path, dist[dst]};
    };

    // Se não houver nó obrigatório
    if (includeNode.empty()) {
        auto [path, time] = runDijkstra(source, dest);
        return {convertCodesToIds(path), time};
    }

    // Dividir em duas partes: source → includeNode e includeNode → dest
    auto [firstHalf, time1] = runDijkstra(source, includeNode);
    auto [secondHalf, time2] = runDijkstra(includeNode, dest);

    if (firstHalf.empty() || secondHalf.empty()) return {{}, std::numeric_limits<int>::max()};

    // Concatenar sem repetir o includeNode
    firstHalf.pop_back(); // remove includeNode do final
    firstHalf.insert(firstHalf.end(), secondHalf.begin(), secondHalf.end());

    return {convertCodesToIds(firstHalf), time1 + time2};
}
