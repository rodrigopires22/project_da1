#include "graph/graph.h" 
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

void showMenu() {
    cout << "============================\n";
    cout << "  Planeador de Rotas Urbanas \n";
    cout << "============================\n";
    cout << "1. Melhor Rota e Rota Alternativa Independente\n";
    cout << "2. Sair\n";
    cout << "Escolha uma opcao: ";
}

void processBatchFile(Graph& graph, const std::string& filenameIn, const std::string& filenameOut) {
    std::ifstream input(filenameIn);
    std::ofstream output(filenameOut);

    if (!input || !output) {
        std::cerr << "Erro ao abrir input.txt ou output.txt\n";
        return;
    }

    auto extractValue = [](const std::string& line) {
        size_t pos = line.find(':');
        if (pos == std::string::npos) return std::string();
        std::string value = line.substr(pos + 1);
        value.erase(0, value.find_first_not_of(" \t\r\n"));
        value.erase(value.find_last_not_of(" \t\r\n") + 1);
        return value;
    };

    std::string line, mode, sourceId, destId;

    while (std::getline(input, line)) {
        if (line.empty()) continue;

        if (line.find("Mode:") == 0) {
            mode = extractValue(line);

            std::getline(input, line);
            sourceId = extractValue(line);

            std::getline(input, line);
            destId = extractValue(line);

            std::string sourceCode = graph.getCodeFromId(sourceId);
            std::string destCode = graph.getCodeFromId(destId);

            if (sourceCode.empty() || destCode.empty()) {
                output << "ERRO: ID inválido. Ignorando este pedido.\n\n";
                continue;
            }

            auto [best, alt, timeBest, timeAlt] = graph.findRoutes(sourceCode, destCode, mode);

            output << "Source:" << sourceId << "\n";
            output << "Destination:" << destId << "\n";

            graph.displayRoutes(best, timeBest, "Best" + mode + "Route", output);
            graph.displayRoutes(alt, timeAlt, "Alternative" + mode + "Route", output);
            output << "\n";
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
            cout << "Encerrando o programa...\n";
        } else {
            cout << "Opcao invalida. Tente novamente.\n";
        }
    } while (option != 2);

    return 0;
}
