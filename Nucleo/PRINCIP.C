#include "NUCLEO.H"
#include <conio.h>
#include <stdio.h>
#include <string.h>

#define N 100
#define TOTAL 1000L
#define FIM 0UL

static semaforo vazios, cheios, mutex;
static unsigned long buffer[N];
static int entrada, saida, ocupacao, maximo, falhas;
static int verificacao;
static unsigned long produzidos, consumidos;
static unsigned long proxima = 1, esperada = 1;

unsigned long produz(void) {
  unsigned long item;

  item = proxima++;
  if (proxima == FIM)
    proxima = 1;
  return item;
}

void insere(unsigned long item) {
  buffer[entrada] = item;
  entrada = (entrada + 1) % N;
  ++ocupacao;
  if (ocupacao > maximo)
    maximo = ocupacao;
  if (ocupacao > N)
    ++falhas;
}

unsigned long retira(void) {
  unsigned long item;

  item = buffer[saida];
  saida = (saida + 1) % N;
  --ocupacao;
  if (ocupacao < 0)
    ++falhas;
  return item;
}

void consome(unsigned long item) {
  if (item != esperada)
    ++falhas;
  ++esperada;
  if (esperada == FIM)
    esperada = 1;
  ++consumidos;
}

void far produtor(void) {
  unsigned long item;

  while (1) {
    if (verificacao) {
      if (produzidos == TOTAL)
        break;
    } else if (kbhit() && getch() == 27) {
      break;
    }
    item = produz();
    P(&vazios);
    P(&mutex);
    insere(item);
    ++produzidos;
    V(&mutex);
    V(&cheios);
  }

  P(&vazios);
  P(&mutex);
  insere(FIM);
  V(&mutex);
  V(&cheios);
  termina_processo();
}

void far consumidor(void) {
  unsigned long item;

  while (1) {
    P(&cheios);
    P(&mutex);
    item = retira();
    V(&mutex);
    V(&vazios);
    if (item == FIM)
      break;
    consome(item);
  }
  termina_processo();
}

int main(int argc, char **argv) {
  int rc, erros;
  FILE *f;
  void interrupt (*antiga)();

  if (argc > 2 || (argc == 2 && strcmp(argv[1], "v") &&
                  strcmp(argv[1], "p"))) {
    printf("Uso: NUCLEO [v]\n");
    return 2;
  }
  verificacao = argc == 2 && !strcmp(argv[1], "v");
  printf("Produtor/consumidor - buffer circular de %d posicoes\n", N);
  if (verificacao)
    printf("Verificacao de %ld itens.\n", TOTAL);
  else
    printf("Execucao continua. Pressione Esc para encerrar.\n");

  inicia_semaforo(&mutex, 1);
  inicia_semaforo(&vazios, N);
  inicia_semaforo(&cheios, 0);
  if (!cria_processo(produtor, "Produtor") ||
      !cria_processo(consumidor, "Consumidor")) {
    printf("Nao foi possivel criar os processos.\n");
    return 2;
  }
  antiga = getvect(8);
  rc = dispara_sistema();
  erros = rc != 0 || getvect(8) != antiga || falhas ||
          produzidos != consumidos || ocupacao != 0 ||
          vazios.s != N || cheios.s != 0 || mutex.s != 1;
  if (verificacao && produzidos != TOTAL)
    erros = 1;

  /* Somente a main imprime: nao ha printf concorrente entre processos. */
  printf("NUCLEO: %s\n", erros ? "FALHOU" : "OK");
  printf("Produzidos=%lu consumidos=%lu maximo=%d final=%d falhas=%d\n",
         produzidos, consumidos, maximo, ocupacao, falhas);
  f = fopen("RESULT.TXT", "a");
  if (!f)
    return 2;
  fprintf(f, "Produtor/consumidor %s: %s N=%d status=%d\n",
          verificacao ? "finito" : "continuo", erros ? "FALHOU" : "OK", N, rc);
  fprintf(f, "Produzidos=%lu consumidos=%lu maximo=%d final=%d falhas=%d\n",
          produzidos, consumidos, maximo, ocupacao, falhas);
  fclose(f);
  return erros ? 1 : 0;
}
