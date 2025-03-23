#include "graph/graph.h"
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

void showMenu() {
    cout << "============================\n";
    cout << "  Planeador de Rotas Urbanas \n";
    cout << "============================\n";
    cout << "1. Melhor Rota\n";
    cout << "2. Rota Alternativa Independente\n";
    cout << "3. Sair\n";
    cout << "Escolha uma opcao: ";
}

int main() {
    Graph graph;
    graph.loadLocations("data/Locations.csv");
    graph.loadDistances("data/Distances.csv");

    int option;
    string sourceId, destinationId, mode;

    do {
        showMenu();
        cin >> option;
        if (option == 1 || option == 2) {
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

            if (option == 1) {
                auto bestRoute = graph.findBestRoute(sourceCode, destinationCode, mode);
                graph.displayRoute(bestRoute, graph.getRouteTime(bestRoute, mode), "BestDrivingRoute", cout);
            } else if (option == 2) {
                auto altRoute = graph.findAlternativeRoute(sourceCode, destinationCode, mode);
                graph.displayRoute(altRoute, graph.getRouteTime(altRoute, mode), "AlternativeDrivingRoute", cout);
            }
        } else if (option == 3) {
            cout << "Encerrando o programa...\n";
        } else {
            cout << "Opcao invalida. Tente novamente.\n";
        }
    } while (option != 3);

    return 0;
}
