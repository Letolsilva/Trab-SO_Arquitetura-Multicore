#include "unidadeControle.hpp"
#include "memoria.hpp"
#include "functions.hpp"
#include "SO.hpp"

vector<PCB> memoria;
vector<Page> memoryPages; // Memoria
mutex mutexProcessos;     // Mutex para controle de acesso ao vetor

// Função auxiliar para salvar no arquivo
void salvarNoArquivo(const string &conteudo)
{
    filesystem::create_directories("./output");
    ofstream arquivo("./output/output.data", ios::app);
    if (arquivo.is_open())
    {
        arquivo << conteudo << endl;
        arquivo.close();
    }
    else
    {
        cerr << "Erro ao abrir o arquivo output.data!" << endl;
    }
}

// Função que processa um processo
void *processarProcesso(void *arg)
{
    auto coreIndexPtr = std::unique_ptr<int>(reinterpret_cast<int *>(arg)); // Ponteiro inteligente

    int coreIndex = *coreIndexPtr;

    auto registradores = std::make_unique<int[]>(8);
    int var = 0;

    while (true)
    {
        int idProcesso = obterProximoProcesso();

        if (idProcesso == -1)
        {
            usleep(1000);
            continue;
        }

        Page paginaAtual;
        PCB processoAtual;

        // Buscar o processo na memória
        if (!buscarProcessoNaMemoria(idProcesso, paginaAtual, processoAtual))
        {
            break; // Se não encontrar o processo, termina o loop
        }

        processoAtual.estado = EXECUTANDO;
        int quantumInicial = processoAtual.quantum;

        stringstream ss;
        ss << "Thread_CPU" << coreIndex << " processando processo ID=" << processoAtual.id << endl;
        ss << "Estado: " << obterEstadoProcesso(processoAtual) << endl;

        // Executar as instruções do processo
        processarInstrucoes(processoAtual);

        // Atualizar timestamp e salvar no arquivo
        atualizarESalvarProcesso(processoAtual, ss, quantumInicial, var);

        usleep(1000);

        // Finaliza a execução do processo
        ss << "Resultado: " << processoAtual.resultado << endl;
        ss << "Quantum Final: " << processoAtual.quantum << endl;
        ss << "Timestamp Final: " << processoAtual.timestamp << endl;
        ss << "Prioridade: " << processoAtual.prioridade << endl;
        ss << "Estado Final: " << obterEstadoProcesso(processoAtual) << endl;
        ss << "=============================" << endl;

        salvarNoArquivo(ss.str());

        if (memoryPages.empty())
        {
            break;
        }
    }

    pthread_exit(nullptr);
}

// Função que busca o processo na memória
bool buscarProcessoNaMemoria(int idProcesso, Page &paginaAtual, PCB &processoAtual)
{
    bool encontrou = false;

    lock_guard<mutex> lock(mutexProcessos);
    for (auto it = memoryPages.begin(); it != memoryPages.end(); ++it)
    {
        if (it->pcb.id == idProcesso)
        {
            paginaAtual = *it;
            memoryPages.erase(it);
            encontrou = true;
            break;
        }
    }

    if (encontrou)
    {
        processoAtual = paginaAtual.pcb;
        processoAtual.timestamp = 0; // Inicializando timestamp
    }

    return encontrou;
}

// Função que retorna o estado do processo em formato de string
string obterEstadoProcesso(const PCB &processo)
{
    if (processo.estado == PRONTO)
        return "PRONTO";
    if (processo.estado == BLOQUEADO)
        return "BLOQUEADO";
    return "EXECUTANDO";
}

// Função que processa as instruções do processo
void processarInstrucoes(PCB &processoAtual)
{
    for (const auto &instrucao : processoAtual.instrucoes)
    {
        if (processoAtual.quantum <= 0)
        {
            cout << "Quantum esgotado para o processo ID=" << processoAtual.id << ". Preempcao ocorrida." << endl;
            processoAtual.estado = PRONTO;
            atualizarListaCircular(processoAtual.id);
            break;
        }

        try
        {
            processoAtual.estado = EXECUTANDO;
            UnidadeControle(processoAtual.registradores.data(), instrucao, processoAtual.quantum, processoAtual);
            processoAtual.estado = PRONTO;
        }
        catch (const runtime_error &e)
        {
            cout << "Erro durante execução do processo ID=" << processoAtual.id << endl;
            processoAtual.estado = BLOQUEADO;
            break;
        }
    }
}

// Função que atualiza o processo e salva as informações no arquivo
void atualizarESalvarProcesso(PCB &processoAtual, stringstream &ss, int quantumInicial, int &var)
{
    ss << "=== Processo ID: " << processoAtual.id << " ===" << endl;
    ss << "Quantum Inicial: " << quantumInicial << endl;
    ss << "Timestamp Inicial: " << processoAtual.timestamp << endl;
    ss << "Instruções:" << endl;

    for (const auto &instrucao : processoAtual.instrucoes)
    {
        ss << "  - " << instrucao << endl;
    }

    var += (quantumInicial - processoAtual.quantum);
    processoAtual.timestamp = var;
}
