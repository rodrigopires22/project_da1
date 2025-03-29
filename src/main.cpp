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
        string id2 = pairStr.substr(comma + 1);c

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

    string line, mode, sourceId, destId;

    while (getline(input, line)) {
        if (line.empty()) continue;

        if (line.find("Mode:") == 0) {
            mode = extractValue(line);
            getline(input, line);
            sourceId = extractValue(line);
            getline(input, line);
            destId = extractValue(line);
            getline(input, line);
            string avoidNodesStr = extractValue(line);
            getline(input, line);
            string avoidSegmentsStr = extractValue(line);
            getline(input, line);
            string includeNodeId = extractValue(line);

            string sourceCode = graph.getCodeFromId(sourceId);
            string destCode = graph.getCodeFromId(destId);
            string includeCode = graph.getCodeFromId(includeNodeId);

            if (sourceCode.empty() || destCode.empty()) {
                output << "ERRO: ID inválido. Ignorando este pedido.\n\n";
                continue;
            }

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

            auto [restrictedRoute, totalTime] = graph.findRestrictedRoute(
                sourceCode, destCode, mode, avoidCodes, avoidSegments, includeCode);

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

        } else if (option == 3) {
            cout << "Encerrando o programa...\n";
        } else {
            cout << "Opcao invalida. Tente novamente.\n";
        }

    } while (option != 3);

    return 0;
}
