#include "../NUCLEO.H"
#include <stdio.h>

static volatile unsigned long cpu1, cpu2;
static semaforo sem;
static int ordem[2], usados, concluidos, dentro, falhas;
static long contador;

void far tarefa1(void) {
  while (!cpu2)
    ++cpu1;
  termina_processo();
}

void far tarefa2(void) {
  while (!cpu1)
    ++cpu2;
  ++cpu2;
}

void far espera1(void) {
  P(&sem);
  ordem[usados++] = 1;
}

void far espera2(void) {
  P(&sem);
  ordem[usados++] = 2;
}

void far libera(void) {
  V(&sem);
  V(&sem);
}

void far retorna(void) {
  ++concluidos;
}

void far incrementa(void) {
  int i;
  volatile unsigned j;
  long anterior;

  for (i = 0; i < 100; ++i) {
    P(&sem);
    if (dentro)
      ++falhas;
    ++dentro;
    anterior = contador;
    for (j = 0; j < 10000; ++j)
      ;
    contador = anterior + 1;
    --dentro;
    V(&sem);
  }
  ++concluidos;
}

int main(int argc, char **argv) {
  char modo;
  int rc, erros = 0, segunda = 0;
  FILE *f;
  void interrupt (*antiga)();

  if (argc != 2)
    return 2;
  modo = argv[1][0];
  antiga = getvect(8);
  inicia_semaforo(&sem, modo == 'm' ? 1 : 0);
  switch (modo) {
  case 't':
    if (!cria_processo(tarefa1, "CPU 1") ||
        !cria_processo(tarefa2, "CPU 2"))
      return 2;
    break;
  case 'f':
    if (!cria_processo(espera1, "Espera 1") ||
        !cria_processo(espera2, "Espera 2") ||
        !cria_processo(libera, "Libera"))
      return 2;
    break;
  case 'd':
    if (!cria_processo(espera1, "Bloqueado"))
      return 2;
    break;
  case 'r':
    if (!cria_processo(retorna, "Retorna"))
      return 2;
    break;
  case 'm':
    if (!cria_processo(incrementa, "Soma 1") ||
        !cria_processo(incrementa, "Soma 2"))
      return 2;
    break;
  case 'z':
    break;
  default:
    return 2;
  }
  rc = dispara_sistema();
  if (rc != (modo == 'd' ? 1 : 0) || getvect(8) != antiga)
    ++erros;
  if (modo == 't' && (!cpu1 || !cpu2 || !trocas))
    ++erros;
  if (modo == 'f' && (usados != 2 || ordem[0] != 1 || ordem[1] != 2 ||
                      sem.s != 0 || sem.Q != NULL))
    ++erros;
  if (modo == 'r') {
    segunda = dispara_sistema();
    if (concluidos != 1 || segunda != -1 || getvect(8) != antiga)
      ++erros;
  }
  if (modo == 'm' && (contador != 200 || concluidos != 2 || dentro || falhas ||
                      sem.s != 1 || sem.Q != NULL))
    ++erros;
  f = fopen("TESTES.TXT", "a");
  if (!f)
    return 2;
  fprintf(f, "modo=%c %s erros=%d retorno=%d trocas=%lu\n",
          modo, erros ? "FALHOU" : "OK", erros, rc, trocas);
  fprintf(f, "cpu=%lu,%lu fila=%d,%d usados=%d contador=%ld concluidos=%d segunda=%d\n",
          cpu1, cpu2, ordem[0], ordem[1], usados, contador, concluidos, segunda);
  fclose(f);
  printf("TNUCLEO %c: %s\n", modo, erros ? "FALHOU" : "OK");
  return erros ? 1 : 0;
}
