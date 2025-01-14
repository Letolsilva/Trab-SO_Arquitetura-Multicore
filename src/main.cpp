#include "functions.hpp"
#include "monitora.hpp"
#include "memoria.hpp"
#include "include.hpp"
#include "SO.hpp"

int PC = 0;
int CLOCK = 0;
int op = 0;
bool perifericos[NUM_PERIFERICOS] = {false};
vector<int> principal;
vector<mutex> mutexCores(NUM_CORE);
// vector<int> processosNaMemoria;

int main()
{

    using namespace std::chrono;

    cout << "\n\n\t ----------{Escolha a Politica de Escalanomento}---------- " << endl;
    cout << "\n\t\t [1] = FCFS.";
    cout << "\n\t\t [2] = Shortest Remaining Job First";
    cout << "\n\t\t [3] = Prioridade";
    cout << "\n\t\t [>] = ";
    cin >> op;
    cout << "\n\n\t --------------------------------------------------------- " << endl;


    auto inicio = high_resolution_clock::now();
    pthread_t monitor = {};
    int status_monitor = pthread_create(&monitor, nullptr, start, nullptr);
    if (status_monitor != 0)
    {
        cerr << "Erro ao criar a thread do Monitor!" << endl;
        return 1;
    }
    pthread_join(monitor, nullptr);

    auto fim = high_resolution_clock::now();
    auto duracao = duration_cast<nanoseconds>(fim - inicio);
    cout << "Tempo de execução: " << duracao.count() << " nanosegundos." << endl;

    return 0;
}