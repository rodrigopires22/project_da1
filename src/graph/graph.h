/**
 * @file Graph.h
 * @brief Defines the Graph class and its associated structures and methods for route planning.
 *
 * This header file provides declarations for the Graph class, which represents a graph of nodes and edges.
 * The class supports various route calculations, including restricted routes, sustainable routes, and
 * environmentally friendly routes, with constraints on parking, nodes, and segments.
 */

#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <limits>
#include <unordered_set>
#include <set>

/**
 * @struct Edge
 * @brief Represents an edge in the graph, including travel times and the destination node.
 */
struct Edge {
    std::string dest; /**< The destination node code. */
    int drivingTime; /**< The travel time by driving in minutes. */
    int walkingTime; /**< The travel time by walking in minutes. */

    /**
     * @brief Equality operator for Edge comparison.
     * @param other The other Edge to compare with.
     * @return True if both edges are equal, false otherwise.
     */
    bool operator==(const Edge& other) const;
};

/**
 * @struct Node
 * @brief Represents a node in the graph, including its adjacency list and parking status.
 */
struct Node {
    std::string code; /**< The code identifying the node. */
    std::vector<Edge> adj; /**< A list of edges connecting to other nodes. */
    bool hasParking; /**< Indicates whether the node has parking available. */
};

/**
 * @class Graph
 * @brief A class representing a graph structure for route planning.
 *
 * The Graph class provides methods for loading data, finding routes, and managing node and edge details.
 */
class Graph {
private:
    std::unordered_map<std::string, Node> nodes; /**< A map of node codes to their corresponding Node objects. */
    std::unordered_map<std::string, std::string> idToCode; /**< A map of unique IDs to their corresponding codes. */
    std::unordered_map<std::string, std::string> codeToId; /**< A map of codes to their corresponding unique IDs. */

    /**
     * @brief Implements Dijkstra's algorithm for shortest path calculation.
     * @param source The starting node.
     * @param dest The destination node.
     * @param mode The mode of transportation ("driving" or "walking").
     * @return A vector of strings representing the shortest path between source and destination.
     */
    std::vector<std::string> dijkstra(const std::string& source, const std::string& dest, const std::string& mode);

public:
    /**
     * @brief Loads location data from a file.
     * @param filename The name of the file containing location data.
     */
    void loadLocations(const std::string& filename);

    /**
     * @brief Loads distance data from a file.
     * @param filename The name of the file containing distance data.
     */
    void loadDistances(const std::string& filename);

    /**
     * @brief Displays the graph's nodes and edges.
     */
    void displayGraph();

    /**
     * @brief Retrieves the code associated with a given ID.
     * @param id The unique ID to look up.
     * @return The corresponding code as a string.
     */
    std::string getCodeFromId(const std::string& id);

    /**
     * @brief Retrieves the unique ID associated with a given code.
     * @param code The code to look up.
     * @return The corresponding unique ID as a string.
     */
    std::string getIdFromCode(const std::string& code);

    /**
     * @brief Converts a route from codes to IDs.
     * @param route A vector of location codes.
     * @return A vector of corresponding location IDs.
     */
    std::vector<std::string> convertCodesToIds(const std::vector<std::string>& route);

    /**
     * @brief Finds routes between a source and destination for a given mode of transportation.
     * @param source The starting node.
     * @param dest The destination node.
     * @param mode The mode of transportation ("driving" or "walking").
     * @return A tuple containing:
     * - A vector of strings for the full route.
     * - A vector of strings for the walking portion of the route.
     * - An integer representing the total time.
     * - An integer representing the walking time.
     */
    std::tuple<std::vector<std::string>, std::vector<std::string>, int, int> findRoutes(
        const std::string& source,
        const std::string& dest,
        const std::string& mode
    );

    /**
     * @brief Displays routes and their details.
     * @param route A vector of strings representing the route.
     * @param totalTime The total time for the route.
     * @param type The type of route ("driving" or "walking").
     * @param output The output stream to display the route details.
     */
    void displayRoutes(const std::vector<std::string>& route, int totalTime, const std::string& type, std::ostream& output);

    /**
     * @brief Calculates the total time for a given route and mode of transportation.
     * @param route A vector of strings representing the route.
     * @param mode The mode of transportation ("driving" or "walking").
     * @return The total time as an integer.
     */
    int getRouteTime(const std::vector<std::string>& route, const std::string& mode);

    /**
     * @brief Prints the edges originating from a specified node.
     * @param id The unique ID of the node.
     */
    void printEdgesFrom(const std::string& id);

    /**
     * @brief Finds a route between two locations with specific restrictions.
     * @param source The starting location.
     * @param dest The destination location.
     * @param mode The mode of transportation ("driving" or "walking").
     * @param avoidNodes A set of nodes to avoid.
     * @param avoidSegments A set of edges (pairs of nodes) to avoid.
     * @param includeNode An optional node that must be included in the route.
     * @return A pair containing the route and its total time.
     */
    std::pair<std::vector<std::string>, int> findRestrictedRoute(
        const std::string& source,
        const std::string& dest,
        const std::string& mode,
        const std::unordered_set<std::string>& avoidNodes,
        const std::set<std::pair<std::string, std::string>>& avoidSegments,
        const std::string& includeNode
    );

    /**
     * @brief Finds the most environmentally friendly route between two locations.
     * @param source The starting location.
     * @param dest The destination location.
     * @param maxWalkingTime The maximum allowable walking time (in minutes).
     * @param avoidNodes A set of nodes to avoid.
     * @param avoidSegments A set of edges (pairs of nodes) to avoid.
     * @return A tuple containing:
     * - The best route (vector of strings).
     * - The total time.
     * - The best driving route.
     * - Driving time.
     * - The best walking route.
     * - Walking time.
     * - The best parking node.
     */
    std::tuple<std::vector<std::string>, int,
               std::vector<std::string>, int,
               std::vector<std::string>, int,
               std::string>
    findEnvironmentallyFriendlyRoute(
        const std::string& source,
        const std::string& dest,
        int maxWalkingTime,
        const std::unordered_set<std::string>& avoidNodes,
        const std::set<std::pair<std::string, std::string>>& avoidSegments
    );

    /**
     * @brief Finds a sustainable route between two locations without considering parking.
     * @param source The starting location.
     * @param dest The destination location.
     * @param avoidNodes A set of nodes to avoid.
     * @param avoidSegments A set of edges (pairs of nodes) to avoid.
     * @return A pair containing:
     * - The route as a vector of strings.
     * - The total time.
     */
    std::pair<std::vector<std::string>, int> findSustainableRouteWithoutParking(
        const std::string& source,
        const std::string& dest,
        const std::unordered_set<std::string>& avoidNodes,
        const std::set<std::pair<std::string, std::string>>& avoidSegments
    );

    /**
 * @brief Finds the top environmentally friendly routes between two locations.
 *
 * @param source The starting location.
 * @param dest The destination location.
 * @param maxWalkingTime The maximum allowable walking time (in minutes).
 * @param avoidNodes A set of nodes to avoid.
 * @param avoidSegments A set of edges (pairs of nodes) to avoid.
 * @param maxResults The maximum number of results to return.
 * @return A vector of tuples, each containing:
 * - The best route (vector of strings).
 * - The total time.
 * - The best driving route.
 * - Driving time.
 * - The best walking route.
 * - Walking time.
 * - The best parking node.
 */
    std::vector<std::tuple<
        std::vector<std::string>, int,
        std::vector<std::string>, int,
        std::vector<std::string>, int,
        std::string
    >>
    findTopEnvironmentallyFriendlyRoutes(
        const std::string& source,
        const std::string& dest,
        int maxWalkingTime,
        const std::unordered_set<std::string>& avoidNodes,
        const std::set<std::pair<std::string, std::string>>& avoidSegments,
        int maxResults
    );
};

#endif


