# Produtor/consumidor conforme o PDF

A aplicação principal em `PRINCIP.C` segue os slides 90–93. `NUCLEO.C/H` fornecem processos, escalonamento pelo timer e semáforos, conforme os slides 60–89.

| Requisito | Implementação |
| --- | --- |
| Buffer circular com N posições | `buffer[N]`, `entrada` e `saida` avançam módulo N; N=100, como no exemplo do slide 93 |
| Produtor não excede a capacidade | `P(vazios)` antes da inserção; contador inicial N |
| Consumidor aguarda itens produzidos | `P(cheios)` antes da retirada; contador inicial 0 |
| Ordem de retirada igual à de inserção | Índices circulares FIFO; `consome` confere a sequência recebida |
| Exclusão mútua no buffer | `P(mutex)` e `V(mutex)` em torno de `insere` e `retira`; contador inicial 1 |
| Produção e consumo fora da região crítica | `produz` antes de adquirir os semáforos; `consome` depois de liberá-los |
| Processos em ciclo contínuo | Produtor e consumidor executam `while (1)` usando os serviços do núcleo |

O exemplo final do slide 93 usa contadores abstratos com down/up. Aqui são usados os semáforos com fila e as primitivas P/V implementadas nos slides 84–89. Os itens são números sequenciais representando mensagens.

## Executar

No Linux: `./rodar.sh` nesta pasta. No DOSBox, a partir desta pasta: `RODAR` ou `NUCLEO`. Pressione Esc para encerrar. A saída durante a execução informa que o ciclo está ativo; os contadores são impressos após o encerramento, evitando chamadas concorrentes de printf.

Para verificação finita: `./rodar.sh v` no Linux. São produzidos e consumidos 1000 itens, suficientes para percorrer o buffer de 100 posições dez vezes. `RESULT.TXT` registra contagens, ocupação máxima, falhas e resultado. A opção `p` continua aceita como alternativa para iniciar produtor/consumidor.

O encerramento por Esc, a marca de fim e a opção finita são recursos auxiliares desta implementação, não exigências do PDF. O produtor enfileira uma marca após seu último item; o consumidor drena os itens anteriores antes de terminar. A marca ocupa uma posição normal, usa os mesmos semáforos e não entra nas contagens de itens. O núcleo então restaura a INT 8 e retorna ao DOS.

## Compilar

Monte `/home/samuel/dos` como C no DOSBox, execute `cd tc\nucleo` e `..\tc`. No Turbo C, selecione `NUCLEO.PRJ` em Project > Project name. Configure Includes, Libraries e Turbo C directory como `..`, e Output directory como `.`; use o modelo Small. Pressione F9. O projeto usa `..\SYSTEM.OBJ` e gera `NUCLEO.EXE` nesta pasta.
