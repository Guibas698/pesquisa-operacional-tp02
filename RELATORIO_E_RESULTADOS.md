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
  4. **Cruzamento OX (`cruzamentoOX`):** Com probabilidade de `85%`, cruzo dois pais usando o operador *Order Crossover* (OX) clássico aplicado apenas nos `n - 2` vertices intermediarios. Sorteio dois pontos de corte `corte1` e `corte2` entre `1` e `n - 2`, copio o trecho `pai1[corte1 .. corte2]` direto para o filho e preencho as posicoes intermediarias restantes de forma circular (comecando em `corte2 + 1`), seguindo a ordem em que os vertices que ainda faltam aparecem no `pai2`.
  5. **Mutacao (`aplicarMutacao`) e Elitismo:** Com probabilidade de `40%`, o filho passa por mutacao nos vertices intermediarios, sorteando entre quatro movimentos classicos de permutacao: troca de dois vertices (*Swap*), inversao de um sub-trecho (*Inversao*), retirada de um vertice para inserir em outra posicao (*Insercao*) ou deslocamento de um pequeno bloco de vertices para outra posicao. Ao longo das `2000` geracoes, uso **elitismo** preservando os `10` melhores individuos de uma geracao para a seguinte. No final de cada instancia, passo tanto o caminho inicial quanto o caminho do AG na funcao `verificarCaminhoValido` do TP 01 para confirmar que estao 100% corretos.

## 2. Tabela de Resultados (Heuristica Inicial vs Algoritmo Genetico vs PLI do TP 01)

Na tabela abaixo estao os resultados que obtive nas 13 instancias para `s = 1` e `t = n`, comparando a Heuristica Inicial, o Algoritmo Genetico e o modelo de Programacao Linear Inteira (PLI com limite de 3 minutos = 180s) do Trabalho Pratico 01:

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

## 3. Analise e Comparacao dos Resultados (Atividade 3)

Comparando os valores e os tempos da tabela acima entre os tres metodos (Heuristica Inicial, Algoritmo Genetico e Modelo PLI do TP 01), deu para observar pontos bem claros:

1. **Evolucao do Algoritmo Genetico em relacao a Heuristica Inicial (Qualidade da Solucao):**
   A heuristica inicial do vizinho mais proximo sempre gerou os custos mais altos nas 13 instancias, e o Algoritmo Genetico conseguiu melhorar bastante todas elas. A reducao de custo trazida pelo AG foi de ate **39.29%** na `br17.atsp` (caindo de `56.00` para o otimo `34.00`), **31.93%** na `ft53.atsp` (caindo de `9583.00` para `6523.00`) e em torno de **15% a 20%** na maioria das outras instancias (`ftv47`, `ftv64`, `ftv70` e `ry48p`).
   Isso acontece porque a heuristica do vizinho mais proximo eh "miope": ela olha apenas o arco mais barato imediato a partir do vertice atual sem planejar o resto do caminho. Com isso, quando chega nos ultimos vertices que sobraram (e na hora de ligar o penultimo vertice obrigatoriamente ao destino `t = n`), ela fica sem opcao e acaba sendo obrigada a pegar arcos muito caros. Ja o Algoritmo Genetico avalia o custo do caminho inteiro e usa o cruzamento OX e as mutacoes para reorganizar a ordem de visitacao, eliminando esses saltos caros do final.

2. **Comparacao entre o Algoritmo Genetico e o Modelo PLI do TP 01:**
   - Na instancia menor (`br17.atsp`, com 17 vertices), o Algoritmo Genetico encontrou exatamente a solucao otima (`34.00`, gap de `0.00%`).
   - Em instancias medias e grandes como `ry48p.atsp` (gap de `2.42%`), `ftv33.atsp` (`3.45%`), `ft70.atsp` (`3.82%`), `ftv38.atsp` (`4.90%`) e `ft53.atsp` (`5.28%`), o AG puro chegou bem perto do valor otimo do PLI, mesmo sem usar nenhum solver exato. Nas instancias maiores da familia `ftv` (`ftv44` ate `ftv70`), como o espaco de permutacoes cresce fatorialmente e os custos sao muito proximos entre si, o AG puro ficou com uma diferenca entre `7.00%` e `13.59%` acima do otimo.
   - **O destaque da instancia `p43.atsp`:** esse foi o resultado mais interessante da comparacao com o TP 01. No primeiro trabalho, o modelo inteiro MTZ sofreu muito na `p43.atsp` por causa da grande quantidade de arcos com custo 0 e empates na matriz, estourando o limite de 3 minutos (`180.11s`) com uma solucao de custo **`610.00`**. Agora no TP 02, o Algoritmo Genetico encontrou um caminho valido de custo **`596.00`** em apenas **`0.40s`** (ficando a apenas `1.19%` do otimo real `589.00`). Ou seja, na instancia onde a relaxacao linear do MTZ era fraca e travou o Branch-and-Bound do PLI no TP 01, a meta-heuristica conseguiu achar uma solucao melhor (`596.00` contra `610.00`) quase 450 vezes mais rapido.

3. **Comparacao dos Tempos de Execucao:**
   A diferenca de escala nos tempos de execucao mostra bem o *trade-off* entre heuristica construtiva, meta-heuristica e metodo exato:
   - **Heuristica Inicial:** rodou na casa dos microssegundos (entre `0.000002s` e `0.000036s`), pois faz apenas dois lacos `for` aninhados de complexidade $O(n^2)$. Entrega uma solucao instantanea, porem de qualidade baixa.
   - **Algoritmo Genetico:** rodou todas as instancias entre **`0.38s` e `1.26s`** (menos de 1 segundo na grande maioria). Como o numero de geracoes (`2000`) e o tamanho da populacao (`300`) sao fixos, o tempo cresce de forma linear e previsivel com o numero de vertices $n$.
   - **Modelo PLI (TP 01):** levou de `2.66s` nas instancias mais faceis ate `59.11s` (`ftv64`), `67.83s` (`ftv70`), `94.80s` (`ry48p`), `151.87s` (`ft53`) e `180.11s` (`p43`). Embora garanta a otimalidade quando fecha o gap em `0%`, o tempo cresce de forma exponencial na arvore de Branch-and-Bound.

4. **Validacao na Funcao da Questao 1 do TP 01:**
   Todas as 13 solucoes da Heuristica Inicial e todas as 13 solucoes do Algoritmo Genetico passaram pela funcao `verificarCaminhoValido` e retornaram **"Sim"**, confirmando que todos os caminhos iniciam em `1`, terminam em `n`, visitam os `n` vertices sem subciclos e batem exatamente com os custos reportados.

## 4. Sobre o Uso de IA no Trabalho

Conforme solicitado no item 3 do PDF, utilizei o assistente de IA (no VS Code e ChatGPT) de forma pontual em tres partes especificas:
1. Para tirar duvida na implementacao da caminhada circular do operador de cruzamento *Order Crossover* (`cruzamentoOX`), garantindo que a leitura do segundo pai a partir de `corte2 + 1` nao sobrescrevesse a origem fixa `s = 1` na posicao `0` nem o destino fixo `t = n` na posicao `n - 1`;
2. Para revisar a manipulacao de indices de `std::vector` (`erase` e `insert`) dentro da funcao de mutacao em C++, evitando erros de acesso fora de faixa (*out of bounds*);
3. Para organizar os dados coletados na saida das 13 instancias e montar a tabela comparativa deste relatorio e do arquivo `resultados_tp02.csv` cruzando com os valores que eu tinha obtido no TP 01.
