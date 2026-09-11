#include "NUCLEO.H"
#include <stdio.h>

static volatile unsigned long conta1, conta2;
static semaforo sinal;
static int ordem[2], quantidade;

void far espera1(void) {
  P(&sinal);
  ordem[quantidade++] = 1;
}

void far espera2(void) {
  P(&sinal);
  ordem[quantidade++] = 2;
}

void far libera(void) {
  V(&sinal);
  V(&sinal);
}

/* Sem transferencias voluntarias: o segundo processo so pode iniciar
   quando o timer interromper o primeiro e acionar o escalador. */
void far processo1(void) {
  while (!conta2) {
    ++conta1;
  }
  termina_processo();
}

void far processo2(void) {
  while (!conta1) {
    ++conta2;
  }
  ++conta2;
  /* O retorno tambem encerra o processo, pela funcao executa do nucleo. */
}

int main(int argc, char **argv) {
  int rc, erros = 0;
  char modo = argc > 1 ? argv[1][0] : 'n';
  FILE *f;
  void interrupt (*antiga)();

  antiga = getvect(8);
  if (modo == 's' || modo == 'd') {
    inicia_semaforo(&sinal, 0);
    if (!cria_processo(espera1, "Espera 1"))
      return 2;
    if (modo == 's') {
      if (!cria_processo(espera2, "Espera 2") ||
          !cria_processo(libera, "Libera"))
        return 2;
    }
  } else if (modo != 'z') {
    if (!cria_processo(processo1, "Processo 1") ||
        !cria_processo(processo2, "Processo 2")) {
      printf("Nao foi possivel criar os processos.\n");
      return 2;
    }
  }
  rc = dispara_sistema();
  if (rc != (modo == 'd' ? 1 : 0) || getvect(8) != antiga) {
    ++erros;
  }
  if (modo == 'n' && (!conta1 || !conta2 || !trocas)) {
    ++erros;
  }

  if (modo == 's' && (quantidade != 2 || ordem[0] != 1 || ordem[1] != 2)) {
    ++erros;
  }

  /* Saida apos o termino: nao exige sincronizacao da biblioteca C. */
  printf("NUCLEO: %s\n", erros ? "FALHOU" : "OK");
  printf("Processo 1: %lu; Processo 2: %lu; Trocas: %lu\n",
         conta1, conta2, trocas);
  f = fopen("RESULT.TXT", "a");
  if (!f) {
    return 2;
  }
  fprintf(f, "NUCLEO %c: %s erros=%d status=%d trocas=%lu cpu=%lu,%lu\n",
          modo,
          erros ? "FALHOU" : "OK", erros, rc, trocas, conta1, conta2);
  fclose(f);
  return erros ? 1 : 0;
}
