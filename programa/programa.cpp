#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <windows.h> // Necesario para SetConsoleCursorPosition
#include <vector>
#include <atomic>
#include <mutex>

using namespace std;

const int META = 50; // La distancia de la meta
mutex mtx; // Mutex para proteger la impresión del ganador

// Clase Animal para representar cada participante en la carrera
class Animal {
public:
    Animal(string n, int v, COORD c) : nombre(n), velocidad(v), posicion(0), coord(c) {}

    // Función que representa el movimiento del animal
    void correr(atomic<bool>& metaAlcanzada) {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> distribucionAvance(1, velocidad);

        while (!metaAlcanzada && posicion < META) {
            int avance = distribucionAvance(gen);
            posicion += avance;

            // Simula pausas aleatorias
            this_thread::sleep_for(chrono::milliseconds(200));

            // Imprime el progreso en forma de puntos, dejando el nombre en la posición inicial
            COORD posicionCursor = coord;
            posicionCursor.X += nombre.length() + 2; // Ajuste para que los puntos empiecen después del nombre

            SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), posicionCursor);
            cout << string(posicion, '.');

            // Comprobación si alcanzó la meta
            if (posicion >= META) {
                // Asegurarse de que solo el primer hilo imprima el mensaje de ganador
                lock_guard<mutex> lock(mtx);
                if (!metaAlcanzada) { // Solo el primer animal que llega a la meta imprime el mensaje
                    metaAlcanzada = true;
                    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 0, (SHORT)(coord.Y + 4) });
                    cout << "\n" << nombre << " ha ganado la carrera!" << endl;
                }
            }
        }
    }

    // Función para imprimir el nombre en la posición inicial
    void mostrarNombre() const {
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
        cout << nombre;
    }

private:
    string nombre;
    int velocidad;
    int posicion;
    COORD coord;
};

int main() {
    // Define participantes
    vector<Animal> animales = {
        Animal("Liebre", 5, {0, 1}),
        Animal("Tortuga", 3, {0, 2}),
        Animal("Perro", 4, {0, 3})
    };

    atomic<bool> metaAlcanzada(false); // Indica si algún animal ha llegado a la meta

    // Muestra los nombres en su posición inicial
    for (const auto& animal : animales) {
        animal.mostrarNombre();
    }

    // Crear hilos para cada animal y lanzarlos
    vector<thread> hilos;
    for (auto& animal : animales) {
        hilos.push_back(thread(&Animal::correr, &animal, ref(metaAlcanzada)));
    }

    // Espera a que todos los hilos terminen
    for (auto& hilo : hilos) {
        hilo.join();
    }

    // Pausa al final para que el usuario pueda ver el mensaje de ganador
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 0, 10 });
    cout << "La carrera ha terminado. Presiona cualquier tecla para cerrar.";
    cin.get();

    return 0;
}
