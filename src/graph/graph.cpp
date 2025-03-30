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
 * @brief Retrieves the unique ID associated with a given code.
 *
 * This function looks up the corresponding ID for a provided location code in the `codeToId` map.
 * If the code does not exist in the map, it returns an empty string.
 *
 * @param code The location code whose associated ID is to be retrieved.
 * @return A string representing the unique ID associated with the provided code.
 * Returns an empty string if the code is not found in the `codeToId` map.
 *
 * @details The function uses the `codeToId` map to perform the lookup and ensures that
 * an invalid code will result in a safe, empty string output.
 *
 * Timecomplexity O(1), as the lookup operation in the unordered map is performed in constant time on average.
 */
std::string Graph::getIdFromCode(const std::string& code) {
  if (codeToId.find(code) == codeToId.end()) {
    return "";
  }
  return codeToId[code];
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


// Returns: (full_route, total_time, driving_route, driving_time, walking_route, walking_time, parking_node)

/**
 * @brief Finds the most environmentally friendly route between a source and a destination, considering both driving and walking segments.
 *
 * This function calculates the optimal route by combining driving and walking routes, minimizing the total travel time while respecting the maximum allowed walking time. It avoids specified nodes and segments and identifies the best parking node to balance driving and walking.
 *
 * @param source The starting node for the journey.
 * @param dest The destination node for the journey.
 * @param maxWalkingTime The maximum allowed walking time (in minutes).
 * @param avoidNodes A set of nodes that must be excluded from the journey.
 * @param avoidSegments A set of segments (pairs of nodes) that must be excluded from the journey.
 * @return A tuple containing:
 * - A vector of strings representing the best route (combined driving and walking).
 * - An integer indicating the total travel time (in minutes) for the best route.
 * - A vector of strings representing the best driving route.
 * - An integer indicating the total driving time (in minutes).
 * - A vector of strings representing the best walking route.
 * - An integer indicating the total walking time (in minutes).
 * - A string representing the best parking node selected.
 *
 * @details The function assumes that the `findRestrictedRoute` method is used to calculate both driving and walking routes, and it excludes invalid routes or those exceeding the maximum allowed walking time. It iterates over available parking nodes, evaluates potential routes, and selects the most suitable one based on total travel time and walking preferences.
 *
 * The time complexity is O(N * (D + W)), where:
 * - N is the number of parking nodes.
 * - D is the time complexity of finding a driving route using `findRestrictedRoute`.
 * - W is the time complexity of finding a walking route using `findRestrictedRoute`.
 * The function evaluates all parking nodes and computes both driving and walking routes for each node.
 */
std::tuple<std::vector<std::string>, int,
           std::vector<std::string>, int,
           std::vector<std::string>, int,
           std::string>
Graph::findEnvironmentallyFriendlyRoute(
    const std::string& source,
    const std::string& dest,
    int maxWalkingTime,
    const std::unordered_set<std::string>& avoidNodes,
    const std::set<std::pair<std::string, std::string>>& avoidSegments)
{
    std::vector<std::string> bestRoute;
    std::vector<std::string> bestDrivingRoute;
    std::vector<std::string> bestWalkingRoute;
    std::string bestParkingNode;
    int minTotalTime = std::numeric_limits<int>::max();
    int bestWalkingTime = 0;
    int bestDrivingTime = 0;

    // Invalid if source or destination has parking
    if (nodes.at(source).hasParking || nodes.at(dest).hasParking) {
        return {{}, minTotalTime, {}, 0, {}, 0, ""};
    }

    // Find all the parking nodes
    std::vector<std::string> parkingNodes;
    for (const auto& [code, node] : nodes) {
        if (node.hasParking && code != source && code != dest && !avoidNodes.count(code)) {
            parkingNodes.push_back(code);
        }
    }

    // If no parking nodes exist
    if (parkingNodes.empty()) {
        return {{}, minTotalTime, {}, 0, {}, 0, ""};
    }

    for (const auto& parkingNode : parkingNodes) {
        auto drivingRoute = findRestrictedRoute(source, parkingNode, "driving", avoidNodes, avoidSegments, "");
        auto walkingRoute = findRestrictedRoute(parkingNode, dest, "walking", avoidNodes, avoidSegments, "");

        // Skip invalid routes
        if (drivingRoute.first.empty() || walkingRoute.first.empty()) continue;

        int walkingTime = walkingRoute.second;
        if (walkingTime > maxWalkingTime) continue;

        int totalTime = drivingRoute.second + walkingTime;

        // Check if this route is better than the best one
        if (totalTime < minTotalTime ||
            (totalTime == minTotalTime && walkingTime > bestWalkingTime)) {
            minTotalTime = totalTime;
            bestWalkingTime = walkingTime;
            bestDrivingTime = drivingRoute.second;
            bestParkingNode = parkingNode;
            bestDrivingRoute = drivingRoute.first;
            bestWalkingRoute = walkingRoute.first;

            // Combine the routes: driving + walking
            bestRoute = drivingRoute.first;
            bestRoute.pop_back();  // Remove the last node of the driving route (because it's the parking node)
            bestRoute.insert(bestRoute.end(), walkingRoute.first.begin(), walkingRoute.first.end());
        }
    }

    return {bestRoute, minTotalTime,
            bestDrivingRoute, bestDrivingTime,
            bestWalkingRoute, bestWalkingTime,
            bestParkingNode};
}

/**
 * @brief Finds a sustainable route between two locations without considering parking nodes.
 *
 * This function calculates the shortest path between a source and a destination while avoiding specified nodes and segments.
 * It ensures that neither the source nor the destination involves parking nodes, and it uses a modified Dijkstra's algorithm
 * for route computation.
 *
 * @param source The starting location code.
 * @param dest The destination location code.
 * @param avoidNodes A set of nodes to be avoided during the journey.
 * @param avoidSegments A set of segments (pairs of nodes) to be avoided during the journey.
 * @return A pair containing:
 * - A vector of strings representing the calculated route (node IDs).
 * - An integer indicating the total travel time for the route.
 *
 * @details The function utilizes a lambda (`runDijkstra`) to execute a Dijkstra's algorithm variant. It iterates through all nodes
 * in the graph, updating distances and paths while applying the given constraints. If the source or destination is invalid (e.g.,
 * involves parking or belongs to the avoided nodes), an empty path and infinite time are returned. The resulting path is reversed
 * and converted to node IDs before being returned.
 *
 * The time complexity is O((V + E) log V), where:
 * - V is the number of nodes in the graph.
 * - E is the number of edges in the graph.
 * This complexity arises from the use of Dijkstra's algorithm with a priority queue for graph traversal and pathfinding.
 */
std::pair<std::vector<std::string>, int> Graph::findSustainableRouteWithoutParking(
    const std::string& source,
    const std::string& dest,
    const std::unordered_set<std::string>& avoidNodes,
    const std::set<std::pair<std::string, std::string>>& avoidSegments) {

    auto runDijkstra = [&](const std::string& src, const std::string& dst) -> std::pair<std::vector<std::string>, int> {
        std::unordered_map<std::string, int> dist;
        std::unordered_map<std::string, std::string> parent;
        std::priority_queue<std::pair<int, std::string>, std::vector<std::pair<int, std::string>>, std::greater<>> pq;

        for (const auto& [code, node] : nodes)
            dist[code] = std::numeric_limits<int>::max();

        if (avoidNodes.count(src) || avoidNodes.count(dst) || nodes.at(src).hasParking || nodes.at(dst).hasParking)
            return {{}, std::numeric_limits<int>::max()};

        dist[src] = 0;
        pq.push({0, src});

        while (!pq.empty()) {
            auto [d, u] = pq.top();
            pq.pop();

            if (avoidNodes.count(u) || nodes[u].hasParking) continue;
            if (d > dist[u]) continue;

            for (const auto& edge : nodes[u].adj) {
                if (avoidNodes.count(edge.dest) || nodes[edge.dest].hasParking) continue;
                if (avoidSegments.count({u, edge.dest}) || avoidSegments.count({edge.dest, u})) continue;

                int weight = edge.walkingTime;
                if (weight == std::numeric_limits<int>::max()) continue;

                int newDist = d + weight;
                if (newDist < dist[edge.dest]) {
                    dist[edge.dest] = newDist;
                    parent[edge.dest] = u;
                    pq.push({newDist, edge.dest});
                }
            }
        }

        if (dist[dst] == std::numeric_limits<int>::max()) return {{}, std::numeric_limits<int>::max()};

        std::vector<std::string> path;
        for (std::string at = dst; at != src; at = parent[at])
            path.push_back(at);
        path.push_back(src);
        std::reverse(path.begin(), path.end());

        return {convertCodesToIds(path), dist[dst]};
    };

    return runDijkstra(source, dest);
}


/**
 * @brief Finds a sustainable route between two locations without considering parking nodes.
 *
 * This function calculates the shortest path between a source and a destination while avoiding specified nodes and segments.
 * It ensures that neither the source nor the destination involves parking nodes, and it uses a modified Dijkstra's algorithm
 * for route computation.
 *
 * @param source The starting location code.
 * @param dest The destination location code.
 * @param avoidNodes A set of nodes to be avoided during the journey.
 * @param avoidSegments A set of segments (pairs of nodes) to be avoided during the journey.
 * @return A pair containing:
 * - A vector of strings representing the calculated route (node IDs).
 * - An integer indicating the total travel time for the route.
 *
 * @details The function utilizes a lambda (`runDijkstra`) to execute a Dijkstra's algorithm variant. It iterates through all nodes
 * in the graph, updating distances and paths while applying the given constraints. If the source or destination is invalid (e.g.,
 * involves parking or belongs to the avoided nodes), an empty path and infinite time are returned. The resulting path is reversed
 * and converted to node IDs before being returned.
 *
 * The time complexity is O((V + E) log V), where:
 * - V is the number of nodes in the graph.
 * - E is the number of edges in the graph.
 * This complexity arises from the use of Dijkstra's algorithm with a priority queue.
 */
std::vector<std::tuple<
    std::vector<std::string>, int,   
    std::vector<std::string>, int,   
    std::vector<std::string>, int,   
    std::string                      
>>
Graph::findTopEnvironmentallyFriendlyRoutes(
    const std::string& source,
    const std::string& dest,
    int maxWalkingTime,
    const std::unordered_set<std::string>& avoidNodes,
    const std::set<std::pair<std::string, std::string>>& avoidSegments,
    int maxResults)
{
    using ResultTuple = std::tuple<
        std::vector<std::string>, int,
        std::vector<std::string>, int,
        std::vector<std::string>, int,
        std::string>;

    std::vector<ResultTuple> candidates;

    for (int walkLimit = maxWalkingTime + 1; walkLimit <= 15; ++walkLimit) {
        for (const auto& [code, node] : nodes) {
            if (!node.hasParking || code == source || code == dest || avoidNodes.count(code)) continue;

            auto driving = findRestrictedRoute(source, code, "driving", avoidNodes, avoidSegments, "");
            auto walking = findRestrictedRoute(code, dest, "walking", avoidNodes, avoidSegments, "");

            if (driving.first.empty() || walking.first.empty()) continue;

            int walkingTime = walking.second;
            if (walkingTime > walkLimit) continue;

            int totalTime = driving.second + walkingTime;

            std::vector<std::string> fullRoute = driving.first;
            fullRoute.pop_back(); 
            fullRoute.insert(fullRoute.end(), walking.first.begin(), walking.first.end());

            candidates.push_back({
                fullRoute, totalTime,
                driving.first, driving.second,
                walking.first, walking.second,
                code
            });
        }
    }

    std::sort(candidates.begin(), candidates.end(), [](const ResultTuple& a, const ResultTuple& b) {
        return std::get<1>(a) < std::get<1>(b);  
    });

    std::vector<ResultTuple> uniqueResults;
    std::unordered_set<std::string> seenParking;

    for (const auto& candidate : candidates) {
        const std::string& parking = std::get<6>(candidate);
        if (seenParking.count(parking)) continue;
        seenParking.insert(parking);
        uniqueResults.push_back(candidate);
        if ((int)uniqueResults.size() == maxResults) break;
    }

    return uniqueResults;
}
