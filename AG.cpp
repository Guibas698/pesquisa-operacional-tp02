#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <utility>
#include <string>
#include <cmath>
#include <algorithm>
#include <random>
#include <chrono>
#include <limits>

#include "readFile.h"
#include "readFile.c"

using namespace std;


// Funcao do TP 01 que recebe a sequencia de arcos e verifica se o caminho eh valido (de s=1 ate t=n)
bool verificarCaminhoValido(
    const vector<pair<int, int>>& arcos,
    int n,
    int s,
    int t,
    const vector<vector<double>>& c,
    double* custoTotal = NULL
) {
    if (custoTotal != NULL) *custoTotal = 0.0;

    // checa se origem e destino sao validos e diferentes
    if (n < 2 || s < 1 || s > n || t < 1 || t > n || s == t) {
        return false;
    }

    // caminho hamiltoniano tem que ter exatamente n - 1 arcos
    if ((int)arcos.size() != n - 1) {
        return false;
    }

    vector<int> grauEntrada(n + 1, 0);
    vector<int> grauSaida(n + 1, 0);
    vector<int> proximo(n + 1, -1);
    double somaCustos = 0.0;

    // percorre os arcos contando grau de entrada/saida e somando o custo
    for (size_t k = 0; k < arcos.size(); k++) {
        int u = arcos[k].first;
        int v = arcos[k].second;

        // nao pode ter vertice fora de 1..n nem auto-laco (u == v)
        if (u < 1 || u > n || v < 1 || v > n || u == v) {
            return false;
        }

        grauSaida[u]++;
        grauEntrada[v]++;
        proximo[u] = v;
        somaCustos += c[u - 1][v - 1];
    }

    // origem s: sai 1 e nao entra nenhum | destino t: entra 1 e nao sai nenhum
    if (grauSaida[s] != 1 || grauEntrada[s] != 0) return false;
    if (grauEntrada[t] != 1 || grauSaida[t] != 0) return false;

    // vertices do meio: tem que entrar 1 e sair 1
    for (int k = 1; k <= n; k++) {
        if (k == s || k == t) continue;
        if (grauEntrada[k] != 1 || grauSaida[k] != 1) {
            return false;
        }
    }

    // caminha de s ate t para garantir que nao tem ciclo/subrota isolada
    vector<bool> visitado(n + 1, false);
    int atual = s;
    int contVisitados = 0;

    while (atual != -1) {
        if (visitado[atual]) return false; // achou ciclo
        visitado[atual] = true;
        contVisitados++;

        if (atual == t) break;
        atual = proximo[atual];
    }

    // confirma se parou em t e se passou por todos os n vertices
    if (atual != t || contVisitados != n) {
        return false;
    }

    if (custoTotal != NULL) *custoTotal = somaCustos;
    return true;
}


// Converte um vetor de vertices (0-indexado) para lista de arcos (1-indexado)
vector<pair<int, int>> converterCaminhoParaArcos(const vector<int>& caminho) {
    vector<pair<int, int>> arcos;
    arcos.reserve(caminho.size() > 0 ? caminho.size() - 1 : 0);
    for (size_t i = 0; i + 1 < caminho.size(); i++) {
        arcos.push_back(make_pair(caminho[i] + 1, caminho[i + 1] + 1));
    }
    return arcos;
}


// Calcula o custo total de uma sequencia de vertices (0-indexado)
double calcularCustoCaminho(const vector<int>& caminho, const vector<vector<double>>& c) {
    double custo = 0.0;
    for (size_t i = 0; i + 1 < caminho.size(); i++) {
        custo += c[caminho[i]][caminho[i + 1]];
    }
    return custo;
}


// ============================================================================
// ATIVIDADE 1 - Heuristica para a Solucao Inicial (Vizinho Mais Proximo)
// ============================================================================
// Comecando do vertice de origem s, adiciona a cada passo o vertice ainda nao
// visitado de menor custo a partir do vertice atual, deixando o vertice de
// destino obrigatorio t para fechar o caminho no ultimo passo.
vector<int> heuristicaSolucaoInicial(
    int n,
    int s,
    int t,
    const vector<vector<double>>& c,
    double* custoFinal = NULL
) {
    vector<bool> visitado(n, false);
    vector<int> caminho;
    caminho.reserve(n);

    int atual = s;
    caminho.push_back(atual);
    visitado[s] = true;
    visitado[t] = true; // reserva o destino t para ser o ultimo vertice do caminho

    // escolhe os n - 2 vertices intermediarios pelo menor custo a partir do atual
    for (int passo = 0; passo < n - 2; passo++) {
        int melhorProximo = -1;
        double menorCusto = numeric_limits<double>::infinity();

        for (int j = 0; j < n; j++) {
            if (!visitado[j] && c[atual][j] < menorCusto) {
                menorCusto = c[atual][j];
                melhorProximo = j;
            }
        }

        // fallback de seguranca caso todos os custos restantes sejam infinitos
        if (melhorProximo == -1) {
            for (int j = 0; j < n; j++) {
                if (!visitado[j]) {
                    melhorProximo = j;
                    break;
                }
            }
        }

        caminho.push_back(melhorProximo);
        visitado[melhorProximo] = true;
        atual = melhorProximo;
    }

    // por fim, adiciona o vertice de destino t
    caminho.push_back(t);

    if (custoFinal != NULL) {
        *custoFinal = calcularCustoCaminho(caminho, c);
    }

    return caminho;
}


// ============================================================================
// ATIVIDADE 2 - Meta-heuristica: Algoritmo Genetico (AG Puro)
// ============================================================================

struct Individuo {
    vector<int> caminho; // cromossomo: [s, v_1, v_2, ..., v_{n-2}, t]
    double custo;        // valor da funcao objetivo (quanto menor, melhor o fitness)

    bool operator<(const Individuo& outro) const {
        return custo < outro.custo;
    }
};


// Gera a populacao inicial combinando a solucao da heuristica inicial (Atv 1),
// variacoes perturbadas dela e individuos gerados aleatoriamente
vector<Individuo> gerarPopulacaoInicial(
    int tamPopulacao,
    int n,
    int s,
    int t,
    const vector<vector<double>>& c,
    const vector<int>& caminhoInicial,
    mt19937& rng
) {
    vector<Individuo> populacao(tamPopulacao);

    // Individuo 0: a propria solucao da heuristica inicial (Atividade 1)
    populacao[0].caminho = caminhoInicial;
    populacao[0].custo = calcularCustoCaminho(caminhoInicial, c);

    // Se n <= 3, existe apenas 1 caminho hamiltoniano possivel de s ate t
    if (n <= 3) {
        for (int k = 1; k < tamPopulacao; k++) {
            populacao[k] = populacao[0];
        }
        return populacao;
    }

    // Lista base com os vertices intermediarios para gerar permutacoes
    vector<int> baseIntermediarios;
    baseIntermediarios.reserve(n - 2);
    for (int v = 0; v < n; v++) {
        if (v != s && v != t) {
            baseIntermediarios.push_back(v);
        }
    }

    uniform_int_distribution<int> distPos(1, n - 2);

    for (int k = 1; k < tamPopulacao; k++) {
        if (k < tamPopulacao / 3) {
            // Parte inicial da populacao: pequenas perturbacoes da solucao inicial gulosa
            populacao[k].caminho = caminhoInicial;
            int qtdTrocas = 1 + (k % 3);
            for (int r = 0; r < qtdTrocas; r++) {
                int p1 = distPos(rng);
                int p2 = distPos(rng);
                swap(populacao[k].caminho[p1], populacao[k].caminho[p2]);
            }
        } else {
            // Restante da populacao: permutacoes aleatorias dos vertices intermediarios
            shuffle(baseIntermediarios.begin(), baseIntermediarios.end(), rng);
            populacao[k].caminho.resize(n);
            populacao[k].caminho[0] = s;
            for (int i = 0; i < n - 2; i++) {
                populacao[k].caminho[i + 1] = baseIntermediarios[i];
            }
            populacao[k].caminho[n - 1] = t;
        }

        populacao[k].custo = calcularCustoCaminho(populacao[k].caminho, c);
    }

    return populacao;
}


// Selecao por Torneio: sorteia k individuos da populacao e retorna o indice do melhor
int selecaoTorneio(const vector<Individuo>& populacao, int tamTorneio, mt19937& rng) {
    uniform_int_distribution<int> distInd(0, (int)populacao.size() - 1);
    int melhorIdx = distInd(rng);

    for (int i = 1; i < tamTorneio; i++) {
        int candidatoIdx = distInd(rng);
        if (populacao[candidatoIdx].custo < populacao[melhorIdx].custo) {
            melhorIdx = candidatoIdx;
        }
    }
    return melhorIdx;
}


// Operador de Cruzamento OX Classico (Order Crossover) nos n - 2 vertices intermediarios
// Mantem s fixo na posicao 0 e t fixo na posicao n - 1.
// Copia a fatia [corte1..corte2] do pai1 e preenche o restante circularmente a partir de corte2 + 1
// seguindo a ordem dos vertices no pai2 (tambem lidos circularmente a partir de corte2 + 1).
Individuo cruzamentoOX(
    const Individuo& pai1,
    const Individuo& pai2,
    int n,
    int s,
    int t,
    const vector<vector<double>>& c,
    mt19937& rng
) {
    if (n <= 3) {
        return pai1;
    }

    Individuo filho;
    filho.caminho.assign(n, -1);
    filho.caminho[0] = s;
    filho.caminho[n - 1] = t;

    int numInterm = n - 2; // posicoes intermediarias: indices 1 ate n - 2
    uniform_int_distribution<int> distCorte(1, n - 2);
    int corte1 = distCorte(rng);
    int corte2 = distCorte(rng);
    if (corte1 > corte2) swap(corte1, corte2);

    vector<bool> jaUsado(n, false);
    jaUsado[s] = true;
    jaUsado[t] = true;

    // 1. Copia a sub-sequencia do pai1 entre [corte1, corte2] para o filho
    for (int i = corte1; i <= corte2; i++) {
        int v = pai1.caminho[i];
        filho.caminho[i] = v;
        jaUsado[v] = true;
    }

    // 2. Percorre o pai2 circularmente a partir de (corte2 + 1) e preenche o filho circularmente
    int posEscrita = 1 + (corte2 % numInterm);
    for (int k = 0; k < numInterm; k++) {
        int posLeitura = 1 + ((corte2 + k) % numInterm);
        int v = pai2.caminho[posLeitura];
        if (!jaUsado[v]) {
            while (posEscrita >= corte1 && posEscrita <= corte2) {
                posEscrita = 1 + (posEscrita % numInterm);
            }
            filho.caminho[posEscrita] = v;
            jaUsado[v] = true;
            posEscrita = 1 + (posEscrita % numInterm);
        }
    }

    filho.custo = calcularCustoCaminho(filho.caminho, c);
    return filho;
}


// Operador de Mutacao: aplica um movimento aleatorio nos vertices intermediarios
// (Swap, Inversao de trecho, Insercao de vertice ou Deslocamento de bloco)
void aplicarMutacao(
    Individuo& ind,
    const vector<vector<double>>& c,
    double taxaMutacao,
    mt19937& rng
) {
    int n = (int)ind.caminho.size();
    if (n <= 3) return;

    uniform_real_distribution<double> distProb(0.0, 1.0);
    if (distProb(rng) >= taxaMutacao) {
        return;
    }

    uniform_int_distribution<int> distPos(1, n - 2);
    uniform_int_distribution<int> distTipo(0, 3);

    int numMutacoes = (distProb(rng) < 0.35) ? 2 : 1;
    for (int m = 0; m < numMutacoes; m++) {
        int i = distPos(rng);
        int j = distPos(rng);
        while (i == j) {
            j = distPos(rng);
        }

        int tipo = distTipo(rng);
        if (tipo == 0) {
            // Mutacao por Troca (Swap): troca dois vertices intermediarios de posicao
            swap(ind.caminho[i], ind.caminho[j]);
        } else if (tipo == 1) {
            // Mutacao por Inversao: inverte a sub-sequencia entre i e j
            if (i > j) swap(i, j);
            reverse(ind.caminho.begin() + i, ind.caminho.begin() + j + 1);
        } else if (tipo == 2) {
            // Mutacao por Insercao: retira o vertice da posicao i e insere na posicao j
            int vert = ind.caminho[i];
            ind.caminho.erase(ind.caminho.begin() + i);
            ind.caminho.insert(ind.caminho.begin() + j, vert);
        } else {
            // Mutacao por Deslocamento de Bloco: move um pequeno sub-caminho para outra posicao
            if (i > j) swap(i, j);
            int tamBloco = min(j - i + 1, max(1, (n - 2) / 5));
            vector<int> bloco(ind.caminho.begin() + i, ind.caminho.begin() + i + tamBloco);
            ind.caminho.erase(ind.caminho.begin() + i, ind.caminho.begin() + i + tamBloco);
            uniform_int_distribution<int> distNovaPos(1, (int)ind.caminho.size() - 1);
            int novaPos = distNovaPos(rng);
            ind.caminho.insert(ind.caminho.begin() + novaPos, bloco.begin(), bloco.end());
        }
    }

    ind.custo = calcularCustoCaminho(ind.caminho, c);
}


// Executa o Algoritmo Genetico puro e retorna o melhor individuo encontrado
Individuo executarAlgoritmoGenetico(
    int n,
    int s,
    int t,
    const vector<vector<double>>& c,
    const vector<int>& caminhoInicial
) {
    if (n <= 3) {
        Individuo unico;
        unico.caminho = caminhoInicial;
        unico.custo = calcularCustoCaminho(caminhoInicial, c);
        return unico;
    }

    // Parametros do Algoritmo Genetico
    const int tamPopulacao = 300;
    const int numGeracoes = 2000;
    const int tamTorneio = 3;
    const int numElites = 10;
    const double taxaCruzamento = 0.85;
    const double taxaMutacao = 0.40;

    // Semente fixa para reprodutibilidade dos experimentos
    mt19937 rng(42);
    uniform_real_distribution<double> distProb(0.0, 1.0);

    // 1. Gera a populacao inicial (incluindo a solucao da heuristica inicial)
    vector<Individuo> populacao = gerarPopulacaoInicial(
        tamPopulacao, n, s, t, c, caminhoInicial, rng
    );

    sort(populacao.begin(), populacao.end());
    Individuo melhorGlobal = populacao[0];

    // 2. Loop evolutivo ao longo das geracoes
    for (int ger = 0; ger < numGeracoes; ger++) {
        vector<Individuo> novaPopulacao;
        novaPopulacao.reserve(tamPopulacao);

        // Elitismo: preserva os melhores individuos da geracao atual
        for (int e = 0; e < numElites && e < tamPopulacao; e++) {
            novaPopulacao.push_back(populacao[e]);
        }

        // Preenche o restante da nova populacao com selecao, cruzamento e mutacao
        while ((int)novaPopulacao.size() < tamPopulacao) {
            int idxPai1 = selecaoTorneio(populacao, tamTorneio, rng);
            int idxPai2 = selecaoTorneio(populacao, tamTorneio, rng);

            Individuo filho;
            if (distProb(rng) < taxaCruzamento) {
                filho = cruzamentoOX(populacao[idxPai1], populacao[idxPai2], n, s, t, c, rng);
            } else {
                filho = populacao[idxPai1];
            }

            aplicarMutacao(filho, c, taxaMutacao, rng);
            novaPopulacao.push_back(move(filho));
        }

        populacao = move(novaPopulacao);
        sort(populacao.begin(), populacao.end());

        if (populacao[0].custo < melhorGlobal.custo - 1e-9) {
            melhorGlobal = populacao[0];
        }
    }

    return melhorGlobal;
}


// Estrutura para guardar os dados de cada instancia e montar a tabela comparativa
struct ResultadoInstancia {
    string nome;
    int n;
    double custoInicial;
    double tempoInicialSeg;
    bool validoInicial;
    double custoAG;
    double tempoAGSeg;
    double tempoTotalSeg;
    bool validoAG;
    double custoPLI;
    double gapPLIPct;
    double tempoPLISeg;
    string statusPLI;
    double otimoConhecido;
    string caminhoAGStr;
};


// Dados do Modelo de Programacao Linear Inteira (PLI - limite de 180s) obtidos no TP 01
// (arquivo TP_01/forma_2_python_mip/resultados_instancias.csv)
struct ReferenciaPLI {
    const char* nome;
    double custoPLI;
    double gapPLIPct;
    double tempoPLI;
    const char* statusPLI;
    double otimoConhecido;
};

const ReferenciaPLI tabelaPLI_TP01[] = {
    {"br17.atsp",  34.00,    0.00,   4.6173, "Otimo",           34.00},
    {"ft53.atsp",  6196.00,  0.00, 151.8679, "Otimo",           6196.00},
    {"ft70.atsp",  37655.00, 0.00,  14.0403, "Otimo",           37655.00},
    {"ftv33.atsp", 1275.00,  0.00,   2.6630, "Otimo",           1275.00},
    {"ftv35.atsp", 1443.00,  0.00,   3.4843, "Otimo",           1443.00},
    {"ftv38.atsp", 1511.00,  0.00,   7.9439, "Otimo",           1511.00},
    {"ftv44.atsp", 1614.00,  0.00,   5.3960, "Otimo",           1614.00},
    {"ftv47.atsp", 1754.00,  0.00,  40.6367, "Otimo",           1754.00},
    {"ftv55.atsp", 1613.00,  0.00,  20.0317, "Otimo",           1613.00},
    {"ftv64.atsp", 1840.00,  0.00,  59.1119, "Otimo",           1840.00},
    {"ftv70.atsp", 1958.00,  0.00,  67.8304, "Otimo",           1958.00},
    {"p43.atsp",   610.00,  10.98, 180.1078, "Limite de Tempo", 589.00},
    {"ry48p.atsp", 14070.00, 0.01,  94.7955, "Otimo",           14070.00}
};

void buscarReferenciaPLI(
    const string& nomeArq,
    double* custoPLI,
    double* gapPLIPct,
    double* tempoPLI,
    string* statusPLI,
    double* otimoConhecido
) {
    *custoPLI = 0.0;
    *gapPLIPct = 0.0;
    *tempoPLI = 0.0;
    *statusPLI = "N/A";
    *otimoConhecido = 0.0;

    int total = sizeof(tabelaPLI_TP01) / sizeof(tabelaPLI_TP01[0]);
    for (int i = 0; i < total; i++) {
        if (nomeArq == tabelaPLI_TP01[i].nome) {
            *custoPLI = tabelaPLI_TP01[i].custoPLI;
            *gapPLIPct = tabelaPLI_TP01[i].gapPLIPct;
            *tempoPLI = tabelaPLI_TP01[i].tempoPLI;
            *statusPLI = tabelaPLI_TP01[i].statusPLI;
            *otimoConhecido = tabelaPLI_TP01[i].otimoConhecido;
            return;
        }
    }
}

string extrairNomeBase(const string& caminho) {
    size_t pos1 = caminho.find_last_of('/');
    size_t pos2 = caminho.find_last_of('\\');
    size_t pos = string::npos;
    if (pos1 != string::npos && pos2 != string::npos) pos = max(pos1, pos2);
    else if (pos1 != string::npos) pos = pos1;
    else if (pos2 != string::npos) pos = pos2;

    if (pos == string::npos) return caminho;
    return caminho.substr(pos + 1);
}


int resolverArquivoInstancia(const char* caminhoArquivo, ResultadoInstancia* resOut = NULL) {
    FILE* pFile = fopen(caminhoArquivo, "r");
    if (pFile == NULL) {
        return -1;
    }

    int n = 0;
    vector<vector<double>> c;
    if (readATSPFile(pFile, &n, c) != 0 || n < 2) {
        fclose(pFile);
        printf("Erro ao ler arquivo ou dimensao invalida: %s\n", caminhoArquivo);
        return -1;
    }
    fclose(pFile);

    // Conforme o PDF: origem s = 1 (indice 0) e destino t = n (indice n-1)
    int s = 0;
    int t = n - 1;
    string nomeBase = extrairNomeBase(caminhoArquivo);

    printf("------------------------------------------------------------\n");
    printf("Instancia: %s (n = %d, s = 1, t = %d)\n", nomeBase.c_str(), n, n);
    printf("------------------------------------------------------------\n");

    // 1. Executa a Heuristica Inicial (Atividade 1)
    auto iniHeur = chrono::steady_clock::now();
    double custoInicial = 0.0;
    vector<int> caminhoInicial = heuristicaSolucaoInicial(n, s, t, c, &custoInicial);
    auto fimHeur = chrono::steady_clock::now();
    double tempoInicial = chrono::duration<double>(fimHeur - iniHeur).count();

    vector<pair<int, int>> arcosIniciais = converterCaminhoParaArcos(caminhoInicial);
    double custoVerifIni = 0.0;
    bool validoIni = verificarCaminhoValido(arcosIniciais, n, s + 1, t + 1, c, &custoVerifIni);

    printf("[Atividade 1 - Heuristica Inicial (Vizinho Mais Proximo)]\n");
    printf("Valor Objetivo: %.2f | Tempo: %.6f s\n", custoInicial, tempoInicial);
    printf("Caminho Inicial: ");
    for (size_t i = 0; i < caminhoInicial.size(); i++) {
        if (i > 0) printf(" -> ");
        printf("%d", caminhoInicial[i] + 1);
    }
    printf("\nVerificacao Q1: %s (Custo = %.2f)\n\n", validoIni ? "Caminho Valido" : "Invalido", custoVerifIni);

    // 2. Executa o Algoritmo Genetico Puro (Atividade 2)
    auto iniAG = chrono::steady_clock::now();
    Individuo melhorAG = executarAlgoritmoGenetico(n, s, t, c, caminhoInicial);
    auto fimAG = chrono::steady_clock::now();
    double tempoAG = chrono::duration<double>(fimAG - iniAG).count();
    double tempoTotal = tempoInicial + tempoAG;

    vector<pair<int, int>> arcosAG = converterCaminhoParaArcos(melhorAG.caminho);
    double custoVerifAG = 0.0;
    bool validoAG = verificarCaminhoValido(arcosAG, n, s + 1, t + 1, c, &custoVerifAG);

    printf("[Atividade 2 - Meta-heuristica: Algoritmo Genetico]\n");
    printf("Valor Objetivo: %.2f | Tempo AG: %.4f s | Tempo Total: %.4f s\n",
           melhorAG.custo, tempoAG, tempoTotal);
    printf("Caminho AG: ");
    string caminhoStr = "";
    for (size_t i = 0; i < melhorAG.caminho.size(); i++) {
        if (i > 0) {
            printf(" -> ");
            caminhoStr += " -> ";
        }
        printf("%d", melhorAG.caminho[i] + 1);
        caminhoStr += to_string(melhorAG.caminho[i] + 1);
    }
    printf("\nVerificacao Q1: %s (Custo = %.2f)\n\n", validoAG ? "Caminho Valido" : "Invalido", custoVerifAG);
    fflush(stdout);

    // Checagem de integridade: caminho invalido ou custo divergente interrompe com erro
    if (!validoIni || !validoAG ||
        fabs(custoInicial - custoVerifIni) > 1e-6 ||
        fabs(melhorAG.custo - custoVerifAG) > 1e-6) {
        fprintf(stderr, "Erro de validacao na instancia %s!\n", nomeBase.c_str());
        return -1;
    }

    if (resOut != NULL) {
        resOut->nome = nomeBase;
        resOut->n = n;
        resOut->custoInicial = custoInicial;
        resOut->tempoInicialSeg = tempoInicial;
        resOut->validoInicial = validoIni;
        resOut->custoAG = melhorAG.custo;
        resOut->tempoAGSeg = tempoAG;
        resOut->tempoTotalSeg = tempoTotal;
        resOut->validoAG = validoAG;
        buscarReferenciaPLI(
            nomeBase,
            &resOut->custoPLI,
            &resOut->gapPLIPct,
            &resOut->tempoPLISeg,
            &resOut->statusPLI,
            &resOut->otimoConhecido
        );
        resOut->caminhoAGStr = caminhoStr;
    }

    return 0;
}


int main(int argc, char* argv[]) {
    if (argc >= 2) {
        if (resolverArquivoInstancia(argv[1]) != 0) {
            printf("Nao foi possivel processar o arquivo: %s\n", argv[1]);
            return 1;
        }
        return 0;
    }

    const char* prefixos[] = {
        "instancias/Completos e Assimetricos nos Arcos/",
        "../Implementacao/instancias/Completos e Assimetricos nos Arcos/",
        "../../Implementacao/instancias/Completos e Assimetricos nos Arcos/",
        "Implementacao/instancias/Completos e Assimetricos nos Arcos/",
        "../TP_01/instancias/Completos e Assimetricos nos Arcos/",
        "TP_01/instancias/Completos e Assimetricos nos Arcos/"
    };

    const char* nomesInstancias[] = {
        "br17.atsp", "ft53.atsp", "ft70.atsp", "ftv33.atsp",
        "ftv35.atsp", "ftv38.atsp", "ftv44.atsp", "ftv47.atsp",
        "ftv55.atsp", "ftv64.atsp", "ftv70.atsp", "p43.atsp", "ry48p.atsp"
    };
    const int totalInstancias = 13;

    string pastaEncontrada = "";
    for (int p = 0; p < 6; p++) {
        string teste = string(prefixos[p]) + "br17.atsp";
        FILE* f = fopen(teste.c_str(), "r");
        if (f != NULL) {
            fclose(f);
            pastaEncontrada = prefixos[p];
            break;
        }
    }

    if (pastaEncontrada.empty()) {
        printf("Uso: %s <arquivo_instancia.atsp>\n", argv[0]);
        return 1;
    }

    vector<ResultadoInstancia> resultados;
    for (int i = 0; i < totalInstancias; i++) {
        string caminhoCompleto = pastaEncontrada + nomesInstancias[i];
        ResultadoInstancia res;
        if (resolverArquivoInstancia(caminhoCompleto.c_str(), &res) != 0) {
            fprintf(stderr, "Falha ao processar a instancia obrigatoria: %s\n", caminhoCompleto.c_str());
            return 1;
        }
        resultados.push_back(res);
    }

    if ((int)resultados.size() != totalInstancias) {
        fprintf(stderr, "Erro: nem todas as %d instancias foram processadas.\n", totalInstancias);
        return 1;
    }

    // Imprime tabela comparativa final (Heuristica Inicial vs Algoritmo Genetico vs PLI TP_01)
    printf("========================================================================================================================\n");
    printf("RESUMO COMPARATIVO: HEURISTICA INICIAL vs ALGORITMO GENETICO vs PLI (TP 01 - Limite 180s)\n");
    printf("========================================================================================================================\n");
    printf("%-11s | %3s | %10s | %10s | %10s | %9s | %10s | %9s | %8s | %7s\n",
           "Instancia", "n", "Heur. Ini", "Tempo Ini", "Alg. Gen.", "Tempo AG", "PLI (TP01)", "Tempo PLI", "Gap Otim", "Valido");
    printf("------------------------------------------------------------------------------------------------------------------------\n");

    for (size_t i = 0; i < resultados.size(); i++) {
        const ResultadoInstancia& r = resultados[i];
        double gapOtimo = (r.otimoConhecido > 0) ? 100.0 * (r.custoAG - r.otimoConhecido) / r.otimoConhecido : 0.0;
        printf("%-11s | %3d | %10.2f | %8.6fs | %10.2f | %8.4fs | %10.2f | %8.4fs | %7.2f%% | %7s\n",
               r.nome.c_str(),
               r.n,
               r.custoInicial,
               r.tempoInicialSeg,
               r.custoAG,
               r.tempoAGSeg,
               r.custoPLI,
               r.tempoPLISeg,
               gapOtimo,
               (r.validoInicial && r.validoAG) ? "Sim" : "Nao");
    }
    printf("========================================================================================================================\n");

    // Salva CSV com os resultados para facilitar a analise depois
    FILE* fCsv = fopen("resultados_tp02.csv", "w");
    if (fCsv == NULL) {
        fprintf(stderr, "Erro ao criar arquivo resultados_tp02.csv\n");
        return 1;
    }

    fprintf(fCsv, "instancia,n,custo_heuristica_inicial,tempo_inicial_s,custo_ag,tempo_ag_s,tempo_total_s,custo_pli_tp01,gap_pli_tp01_pct,tempo_pli_tp01_s,status_pli_tp01,otimo_conhecido,melhoria_ag_vs_inicial_pct,desvio_ag_vs_pli_pct,gap_ag_vs_otimo_pct,caminho_valido,caminho_ag\n");
    for (size_t i = 0; i < resultados.size(); i++) {
        const ResultadoInstancia& r = resultados[i];
        double melhoriaIni = (r.custoInicial > 0) ? 100.0 * (r.custoInicial - r.custoAG) / r.custoInicial : 0.0;
        double desvioPLI = (r.custoPLI > 0) ? 100.0 * (r.custoAG - r.custoPLI) / r.custoPLI : 0.0;
        double gapOtimo = (r.otimoConhecido > 0) ? 100.0 * (r.custoAG - r.otimoConhecido) / r.otimoConhecido : 0.0;
        fprintf(fCsv, "%s,%d,%.2f,%.6f,%.2f,%.4f,%.4f,%.2f,%.2f,%.4f,%s,%.2f,%.2f,%.2f,%.2f,%s,%s\n",
                r.nome.c_str(),
                r.n,
                r.custoInicial,
                r.tempoInicialSeg,
                r.custoAG,
                r.tempoAGSeg,
                r.tempoTotalSeg,
                r.custoPLI,
                r.gapPLIPct,
                r.tempoPLISeg,
                r.statusPLI.c_str(),
                r.otimoConhecido,
                melhoriaIni,
                desvioPLI,
                gapOtimo,
                (r.validoInicial && r.validoAG) ? "Sim" : "Nao",
                r.caminhoAGStr.c_str());
    }
    fclose(fCsv);

    return 0;
}
