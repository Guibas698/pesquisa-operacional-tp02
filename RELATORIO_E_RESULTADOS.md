# Relatorio - Trabalho Pratico 02
**UFC - Campus Quixada**  
**QXD0181 - Pesquisa Operacional**  
**Prof. Paulo Henrique Macedo**

## 1. Sobre a Implementacao (Heuristica Inicial e Algoritmo Genetico)

Nesse segundo trabalho o objetivo era implementar a heuristica construtiva da Secao 2 para gerar uma solucao inicial e a meta-heuristica sorteada para a equipe, que no meu caso foi o **Algoritmo Genetico (AG)**, para resolver o Problema do Caminho Hamiltoniano Minimo (PCHM) nas mesmas 13 instancias `.atsp` do Trabalho Pratico 01, mantendo sempre a origem fixa em `s = 1` e o destino fixo em `t = n`.

Diferente do TP 01 (em que precisei lidar com a biblioteca do CPLEX e por isso acabei fazendo duas versoes), nesse TP 02 eu fiz tudo direto em **C++** no arquivo `AG.cpp`, reaproveitando o `readFile.c`, o `readFile.h` e a funcao `verificarCaminhoValido` que eu ja tinha feito no primeiro trabalho. Como o Algoritmo Genetico usa apenas estruturas da biblioteca padrao do C++ (`vector`, `algorithm`, `random` e `chrono`), foi bem mais tranquilo compilar e rodar direto com o `g++` no Windows.

Resumindo como implementei cada parte do codigo:

- **Atividade 1 (Heuristica para a Solucao Inicial - Vizinho Mais Proximo):**
  Fiz a funcao `heuristicaSolucaoInicial` seguindo a regra gulosa da Secao 2 do PDF. O caminho comeca no vertice de origem `s = 1` (indice `0` na matriz) e, a cada passo, procura entre todos os vertices ainda nao visitados aquele que tem o menor custo de arco `c[atual][j]` saindo do vertice atual. Um cuidado importante que tive que tomar aqui eh que o destino obrigatorio `t = n` (indice `n - 1`) precisa ser o ultimo vertice do caminho. Se eu deixasse o `t` livre durante a escolha gulosa, o algoritmo poderia entrar em `n` no meio do caminho e nao conseguiria mais terminar em `n` depois de visitar o resto. Por isso, marco `visitado[t] = true` no inicio para escolher primeiro os `n - 2` vertices intermediarios de forma gulosa e, no ultimo passo, fecho o caminho conectando o ultimo vertice intermediario ao destino `t = n`.

- **Atividade 2 (Meta-heuristica - Algoritmo Genetico):**
  Para implementar o Algoritmo Genetico puro sem correr o risco de gerar caminhos invalidos (com ciclos ou vertices repetidos), estruturei o algoritmo da seguinte forma:
  1. **Representacao do Individuo (Cromossomo):** Criei uma `struct Individuo` contendo um `vector<int> caminho` de tamanho `n` e o seu `custo`. Em todo individuo, a primeira posicao (`caminho[0]`) fica travada na origem `s = 1`, a ultima posicao (`caminho[n - 1]`) fica travada no destino `t = n`, e as posicoes do meio (`1` ate `n - 2`) guardam uma permutacao dos vertices intermediarios `{2, 3, ..., n - 1}`. Assim, qualquer individuo da populacao ja representa automaticamente um caminho hamiltoniano valido de `1` ate `n`, e o fitness eh o proprio custo total dos arcos (quanto menor o custo, melhor o individuo).
  2. **Populacao Inicial (`gerarPopulacaoInicial`):** Usei uma populacao de `300` individuos. Na posicao `0` da populacao eu coloco a propria solucao gerada pela Heuristica Inicial da Atividade 1 (assim o AG ja parte de uma base boa e nunca entrega um resultado pior que a heuristica inicial). No primeiro terco da populacao coloco variacoes da solucao gulosa com pequenas trocas aleatorias de pares de vertices, e nos outros dois tercos gero permutacoes totalmente aleatorias dos vertices intermediarios com `shuffle` para dar diversidade genetica.
  3. **Selecao por Torneio (`selecaoTorneio`):** Para escolher os pais a cada geracao, fiz um torneio de tamanho `3`: sorteio 3 individuos aleatorios da populacao atual e escolho aquele que tem o menor custo.
  4. **Cruzamento OX (`cruzamentoOX`):** Com probabilidade de `85%`, cruzo dois pais usando o operador *Order Crossover* (OX) classico aplicado apenas nos `n - 2` vertices intermediarios. Sorteio dois pontos de corte `corte1` e `corte2` entre `1` e `n - 2`, copio o trecho `pai1[corte1 .. corte2]` direto para o filho e preencho as posicoes intermediarias restantes de forma circular (comecando em `corte2 + 1`), seguindo a ordem em que os vertices que ainda faltam aparecem no `pai2`.
  5. **Mutacao (`aplicarMutacao`) e Elitismo:** Com probabilidade de `40%`, o filho passa por mutacao nos vertices intermediarios, sorteando entre quatro movimentos classicos de permutacao: troca de dois vertices (*Swap*), inversao de um sub-trecho (*Inversao*), retirada de um vertice para inserir em outra posicao (*Insercao*) ou deslocamento de um pequeno bloco de vertices para outra posicao. Ao longo das `2000` geracoes, uso **elitismo** preservando os `10` melhores individuos de uma geracao para a seguinte. No final de cada instancia, passo tanto o caminho inicial quanto o caminho do AG na funcao `verificarCaminhoValido` do TP 01 para confirmar que estao 100% corretos.

## 2. Tabela de Resultados (Heuristica Inicial vs Algoritmo Genetico vs PLI do TP 01)

Na tabela abaixo estao os resultados que obtive nas 13 instancias para `s = 1` e `t = n`, comparando os **valores** e os **tempos de execucao** dos tres metodos: a Heuristica Inicial, o Algoritmo Genetico e o modelo de Programacao Linear Inteira (PLI com limite de 3 minutos = 180s) do Trabalho Pratico 01:

| Instancia | n | Heur. Inicial | Tempo Ini (s) | Alg. Genetico | Tempo AG (s) | PLI (TP 01) | Tempo PLI (s) | Melhoria AG vs Ini | Gap AG vs Otimo | Valido (Q1) |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| `br17.atsp` | 17 | 56.00 | 0.000002 | 34.00 | 0.5465 | 34.00 | 4.6173 | 39.29% | 0.00% | Sim |
| `ft53.atsp` | 53 | 9583.00 | 0.000013 | 6523.00 | 1.2601 | 6196.00 | 151.8679 | 31.93% | 5.28% | Sim |
| `ft70.atsp` | 70 | 42057.00 | 0.000016 | 39095.00 | 0.9668 | 37655.00 | 14.0403 | 7.04% | 3.82% | Sim |
| `ftv33.atsp` | 34 | 1547.00 | 0.000036 | 1319.00 | 0.6507 | 1275.00 | 2.6630 | 14.74% | 3.45% | Sim |
| `ftv35.atsp` | 36 | 1750.00 | 0.000005 | 1528.00 | 0.6023 | 1443.00 | 3.4843 | 12.69% | 5.89% | Sim |
| `ftv38.atsp` | 39 | 1828.00 | 0.000022 | 1585.00 | 0.8111 | 1511.00 | 7.9439 | 13.29% | 4.90% | Sim |
| `ftv44.atsp` | 45 | 1933.00 | 0.000006 | 1743.00 | 0.4099 | 1614.00 | 5.3960 | 9.83% | 7.99% | Sim |
| `ftv47.atsp` | 48 | 2298.00 | 0.000006 | 1899.00 | 0.4021 | 1754.00 | 40.6367 | 17.36% | 8.27% | Sim |
| `ftv55.atsp` | 56 | 2052.00 | 0.000009 | 1764.00 | 0.3803 | 1613.00 | 20.0317 | 14.04% | 9.36% | Sim |
| `ftv64.atsp` | 65 | 2586.00 | 0.000007 | 2090.00 | 0.4067 | 1840.00 | 59.1119 | 19.18% | 13.59% | Sim |
| `ftv70.atsp` | 71 | 2637.00 | 0.000018 | 2095.00 | 0.4683 | 1958.00 | 67.8304 | 20.55% | 7.00% | Sim |
| `p43.atsp` | 43 | 642.00 | 0.000010 | 596.00 | 0.4043 | 610.00* | 180.1078 | 7.17% | 1.19% | Sim |
| `ry48p.atsp` | 48 | 17971.00 | 0.000007 | 14411.00 | 0.4080 | 14070.00 | 94.7955 | 19.81% | 2.42% | Sim |

*\*Observacao sobre a `p43.atsp`: na execucao de 180 segundos do TP 01 (`resultados_instancias.csv`), o modelo PLI com restricoes MTZ atingiu o limite de tempo de 3 minutos entregando a solucao viavel `610.00` (com gap MIP de 10.98%), enquanto o valor otimo inteiro real dessa instancia eh `589.00`. Nas demais 12 instancias, o PLI do TP 01 encontrou o proprio valor otimo.*

## 3. Analise e Comparacao por Extenso dos Tres Metodos (Atividade 3)

### 3.1. Comparacao Geral dos Valores Encontrados (Heuristica Inicial x Algoritmo Genetico x PLI)

Olhando primeiro para a qualidade dos custos encontrados pelos tres algoritmos, da para ver uma escada bem definida em quase todas as instancias: a **Heuristica Inicial** entrega sempre o custo mais alto (a pior solucao dos tres), o **Algoritmo Genetico** consegue dar um salto grande de melhoria ficando bem mais perto do otimo, e o **Modelo Inteiro (PLI)** entrega o valor otimo exato (com excecao da `p43.atsp`, onde o Algoritmo Genetico conseguiu ganhar do PLI de 3 minutos).

O motivo da Heuristica Inicial dar valores tao mais altos eh que a estrategia do vizinho mais proximo eh totalmente "miope": no comeco do caminho ela vai escolhendo os arcos mais baratos que encontra pela frente sem pensar em quem vai sobrar para o final. Quando restam poucos vertices nao visitados (e principalmente no ultimo passo, em que ela eh obrigada a puxar um arco para o destino final `t = n`), nao existe mais alternativa barata disponivel e ela eh obrigada a somar arcos muito caros para fechar a rota.

O Algoritmo Genetico corrige exatamente esse defeito da heuristica inicial. Como a solucao inicial entra na populacao da primeira geracao e vai sofrendo cruzamentos OX e mutacoes ao longo de 2000 geracoes, o AG consegue testar trocas na ordem de visitacao do caminho inteiro. Quando ele muda um vertice do meio do caminho, ele evita aqueles arcos ruins que tinham sobrado no final da escolha gulosa. Por outro lado, como o AG eh uma meta-heuristica (nao varre toda a arvore de combinacoes como o Branch-and-Bound do PLI), em grafos maiores ele estabiliza em solucoes muito boas (geralmente entre 1% e 9% acima do otimo), mas sem a garantia matematica de atingir o otimo global em todas.

### 3.2. Comparacao Geral dos Tempos de Execucao dos Tres Metodos

Quando comparamos os tempos de execucao dos tres metodos, a situacao se inverte completamente em relacao aos custos:

- A **Heuristica Inicial** eh praticamente instantanea em todas as 13 instancias: o tempo ficou entre `0.000002s` (2 microssegundos) e `0.000036s` (36 microssegundos). Isso acontece porque ela nao faz nenhuma busca nem volta atras; sao apenas dois lacos `for` aninhados ($O(n^2)$) que escolhem um vertice por vez e terminam.
- O **Algoritmo Genetico** ficou numa faixa de tempo muito rapida e super estavel, rodando todas as 13 instancias entre **`0.38s` e `1.26s`** (na maioria das instancias levou cerca de `0.4s` a `0.6s`). O ponto mais legal de reparar aqui eh que o tempo do AG quase nao sofre quando o grafo cresce ou quando a matriz tem muitos empates, porque o custo computacional dele depende basicamente do tamanho da populacao (`300`) multiplicado pelo numero de geracoes (`2000`) e pelo tamanho do vetor $n$.
- O **Modelo de Programacao Linear Inteira (PLI do TP 01)** foi o mais lento em 100% das instancias e o que teve o tempo mais imprevisivel. Nas instancias menores ou com relaxacao linear mais forte, ele levou entre `2.66s` e `14.04s`. Mas quando o numero de vertices subiu ou quando a relaxacao linear do modelo MTZ ficou mais fraca, o tempo do PLI disparou para `40.63s` (`ftv47`), `59.11s` (`ftv64`), `67.83s` (`ftv70`), `94.79s` (`ry48p`), `151.86s` (`ft53`) e chegou a estourar os `180.10s` (`p43`). Ou seja, enquanto o AG resolve tudo em menos de 1 segundo e meio, o PLI chega a demorar de 10 ate 450 vezes mais tempo.

### 3.3. Analise Detalhada por Instancia

Para deixar a comparacao completa conforme pedido no enunciado, abaixo comento o comportamento dos tres algoritmos em cada instancia testada:

- **`br17.atsp` ($n = 17$):**
  Eh a menor instancia do conjunto. Nela a Heuristica Inicial teve o seu pior desempenho proporcional: entregou custo `56.00` em `0.000002s`. Isso aconteceu porque a matriz da `br17` tem varios arcos de custo 0 e 3 que enganam a escolha gulosa no comeco e jogam o caminho para arcos caros no final. O Algoritmo Genetico rodou em `0.5465s` e conseguiu derrubar o custo de `56.00` para **`34.00`** (uma reducao de `39.29%`), empatando exatamente com o valor otimo encontrado pelo PLI do TP 01 (`34.00`, gap `0.00%`), sendo que o PLI precisou de `4.6173s` (mais de 8 vezes o tempo do AG).

- **`p43.atsp` ($n = 43$):**
  Essa foi a instancia que deu o resultado mais interessante na comparacao entre os tres metodos. A Heuristica Inicial achou um caminho de custo `642.00` em `0.000010s`. No TP 01, o modelo PLI com restricoes MTZ sofreu demais nessa instancia porque a matriz dela eh cheia de zeros e valores empatados, o que deixa a relaxacao linear muito longe do inteiro (`501.65` na relaxacao). Por conta disso, o PLI rodou durante os 3 minutos inteiros (`180.1078s`) e parou por limite de tempo com a solucao **`610.00`** (ainda com `10.98%` de gap aberto). Ja aqui no TP 02, o Algoritmo Genetico rodou em apenas **`0.4043s`** e encontrou um caminho valido de custo **`596.00`**! Ou seja, o AG nao so melhorou a heuristica inicial (`642` para `596`), como conseguiu entregar uma solucao melhor que a do proprio PLI de 3 minutos (`596.00` contra `610.00`, ficando a apenas `1.19%` do otimo verdadeiro que eh `589.00`), gastando menos de meio segundo.

- **`ft53.atsp` ($n = 53$) e `ft70.atsp` ($n = 70$):**
  - Na **`ft53.atsp`**, a Heuristica Inicial foi muito mal, gerando um caminho de custo `9583.00` (`0.000013s`). O Algoritmo Genetico conseguiu cortar mais de 3 mil pontos desse valor, baixando para **`6523.00`** em `1.2601s` (uma melhoria de `31.93%` sobre a solucao inicial). O PLI do TP 01 encontrou o otimo `6196.00` (o AG ficou `5.28%` acima dele), mas para conseguir fechar essa prova o PLI demorou **`151.8679s`** (quase 2 minutos e meio, cerca de 120 vezes mais demorado que o AG).
  - Na **`ft70.atsp`**, que ja tem 70 vertices, a Heuristica Inicial deu `42057.00` (`0.000016s`), o Algoritmo Genetico baixou para **`39095.00`** em `0.9668s` (melhoria de `7.04%`) e o PLI achou o otimo `37655.00` em `14.0403s` (gap de `3.82%` do AG para o otimo). Aqui da para notar que mesmo a `ft70` tendo mais vertices que a `ft53`, o PLI resolveu a `ft70` muito mais rapido (`14.04s` contra `151.86s`) porque a relaxacao linear da `ft70` eh bem justa, enquanto o Algoritmo Genetico manteve o tempo em torno de 1 segundo nas duas.

- **`ry48p.atsp` ($n = 48$):**
  Nessa instancia a Heuristica Inicial entregou custo `17971.00` em `0.000007s`. O Algoritmo Genetico teve um desempenho excelente: baixou o custo para **`14411.00`** em apenas `0.4080s` (cortando `19.81%` do custo inicial) e ficou muito perto do valor otimo do PLI, que foi `14070.00` (apenas `2.42%` de diferenca). Quando olhamos o tempo, a vantagem do AG fica clara: `0.4080s` no AG contra **`94.7955s`** no PLI (o solver inteiro levou mais de 230 vezes o tempo do AG para ganhar apenas 2.42% de custo).

- **Instancias menores da familia `ftv` (`ftv33.atsp`, `ftv35.atsp` e `ftv38.atsp`):**
  - Na **`ftv33.atsp`** ($n = 34$), a Heuristica Inicial deu `1547.00` (`0.000036s`), o Algoritmo Genetico reduziu para **`1319.00`** em `0.6507s` (melhoria de `14.74%`) e o PLI encontrou o otimo `1275.00` em `2.6630s` (o AG ficou a `3.45%` do otimo).
  - Na **`ftv35.atsp`** ($n = 36$), a Heuristica Inicial deu `1750.00` (`0.000005s`), o AG baixou para **`1528.00`** em `0.6023s` (melhoria de `12.69%`) e o PLI achou `1443.00` em `3.4843s` (diferenca de `5.89%`).
  - Na **`ftv38.atsp`** ($n = 39$), a Heuristica Inicial deu `1828.00` (`0.000022s`), o AG caiu para **`1585.00`** em `0.8111s` (melhoria de `13.29%`) e o PLI achou `1511.00` em `7.9439s` (diferenca de `4.90%`).
  Nessas tres instancias de tamanho entre 34 e 39 vertices, o PLI ainda consegue achar o otimo em poucos segundos (`2.6s` a `7.9s`), mas o AG ja mostra consistencia rodando abaixo de `0.8s` e tirando quase toda a gordura deixada pela heuristica inicial.

- **Instancias medias e grandes da familia `ftv` (`ftv44.atsp`, `ftv47.atsp`, `ftv55.atsp`, `ftv64.atsp` e `ftv70.atsp`):**
  - Na **`ftv44.atsp`** ($n = 45$), a Heuristica Inicial deu `1933.00` (`0.000006s`), o AG baixou para **`1743.00`** em `0.4099s` (melhoria de `9.83%`) e o PLI achou `1614.00` em `5.3960s` (gap de `7.99%`).
  - Na **`ftv47.atsp`** ($n = 48$), a Heuristica Inicial deu `2298.00` (`0.000006s`), o AG reduziu bem para **`1899.00`** em `0.4021s` (melhoria de `17.36%`), e o PLI achou `1754.00`, mas ja precisou de **`40.6367s`** (100 vezes o tempo do AG).
  - Na **`ftv55.atsp`** ($n = 56$), a Heuristica Inicial fez `2052.00` (`0.000009s`), o AG caiu para **`1764.00`** em `0.3803s` (melhoria de `14.04%`) e o PLI achou `1613.00` em `20.0317s` (gap de `9.36%`).
  - Na **`ftv64.atsp`** ($n = 65$), a Heuristica Inicial deu `2586.00` (`0.000007s`), o AG cortou quase 500 pontos chegando em **`2090.00`** em `0.4067s` (melhoria de `19.18%`), enquanto o PLI levou **`59.1119s`** para encontrar `1840.00`.
  - Por fim, na **`ftv70.atsp`** ($n = 71$, a maior instancia de todas), a Heuristica Inicial deu `2637.00` (`0.000018s`), o Algoritmo Genetico derrubou o custo em `20.55%` chegando em **`2095.00`** em apenas `0.4683s` (gap de `7.00%` para o otimo), enquanto o PLI precisou de **`67.8304s`** para fechar em `1958.00`.

### 3.4. Validacao dos Caminhos na Funcao da Questao 1 do TP 01

Todas as 13 solucoes da Heuristica Inicial e todas as 13 solucoes do Algoritmo Genetico passaram pela funcao `verificarCaminhoValido` e deram **"Sim"**, confirmando que todas saem do vertice `1`, terminam no vertice `n`, visitam todos os `n` vertices sem formar subciclos e batem exatamente o mesmo custo calculado.

## 4. Sobre o Uso de IA no Trabalho

Conforme solicitado no item 3 do PDF, utilizei o assistente de IA (no VS Code e ChatGPT) de forma pontual nas seguintes partes:
1. Para fazer e formatar as tabelas de resultados para este relatorio (e organizar o arquivo `resultados_tp02.csv`) a partir da saida do terminal das 13 instancias, juntando com os valores que eu tinha obtido no TP 01 sem precisar digitar numero por numero na mao;
2. Para tirar duvida na implementacao da caminhada circular do operador de cruzamento *Order Crossover* (`cruzamentoOX`), garantindo que a leitura do segundo pai a partir de `corte2 + 1` nao sobrescrevesse a origem fixa `s = 1` na posicao `0` nem o destino fixo `t = n` na posicao `n - 1`;
3. Para revisar a manipulacao de indices de `std::vector` (`erase` e `insert`) dentro da funcao de mutacao em C++, evitando erros de acesso fora de faixa (*out of bounds*).
