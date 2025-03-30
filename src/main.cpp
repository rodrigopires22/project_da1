#include "graph/graph.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <set>
using namespace std;

void showMenu() {
    cout << "============================\n";
    cout << "  Planeador de Rotas Urbanas \n";
    cout << "============================\n";
    cout << "1. Melhor Rota e Rota Alternativa Independente\n";
    cout << "2. Rota com Restrições \n";
	cout << "3. Rota Sustentável \n";
    cout << "4. Sair\n";
    cout << "Escolha uma opcao: ";
}

string extractValue(const string& line) {
    size_t pos = line.find(':');
    if (pos == string::npos) return "";
    string value = line.substr(pos + 1);
    value.erase(0, value.find_first_not_of(" \t\r\n"));
    value.erase(value.find_last_not_of(" \t\r\n") + 1);
    return value;
}

set<pair<string, string>> parseAvoidSegments(const string& segmentStr, Graph& graph) {
    set<pair<string, string>> segments;
    if (segmentStr.empty()) return segments;

    stringstream ss(segmentStr);
    string segment;
    while (getline(ss, segment, ')')) {
        size_t start = segment.find('(');
        if (start == string::npos) continue;

        string pairStr = segment.substr(start + 1);
        size_t comma = pairStr.find(',');
        if (comma == string::npos) continue;

        string id1 = pairStr.substr(0, comma);
        string id2 = pairStr.substr(comma + 1);

        string code1 = graph.getCodeFromId(id1);
        string code2 = graph.getCodeFromId(id2);

        if (!code1.empty() && !code2.empty()) {
            segments.insert({code1, code2});
        }
    }
    return segments;
}

void processBatchFile(Graph& graph, const string& filenameIn, const string& filenameOut) {
    ifstream input(filenameIn);
    ofstream output(filenameOut);

    if (!input || !output) {
        cerr << "Erro ao abrir input.txt ou output.txt\n";
        return;
    }

    string line;
    while (getline(input, line)) {
        if (line.empty()) continue;

        if (line.find("Mode:") == 0) {
            string mode = extractValue(line);
            getline(input, line); string sourceId = extractValue(line);
            getline(input, line); string destId = extractValue(line);

            string sourceCode = graph.getCodeFromId(sourceId);
            string destCode = graph.getCodeFromId(destId);

            if (sourceCode.empty() || destCode.empty()) {
                output << "Source:" << sourceId << "\n";
                output << "Destination:" << destId << "\n";
                output << "ERRO: ID inválido. Ignorando este pedido.\n\n";
                continue;
            }

            if (mode == "driving" || mode == "walking") {
                auto [best, alt, timeBest, timeAlt] = graph.findRoutes(sourceCode, destCode, mode);
                output << "Source:" << sourceId << "\n";
                output << "Destination:" << destId << "\n";
                graph.displayRoutes(best, timeBest, "Best" + mode + "Route", output);
                graph.displayRoutes(alt, timeAlt, "Alternative" + mode + "Route", output);
                output << "\n";

            } else if (mode == "driving-restricted") {
                getline(input, line); string avoidNodesStr = extractValue(line);
                getline(input, line); string avoidSegmentsStr = extractValue(line);
                getline(input, line); string includeNodeId = extractValue(line);

                unordered_set<string> avoidCodes;
                if (!avoidNodesStr.empty()) {
                    stringstream ss(avoidNodesStr);
                    string id;
                    while (getline(ss, id, ',')) {
                        string code = graph.getCodeFromId(id);
                        if (!code.empty()) avoidCodes.insert(code);
                    }
                }

                string includeCode = graph.getCodeFromId(includeNodeId);
                set<pair<string, string>> avoidSegments = parseAvoidSegments(avoidSegmentsStr, graph);

                auto [route, time] = graph.findRestrictedRoute(sourceCode, destCode, "driving", avoidCodes, avoidSegments, includeCode);

                output << "Source:" << sourceId << "\n";
                output << "Destination:" << destId << "\n";
                if (route.empty()) {
                    output << "RestrictedDrivingRoute:none\n\n";
                } else {
                    output << "RestrictedDrivingRoute:";
                    for (size_t i = 0; i < route.size(); ++i) {
                        output << route[i];
                        if (i < route.size() - 1) output << ",";
                    }
                    output << "(" << time << ")\n\n";
                }

            } else if (mode == "driving-walking") {
                getline(input, line); string maxWalkTimeStr = extractValue(line);
                getline(input, line); string avoidNodesStr = extractValue(line);
                getline(input, line); string avoidSegmentsStr = extractValue(line);
                int maxWalkTime = maxWalkTimeStr.empty() ? 0 : stoi(maxWalkTimeStr);
            
                unordered_set<string> avoidCodes;
                if (!avoidNodesStr.empty()) {
                    stringstream ss(avoidNodesStr);
                    string id;
                    while (getline(ss, id, ',')) {
                        string code = graph.getCodeFromId(id);
                        if (!code.empty()) avoidCodes.insert(code);
                    }
                }
            
                set<pair<string, string>> avoidSegments = parseAvoidSegments(avoidSegmentsStr, graph);
            
                auto [fullRoute, totalTime, drivingRoute, drivingTime, walkingRoute, walkingTime, parkingNode] =
                    graph.findEnvironmentallyFriendlyRoute(sourceCode, destCode, maxWalkTime, avoidCodes, avoidSegments);
            
                output << "Source:" << sourceId << "\n";
                output << "Destination:" << destId << "\n";
            
                if (fullRoute.empty()) {
                    output << "DrivingRoute:none\n";
                    output << "ParkingNode:none\n";
                    output << "WalkingRoute:none\n";
                    output << "TotalTime:\n";
                    output << "Message:No possible route with max. walking time of " << maxWalkTime << " minutes\n\n";
            
                    vector<tuple<vector<string>, int, string, vector<string>, int, int>> suggestions;
                    unordered_set<string> usedParking;
            
                    for (int newMaxTime = maxWalkTime + 1; newMaxTime <= 15; ++newMaxTime) {
                        auto alternatives = graph.findTopEnvironmentallyFriendlyRoutes(sourceCode, destCode, newMaxTime, avoidCodes, avoidSegments, 10);
            
                        for (const auto& alt : alternatives) {
                            auto [full, total, drive, driveTime, walk, walkTime, parking] = alt;
            
                            if (usedParking.count(parking)) continue;
            
                            suggestions.push_back({drive, driveTime, parking, walk, walkTime, total});
                            usedParking.insert(parking);
            
                            if (suggestions.size() == 2) break;
                        }
            
                        if (suggestions.size() == 2) break;
                    }
            
                    if (suggestions.empty()) {
                        output << "No approximate alternatives found.\n";
                    } else {
                        output << "Approximate Alternatives:\n";
                        for (size_t i = 0; i < suggestions.size(); ++i) {
                            auto [droute, dtime, pnode, wroute, wtime, ttime] = suggestions[i];
                            output << "DrivingRoute" << (i + 1) << ":";
                            for (size_t j = 0; j < droute.size(); ++j) {
                                output << droute[j];
                                if (j < droute.size() - 1) output << ",";
                            }
                            output << "(" << dtime << ")\n";
                            output << "ParkingNode" << (i + 1) << ":" << graph.getIdFromCode(pnode) << "\n";
                            output << "WalkingRoute" << (i + 1) << ":";
                            for (size_t j = 0; j < wroute.size(); ++j) {
                                output << wroute[j];
                                if (j < wroute.size() - 1) output << ",";
                            }
                            output << "(" << wtime << ")\n";
                            output << "TotalTime" << (i + 1) << ":" << ttime << "\n";
                        }
                    }
            
                } else {
                    graph.displayRoutes(drivingRoute, drivingTime, "DrivingRoute", output);
                    output << "ParkingNode:" << graph.getIdFromCode(parkingNode) << "\n";
                    graph.displayRoutes(walkingRoute, walkingTime, "WalkingRoute", output);
                    output << "TotalTime:" << totalTime << "\n\n";
                }
            }
             else {
                output << "Modo inválido: " << mode << "\n\n";
            }
        }
    }
}

int main() {
    Graph graph;
    graph.loadLocations("data/Locations.csv");
    graph.loadDistances("data/Distances.csv");

    int option;

    do {
        showMenu();
        cin >> option;

        if (option == 1) {
            int inputChoice;
            cout << "\n[1] Calcular Melhor Rota\n";
            cout << "1. Ler parâmetros do ficheiro input.txt\n";
            cout << "2. Introduzir parâmetros manualmente\n";
            cout << "Escolha: ";
            cin >> inputChoice;

            if (inputChoice == 1) {
                processBatchFile(graph, "input.txt", "output.txt");
                cout << "Resultados escritos em main/output.txt\n";
            } else if (inputChoice == 2) {
                string sourceId, destinationId, mode;
                cout << "Modo (driving/walking): ";
                cin >> mode;
                cout << "Origem (ID): ";
                cin >> sourceId;
                cout << "Destino (ID): ";
                cin >> destinationId;

                string sourceCode = graph.getCodeFromId(sourceId);
                string destinationCode = graph.getCodeFromId(destinationId);

                if (sourceCode.empty() || destinationCode.empty()) {
                    cout << "ID invalido. Tente novamente.\n";
                    continue;
                }

                auto [bestRoute, altRoute, bestTime, altTime] = graph.findRoutes(sourceCode, destinationCode, mode);

                cout << "Source:" << sourceId << "\n";
                cout << "Destination:" << destinationId << "\n";

                graph.displayRoutes(bestRoute, bestTime, "Best" + mode + "Route", cout);
                graph.displayRoutes(altRoute, altTime, "Alternative" + mode + "Route", cout);
            } else {
                cout << "Opcao invalida. Tente novamente.\n";
            }

        } else if (option == 2) {
            int inputChoice;
            cout << "\n[2] Calcular Rota com Restrições \n";
            cout << "1. Ler parâmetros do ficheiro input.txt\n";
            cout << "2. Introduzir parâmetros manualmente\n";
            cout << "Escolha: ";
            cin >> inputChoice;

            if (inputChoice == 1) {
                processBatchFile(graph, "input.txt", "output.txt");
                cout << "Resultados escritos em main/output.txt\n";
            } else if (inputChoice == 2) {
                string sourceId, destId, avoidStr, segmentStr, includeId;
                cout << "Origem (ID): ";
                cin >> sourceId;
                cout << "Destino (ID): ";
                cin >> destId;
                cin.ignore();
                cout << "IDs a evitar (separados por vírgula): ";
                getline(cin, avoidStr);
                cout << "Segmentos a evitar (ex: (1,2),(3,4)): ";
                getline(cin, segmentStr);
                cout << "ID obrigatório a incluir no caminho: ";
                getline(cin, includeId);

                string sourceCode = graph.getCodeFromId(sourceId);
                string destCode = graph.getCodeFromId(destId);
                string includeCode = graph.getCodeFromId(includeId);

                if (sourceCode.empty() || destCode.empty()) {
                    cout << "ID invalido. Tente novamente.\n";
                    continue;
                }

                unordered_set<string> avoidCodes;
                stringstream ss(avoidStr);
                string id;
                while (getline(ss, id, ',')) {
                    string code = graph.getCodeFromId(id);
                    if (!code.empty()) avoidCodes.insert(code);
                }

                set<pair<string, string>> avoidSegments = parseAvoidSegments(segmentStr, graph);

                auto [route, time] = graph.findRestrictedRoute(
                    sourceCode, destCode, "driving", avoidCodes, avoidSegments, includeCode);

                cout << "Source:" << sourceId << "\n";
                cout << "Destination:" << destId << "\n";

                if (route.empty()) {
                    cout << "RestrictedDrivingRoute:none\n";
                } else {
                    cout << "RestrictedDrivingRoute:";
                    for (size_t i = 0; i < route.size(); ++i) {
                        cout << route[i];
                        if (i < route.size() - 1) cout << ",";
                    }
                    cout << "(" << time << ")\n";
                }
            } else {
                cout << "Opcao invalida. Tente novamente.\n";
            }

        }   else if (option == 3) {
            int inputChoice;
            cout << "\n[3] Calcular Rota Sustentável \n";
            cout << "1. Ler parâmetros do ficheiro input.txt\n";
            cout << "2. Introduzir parâmetros manualmente\n";
            cout << "Escolha: ";
            cin >> inputChoice;
        
            if (inputChoice == 1) {
                processBatchFile(graph, "input.txt", "output.txt");
                cout << "Resultados escritos em main/output.txt\n";
            }
            else if (inputChoice == 2) {
                int maxWalkingTime;
                string sourceId, destId, avoidStr, segmentStr;
                cout << "Origem (ID): ";
                cin >> sourceId;
                cout << "Destino (ID): ";
                cin >> destId;
                cout << "Max Walking Time: ";
                cin >> maxWalkingTime;
                cin.ignore();
                cout << "IDs a evitar (separados por vírgula): ";
                getline(cin, avoidStr);
                cout << "Segmentos a evitar (ex: (1,2),(3,4)): ";
                getline(cin, segmentStr);
        
                string sourceCode = graph.getCodeFromId(sourceId);
                string destCode = graph.getCodeFromId(destId);
        
                if (sourceCode.empty() || destCode.empty()) {
                    cout << "ID invalido. Tente novamente.\n";
                    continue;
                }
        
                unordered_set<string> avoidCodes;
                stringstream ss(avoidStr);
                string id;
                while (getline(ss, id, ',')) {
                    string code = graph.getCodeFromId(id);
                    if (!code.empty()) avoidCodes.insert(code);
                }
        
                set<pair<string, string>> avoidSegments = parseAvoidSegments(segmentStr, graph);
        
                auto [fullRoute, totalTime, drivingRoute, drivingTime, walkingRoute, walkingTime, parkingNode] =
                    graph.findEnvironmentallyFriendlyRoute(sourceCode, destCode, maxWalkingTime, avoidCodes, avoidSegments);
        
                cout << "Source:" << sourceId << "\n";
                cout << "Destination:" << destId << "\n";
        
                if (fullRoute.empty()) {
                    cout << "DrivingRoute:none\n";
                    cout << "ParkingNode:none\n";
                    cout << "WalkingRoute:none\n";
                    cout << "TotalTime:\n";
                    cout << "No possible route with max. walking time of " << maxWalkingTime << " minutes\n\n";
        
                    vector<tuple<vector<string>, int, string, vector<string>, int, int>> suggestions;
        
                    unordered_set<string> usedParkingNodes;

                    for (int newMaxTime = maxWalkingTime + 1; newMaxTime <= 15; ++newMaxTime) {
                        auto alternatives = graph.findTopEnvironmentallyFriendlyRoutes(sourceCode, destCode, newMaxTime, avoidCodes, avoidSegments, 2);
                    
                        for (const auto& alt : alternatives) {
                            auto [full, total, drive, driveTime, walk, walkTime, parking] = alt;
                            suggestions.push_back({drive, driveTime, parking, walk, walkTime, total});
                            if (suggestions.size() == 2) break;
                        }
                    
                        if (suggestions.size() == 2) break;
                    }                                      
        
                    if (suggestions.empty()) {
                        cout << "No approximate alternatives found.\n";
                    } else {
                        cout << "Approximate Alternatives:\n";
                        for (size_t i = 0; i < suggestions.size(); ++i) {
                            auto [droute, dtime, pnode, wroute, wtime, ttime] = suggestions[i];
                            cout << "DrivingRoute" << (i + 1) << ":";
                            for (size_t j = 0; j < droute.size(); ++j) {
                                cout << droute[j];
                                if (j < droute.size() - 1) cout << ",";
                            }
                            cout << "(" << dtime << ")\n";
                            cout << "ParkingNode" << (i + 1) << ":" << graph.getIdFromCode(pnode) << "\n";
                            cout << "WalkingRoute" << (i + 1) << ":";
                            for (size_t j = 0; j < wroute.size(); ++j) {
                                cout << wroute[j];
                                if (j < wroute.size() - 1) cout << ",";
                            }                            
                            cout << "(" << wtime << ")\n";
                            cout << "TotalTime" << (i + 1) << ":" << ttime << "\n";
                        }
                    }
                }
                else {
                    cout << "DrivingRoute:";
                    for (size_t j = 0; j < drivingRoute.size(); ++j) {
                        cout << drivingRoute[j];
                        if (j < drivingRoute.size() - 1) cout << ",";
                    }
                    cout << "(" << drivingTime << ")\n";
                
                    cout << "ParkingNode:" << graph.getIdFromCode(parkingNode) << "\n";
                
                    cout << "WalkingRoute:";
                    for (size_t j = 0; j < walkingRoute.size(); ++j) {
                        cout << walkingRoute[j];
                        if (j < walkingRoute.size() - 1) cout << ",";
                    }
                    cout << "(" << walkingTime << ")\n";
                
                    cout << "TotalTime:" << totalTime << "\n"; 
                }
            }
        }
    } while (option != 4);

    return 0;
}
