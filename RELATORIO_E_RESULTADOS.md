# Trabalho Prático 02 — Heurística Inicial e Algoritmo Genético
**Professor Paulo Henrique Macêdo**

---

## 1. Problema e Implementação

O Trabalho Prático 02 retoma o Problema do Caminho Hamiltoniano Mínimo estudado no TP 01. O objetivo é encontrar um caminho direcionado de menor custo que comece no vértice $1$, termine no vértice $n$ e visite todos os vértices exatamente uma vez. Como se trata de um caminho, o custo é a soma dos $n - 1$ arcos percorridos, sem retorno à origem. Os custos são direcionados: ir de $i$ para $j$ pode ter um custo diferente de ir de $j$ para $i$.

A implementação do TP 02 foi feita em C++, no arquivo `AG.cpp`, e avaliada nas 13 instâncias ATSP disponibilizadas para a disciplina. O programa reutiliza a lógica de validação de caminhos do primeiro trabalho. A comparação usa a versão Python do modelo inteiro MTZ do TP 01, resolvida pelo HiGHS por meio do SciPy.

### 1.1. Heurística inicial — vizinho mais próximo

A função `heuristicaSolucaoInicial` começa no vértice $1$. A cada passo, escolhe o intermediário ainda não visitado com o menor custo de saída a partir do vértice atual. O destino fica reservado para o último passo. Sem essa reserva, a regra gulosa poderia escolher o vértice $n$ antes de passar pelos demais. No código, os vértices são indexados a partir de zero, então a origem é representada por $0$ e o destino por $n - 1$.

A escolha é feita sem voltar atrás. Isso torna a construção simples e rápida, com complexidade $O(n^2)$, mas o arco mais barato de um passo não garante o menor custo para o caminho completo.

### 1.2. Algoritmo genético

Cada indivíduo guarda uma rota e seu custo. As pontas do vetor ficam fixas e as posições internas contêm uma permutação dos vértices intermediários. Nas matrizes completas usadas no trabalho, preservar essa permutação e os extremos é suficiente para obter um caminho hamiltoniano viável. O próprio custo da rota serve de avaliação: quanto menor, melhor.

A população tem 300 indivíduos. O primeiro é a solução gulosa. Aproximadamente um terço da população é composto pela solução inicial e por pequenas perturbações dela. Os demais indivíduos são gerados embaralhando os vértices intermediários. Cada pai é escolhido por um torneio de três sorteios com reposição, vencido pelo indivíduo de menor custo.

O cruzamento tem probabilidade de 85%. O operador OX copia para o filho o trecho entre dois cortes do primeiro pai. Para preencher as lacunas, percorre o segundo pai circularmente a partir da posição seguinte ao segundo corte, ignorando os vértices já copiados. A leitura e o preenchimento circulam apenas entre as posições intermediárias. Se não houver cruzamento, o filho começa como uma cópia do primeiro pai.

Cada filho tem 40% de chance de sofrer mutação. Quando a mutação é acionada, o código aplica um ou dois movimentos aleatórios entre troca, inversão, inserção e deslocamento de bloco. O custo é recalculado depois das operações, o que também considera os arcos internos cuja direção muda em uma inversão. Não há refinamento por busca local. Nas 2.000 gerações, os dez melhores indivíduos de cada geração são preservados na seguinte. Como a solução gulosa entra na população inicial e o programa mantém o melhor custo encontrado, a solução devolvida pelo AG não é pior do que a inicial.

---

## 2. Resultados e Como Foram Medidos

O TP 02 usou semente fixa 42 em cada instância. Os tempos abaixo são os da rodada registrada em `resultados_tp02.csv`. As medições da gulosa e do AG foram feitas separadamente com `steady_clock`. O tempo do AG não inclui a geração da rota inicial. Para medir o custo das duas etapas em sequência, o CSV também guarda a soma desses tempos em `tempo_total_s`. Leitura de arquivos, impressão e validação ficam fora dessas janelas. Os tempos do PLI foram importados do TP 01 e correspondem à chamada do solver, com limite configurado de 180 segundos. A comparação descreve as rodadas registradas, sem pressupor um benchmark controlado entre os dois trabalhos.

Na tabela, “Melhoria” é a redução percentual de custo em relação à gulosa. “Gap ótimo” é a diferença entre o custo do AG e o ótimo de referência, dividida pelo ótimo. O gap MIP, registrado separadamente no CSV, é uma medida fornecida pelo solver e não deve ser confundido com nenhuma dessas duas porcentagens.

| Instância | $n$ | Gulosa | t gulosa (s) | AG | t AG (s) | PLI TP 01 | t PLI (s) | Ótimo | Melhoria | Gap ótimo |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| `br17.atsp` | 17 | 56 | 0,000002 | 34 | 0,5465 | 34 | 4,6173 | 34 | 39,29% | 0,00% |
| `ft53.atsp` | 53 | 9.583 | 0,000013 | 6.523 | 1,2601 | 6.196 | 151,8679 | 6.196 | 31,93% | 5,28% |
| `ft70.atsp` | 70 | 42.057 | 0,000016 | 39.095 | 0,9668 | 37.655 | 14,0403 | 37.655 | 7,04% | 3,82% |
| `ftv33.atsp` | 34 | 1.547 | 0,000036 | 1.319 | 0,6507 | 1.275 | 2,6630 | 1.275 | 14,74% | 3,45% |
| `ftv35.atsp` | 36 | 1.750 | 0,000005 | 1.528 | 0,6023 | 1.443 | 3,4843 | 1.443 | 12,69% | 5,89% |
| `ftv38.atsp` | 39 | 1.828 | 0,000022 | 1.585 | 0,8111 | 1.511 | 7,9439 | 1.511 | 13,29% | 4,90% |
| `ftv44.atsp` | 45 | 1.933 | 0,000006 | 1.743 | 0,4099 | 1.614 | 5,3960 | 1.614 | 9,83% | 7,99% |
| `ftv47.atsp` | 48 | 2.298 | 0,000006 | 1.899 | 0,4021 | 1.754 | 40,6367 | 1.754 | 17,36% | 8,27% |
| `ftv55.atsp` | 56 | 2.052 | 0,000009 | 1.764 | 0,3803 | 1.613 | 20,0317 | 1.613 | 14,04% | 9,36% |
| `ftv64.atsp` | 65 | 2.586 | 0,000007 | 2.090 | 0,4067 | 1.840 | 59,1119 | 1.840 | 19,18% | 13,59% |
| `ftv70.atsp` | 71 | 2.637 | 0,000018 | 2.095 | 0,4683 | 1.958 | 67,8304 | 1.958 | 20,55% | 7,00% |
| `p43.atsp` | 43 | 642 | 0,000010 | 596 | 0,4043 | 610* | 180,1078 | 589 | 7,17% | 1,19% |
| `ry48p.atsp` | 48 | 17.971 | 0,000007 | 14.411 | 0,4080 | 14.070 | 94,7955 | 14.070 | 19,81% | 2,42% |

*\* Em `p43`, o TP 01 devolveu a melhor solução viável encontrada naquela rodada (610), com gap MIP de 10,98%, e encerrou por limite de tempo. O cronômetro externo registrou 180,1078 s, embora o limite do solver estivesse configurado em 180 s. O ótimo 589 foi confirmado separadamente, na auditoria, por um modelo inteiro com restrições de grau e cortes de subciclos. O melhor caminho viável e o limite inferior do modelo coincidiram em 589. Esse valor não foi produzido pela rodada MTZ de 180 s.*

Os 26 caminhos obtidos no TP 02, 13 da heurística inicial e 13 do AG, passaram pelo validador. A função confere os $n - 1$ arcos, os graus dos vértices e a travessia completa de $1$ a $n$, que impede que um subciclo isolado passe despercebido. O programa também verifica se a soma dos custos dos arcos bate com o custo armazenado.

---

## 3. Análise Comparativa

O AG melhorou a solução gulosa em todas as instâncias, com redução entre 7,04% e 39,29%. Encontrou o ótimo em `br17`. Nas outras 12 instâncias, terminou entre 1,19% e 13,59% acima do ótimo. A diferença entre os métodos faz sentido: a gulosa aceita a melhor escolha imediata, enquanto o AG pode reorganizar trechos do caminho e encontrar combinações que não aparecem seguindo apenas o menor arco de cada passo. Isso ajuda a explicar a melhora, mas não é uma garantia de otimalidade.

A maior redução ocorreu em `br17`, de 56 para 34. O caso de `ft53` também mostra um ganho expressivo: a gulosa obteve 9.583 e o AG chegou a 6.523, uma redução de 31,93%. O PLI encontrou 6.196, deixando o AG 5,28% acima do ótimo. Em `ft70`, o AG melhorou menos em relação à gulosa (7,04%), mas terminou mais perto do ótimo (3,82%). São critérios diferentes: melhorar muito uma solução inicial não implica terminar mais perto do ótimo.

A comparação entre `ft53` e `ft70` também mostra que o número de vértices, sozinho, não explica o tempo do PLI. A instância de 53 vértices levou 151,8679 s e a de 70 levou 14,0403 s. Os resultados da relaxação do TP 01 dão contexto: ela ficou mais próxima do inteiro em `ft70` do que em `ft53`. Isso é compatível com a diferença observada, mas não prova que a qualidade da relaxação, isoladamente, causou os tempos registrados.

Entre as instâncias `ftv33`, `ftv35` e `ftv38`, as reduções da gulosa ficaram entre 12,69% e 14,74%, com o AG de 3,45% a 5,89% acima do ótimo. Em `ftv44`, `ftv47` e `ftv55`, a redução ainda foi clara, mas a distância ao ótimo aumentou para uma faixa de 7,99% a 9,36%. O maior afastamento foi em `ftv64`: o AG reduziu o custo inicial de 2.586 para 2.090, mas ficou 13,59% acima de 1.840. Já em `ftv70`, a maior instância do conjunto, o afastamento foi de 7,00%. Assim, um maior número de vértices não implicou necessariamente um gap maior para o AG. Em `ry48p`, o AG reduziu o custo de 17.971 para 14.411, ficando 2,42% acima da referência 14.070. O CSV do TP 01 registra status “Otimo” com gap MIP de 0,01%, e a auditoria exata confirmou o custo de referência separadamente.

O caso `p43` exige atenção especial. O AG melhorou a gulosa de 642 para 596 e ficou 2,30% abaixo da solução 610 devolvida pelo PLI ao atingir o limite de tempo. O AG não superou o ótimo: seu custo ficou 1,19% acima de 589. A mesma solução pode, portanto, superar o resultado disponível de uma rodada do solver e ainda ter espaço para melhorar.

Nos tempos registrados, a gulosa foi a mais rápida (2 a 36 microssegundos por instância), seguida do AG (0,3803 a 1,2601 s) e do PLI (2,6630 a 180,1078 s). O AG melhorou a solução inicial com um orçamento fixo de população e gerações, enquanto o PLI buscou também estabelecer a qualidade ou a otimalidade da solução. Uma única rodada com semente fixa é suficiente para reproduzir os resultados nas mesmas condições, mas não mede a variabilidade do AG. Para avaliar estabilidade ou estimar ganho de velocidade, seria necessário repetir as medições em ambiente comum e, no caso do AG, testar também outras sementes.

---

## 4. Conclusão

A heurística inicial produziu rapidamente uma solução viável para as 13 instâncias. O AG melhorou todos os custos iniciais mantendo os caminhos válidos, mas não certifica o ótimo. O PLI complementou a análise ao fornecer a referência do TP 01 e o status da solução encontrada. Em `p43`, separar o resultado do solver sob limite de tempo do ótimo verificado foi indispensável para interpretar a comparação corretamente.

---

## 5. Uso de Inteligência Artificial

O assistente de IA no VS Code e o ChatGPT foram usados como apoio para organizar as tabelas e o CSV, conferir a lógica circular do cruzamento OX e revisar os índices das operações `erase` e `insert` nas mutações. O ChatGPT também auxiliou na revisão técnica e textual deste relatório, na conferência dos resultados entre os dois trabalhos e na verificação independente da referência ótima de `p43`.

---

## Fontes dos Resultados

- **TP 02, commit `e69a676` — `AG.cpp` e `resultados_tp02.csv`:**  
  https://github.com/Guibas698/pesquisa-operacional-tp02/tree/e69a676dd8940b78a4013a06c67e7a2f0a2b90f0
- **TP 01, commit `c928fe0` — CSV da implementação Python/SciPy/HiGHS:**  
  https://github.com/Guibas698/pesquisa-operacional-tp01/blob/c928fe096e365f849fd2c7d817c66fde82533ee2/TP_01/forma_2_python_mip/resultados_instancias.csv
