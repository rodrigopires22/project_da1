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

int main() {
    Graph graph;
    graph.loadLocations("data/Locations.csv");
    graph.loadDistances("data/Distances.csv");

    int option;
    string sourceId, destinationId, mode;

    do {
        showMenu();
        cin >> option;
        if (option == 1) {
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

            auto [bestRoute, altRoute] = graph.findRoutes(sourceCode, destinationCode, mode);

            cout << "Source:" << sourceId << "\n";
            cout << "Destination:" << destinationId << "\n";

            if (!bestRoute.empty()) {
                graph.displayRoutes(bestRoute, graph.getRouteTime(bestRoute, mode), "BestDrivingRoute", cout);
            } else {
                cout << "BestDrivingRoute:none\n";
            }

            if (!altRoute.empty()) {
                graph.displayRoutes(altRoute, graph.getRouteTime(altRoute, mode), "AlternativeDrivingRoute", cout);
            } else {
                cout << "AlternativeDrivingRoute:none\n";
            }

        } else if (option == 2) {
            cout << "Encerrando o programa...\n";
        } else {
            cout << "Opcao invalida. Tente novamente.\n";
        }
    } while (option != 2);

    return 0;
}
