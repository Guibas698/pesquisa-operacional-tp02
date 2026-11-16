# Trabalho Prático 02 - Pesquisa Operacional (QXD0181)

Implementação da **Heurística Inicial (Vizinho Mais Próximo)** e da meta-heurística **Algoritmo Genético (AG)** em C++ para o Problema do Caminho Hamiltoniano Mínimo (PCHM) com vértices de origem ($s = 1$) e destino ($t = n$).

## Estrutura do Repositório

- `AG.cpp`: Implementação em C++ contendo:
  - **Validador (`verificarCaminhoValido`):** Função do TP 01 que verifica se a sequência de arcos forma um caminho hamiltoniano válido de $s = 1$ até $t = n$ (graus de entrada/saída, conectividade, ausência de ciclos e soma exata dos custos).
  - **Atividade 1 (`heuristicaSolucaoInicial`):** Heurística gulosa do vizinho mais próximo partindo de $s = 1$, escolhendo a cada passo o vértice intermediário ainda não visitado de menor custo e finalizando obrigatoriamente em $t = n$.
  - **Atividade 2 (`executarAlgoritmoGenetico`):** Meta-heurística Algoritmo Genético puro com representação por permutação dos vértices intermediários, população inicial (solução gulosa + perturbações + permutações aleatórias), seleção por torneio, cruzamento de ordem clássico (*Order Crossover* - OX circular), mutação (*Swap*, *Inversão* e *Inserção*) e elitismo.
- `readFile.h` e `readFile.c`: Leitura das matrizes de adjacência/custos das instâncias TSPLIB (`.atsp`).
- `instancias/Completos e Assimetricos nos Arcos/`: As 13 instâncias `.atsp` disponibilizadas na disciplina.
- `resultados_tp02.csv`: Tabela gerada automaticamente comparando Heurística Inicial, Algoritmo Genético e Modelo Inteiro PLI (TP 01).
- `TP 02.pdf`: Enunciado do Trabalho Prático 02.
- `Makefile` e `executar_todas.sh`: Scripts de compilação e execução.

## Como Compilar e Executar

### Compilação com `g++`:
```bash
g++ -O2 -Wall AG.cpp -o AG
```

### Executar em todas as 13 instâncias (e gerar `resultados_tp02.csv`):
```bash
./AG
```

### Executar em uma instância específica:
```bash
./AG "instancias/Completos e Assimetricos nos Arcos/br17.atsp"
```
