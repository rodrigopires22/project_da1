#include "graph/graph.h" 
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_set>
using namespace std;

void showMenu() {
    cout << "============================\n";
    cout << "  Planeador de Rotas Urbanas \n";
    cout << "============================\n";
    cout << "1. Melhor Rota e Rota Alternativa Independente\n";
    cout << "2. Rota com Restrições \n";
    cout << "3. Sair\n";
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

void processBatchFile(Graph& graph, const string& filenameIn, const string& filenameOut) {
    ifstream input(filenameIn);
    ofstream output(filenameOut);

    if (!input || !output) {
        cerr << "Erro ao abrir input.txt ou output.txt\n";
        return;
    }

    string line, mode, sourceId, destId;

    while (getline(input, line)) {
        if (line.empty()) continue;

        if (line.find("Mode:") == 0) {
            mode = extractValue(line);
            getline(input, line);
            sourceId = extractValue(line);
            getline(input, line);
            destId = extractValue(line);

            string sourceCode = graph.getCodeFromId(sourceId);
            string destCode = graph.getCodeFromId(destId);

            if (sourceCode.empty() || destCode.empty()) {
                output << "ERRO: ID inválido. Ignorando este pedido.\n\n";
                continue;
            }

            if (mode == "driving") {
                // T2.2 - Restricted Route
                string avoidNodesStr;
                getline(input, line); // AvoidNodes
                avoidNodesStr = extractValue(line);

                getline(input, line); // Skip AvoidSegments
                getline(input, line); // Skip IncludeNode

                unordered_set<string> avoidCodes;
                if (!avoidNodesStr.empty()) {
                    stringstream ss(avoidNodesStr);
                    string id;
                    while (getline(ss, id, ',')) {
                        string code = graph.getCodeFromId(id);
                        if (!code.empty()) avoidCodes.insert(code);
                    }
                }

                auto [restrictedRoute, totalTime] = graph.findRestrictedRoute(sourceCode, destCode, mode, avoidCodes);

                output << "Source:" << sourceId << "\n";
                output << "Destination:" << destId << "\n";

                if (restrictedRoute.empty()) {
                    output << "RestrictedDrivingRoute:none\n\n";
                } else {
                    output << "RestrictedDrivingRoute:";
                    for (size_t i = 0; i < restrictedRoute.size(); ++i) {
                        output << restrictedRoute[i];
                        if (i < restrictedRoute.size() - 1) output << ",";
                    }
                    output << "(" << totalTime << ")\n\n";
                }

            } else {
                // T2.1 - Best and Alternative Route
                auto [best, alt, timeBest, timeAlt] = graph.findRoutes(sourceCode, destCode, mode);

                output << "Source:" << sourceId << "\n";
                output << "Destination:" << destId << "\n";

                graph.displayRoutes(best, timeBest, "Best" + mode + "Route", output);
                graph.displayRoutes(alt, timeAlt, "Alternative" + mode + "Route", output);
                output << "\n";
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
                string sourceId, destId, avoidStr;
                cout << "Origem (ID): ";
                cin >> sourceId;
                cout << "Destino (ID): ";
                cin >> destId;
                cin.ignore();
                cout << "IDs a evitar (separados por vírgula): ";
                getline(cin, avoidStr);

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

                auto [route, time] = graph.findRestrictedRoute(sourceCode, destCode, "driving", avoidCodes);

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

        } else if (option == 3) {
            cout << "Encerrando o programa...\n";
        } else {
            cout << "Opcao invalida. Tente novamente.\n";
        }

    } while (option != 3);

    return 0;
}
