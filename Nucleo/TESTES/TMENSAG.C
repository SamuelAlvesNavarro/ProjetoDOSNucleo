#include "../NUCLEO.H"
#include <stdio.h>
#include <string.h>

static PTR_DESC_PROC a, b, c, destino;
static char modo;
static int erros, enviados, recebidos, etapa;
static int ordem[3], chegada[3], n_chegadas;

void far emite(void) {
  etapa = 1;
  if (Envia(destino, "ola") != MSG_OK || etapa != 2)
    ++erros;
  ++enviados;
}

void far recebe_depois(void) {
  char msg[TAM_MSG];
  PTR_DESC_PROC de;

  if (a->estado != bloq_envia || enviados != 0)
    ++erros;
  etapa = 2;
  if (Recebe(QUALQUER, msg, &de) != MSG_OK || de != a || strcmp(msg, "ola"))
    ++erros;
  ++recebidos;
}

void far recebe_antes(void) {
  char msg[TAM_MSG];
  PTR_DESC_PROC de;

  etapa = 1;
  if (Recebe(a, msg, &de) != MSG_OK || etapa != 2 ||
      de != a || strcmp(msg, "ola"))
    ++erros;
  ++recebidos;
}

void far emite_depois(void) {
  if (destino->estado != bloq_recebe || recebidos != 0)
    ++erros;
  etapa = 2;
  if (Envia(destino, "ola") != MSG_OK)
    ++erros;
  ++enviados;
}

static void envia_id(int id) {
  char msg[2];

  msg[0] = '0' + id;
  msg[1] = 0;
  disable();
  chegada[n_chegadas++] = id;
  if (Envia(destino, msg) != MSG_OK)
    ++erros;
  ++enviados;
}

void far envia1(void) { envia_id(1); }
void far envia2(void) { envia_id(2); }
void far envia3(void) { envia_id(3); }

void far fila(void) {
  char msg[TAM_MSG];
  PTR_DESC_PROC de, origem;
  int i, esperado;

  while (a->estado != bloq_envia || b->estado != bloq_envia ||
         c->estado != bloq_envia)
    ;
  for (i = 0; i < 3; ++i) {
    origem = QUALQUER;
    esperado = chegada[i];
    if (modo == 's') {
      origem = i == 0 ? c : (i == 1 ? b : a);
      esperado = i == 0 ? 3 : (i == 1 ? 2 : 1);
    }
    if (Recebe(origem, msg, &de) != MSG_OK)
      ++erros;
    ordem[i] = msg[0] - '0';
    if (ordem[i] != esperado ||
        de != (esperado == 1 ? a : (esperado == 2 ? b : c)))
      ++erros;
    ++recebidos;
  }
  if (prim->envios != NULL || prim->ultimo_envio != NULL)
    ++erros;
}

void far espera_envio(void) {
  if (Envia(destino, "sem receptor") != MSG_ERRO)
    ++erros;
  ++enviados;
}

void far espera_recebimento(void) {
  char msg[TAM_MSG];
  PTR_DESC_PROC de;

  if (Recebe(destino, msg, &de) != MSG_ERRO || de != NULL)
    ++erros;
  ++recebidos;
}

void far encerra(void) {
  while (a->estado != bloq_envia || b->estado != bloq_recebe)
    ;
}

void far bloqueado(void) {
  char msg[TAM_MSG];

  Recebe(QUALQUER, msg, NULL);
  ++erros;
}

void far limites_envia(void) {
  char longa[TAM_MSG + 1], maxima[TAM_MSG];
  int i;

  for (i = 0; i < TAM_MSG; ++i)
    longa[i] = 'x';
  longa[TAM_MSG] = 0;
  for (i = 0; i < TAM_MSG - 1; ++i)
    maxima[i] = 'a';
  maxima[TAM_MSG - 1] = 0;
  if (Envia(destino, longa) != MSG_LONGA ||
      Envia(NULL, "x") != MSG_ERRO ||
      Envia(prim, "x") != MSG_ERRO ||
      Envia((PTR_DESC_PROC)1, "x") != MSG_ERRO ||
      Envia(destino, NULL) != MSG_ERRO ||
      Recebe(prim, maxima, NULL) != MSG_ERRO ||
      Recebe((PTR_DESC_PROC)1, maxima, NULL) != MSG_ERRO ||
      Recebe(QUALQUER, NULL, NULL) != MSG_ERRO)
    ++erros;
  if (Envia(destino, "") != MSG_OK || Envia(destino, maxima) != MSG_OK)
    ++erros;
  enviados = 2;
  while (destino->estado != terminado)
    ;
  if (Envia(destino, "x") != MSG_ERRO ||
      Recebe(destino, maxima, NULL) != MSG_ERRO)
    ++erros;
}

void far limites_recebe(void) {
  char msg[TAM_MSG];
  int i;

  if (Recebe(a, msg, NULL) != MSG_OK || msg[0] != 0)
    ++erros;
  ++recebidos;
  if (Recebe(a, msg, NULL) != MSG_OK || strlen(msg) != TAM_MSG - 1)
    ++erros;
  for (i = 0; i < TAM_MSG - 1; ++i)
    if (msg[i] != 'a')
      ++erros;
  ++recebidos;
}

void far ping(void) {
  char msg[TAM_MSG];
  int i;

  for (i = 0; i < 200; ++i) {
    if (Envia(destino, "ping") != MSG_OK ||
        Recebe(destino, msg, NULL) != MSG_OK || strcmp(msg, "pong"))
      ++erros;
    ++enviados;
  }
}

void far pong(void) {
  char msg[TAM_MSG];
  int i;

  for (i = 0; i < 200; ++i) {
    if (Recebe(a, msg, NULL) != MSG_OK || strcmp(msg, "ping") ||
        Envia(a, "pong") != MSG_OK)
      ++erros;
    ++recebidos;
  }
}

int main(int argc, char **argv) {
  int rc, esperado;
  FILE *f;
  void interrupt (*antiga)();

  if (argc != 2)
    return 2;
  modo = argv[1][0];
  esperado = 1;
  antiga = getvect(8);
  switch (modo) {
  case 'e':
    a = cria_processo(emite, "Emissor");
    destino = cria_processo(recebe_depois, "Receptor");
    break;
  case 'r':
    destino = cria_processo(recebe_antes, "Receptor");
    a = cria_processo(emite_depois, "Emissor");
    break;
  case 'f':
  case 's':
    a = cria_processo(envia1, "Emissor 1");
    b = cria_processo(envia2, "Emissor 2");
    c = cria_processo(envia3, "Emissor 3");
    destino = cria_processo(fila, "Receptor");
    if (!b || !c)
      return 2;
    esperado = 3;
    break;
  case 'x':
    a = cria_processo(espera_envio, "Espera envio");
    b = cria_processo(espera_recebimento, "Espera recepcao");
    destino = cria_processo(encerra, "Termina");
    if (!b)
      return 2;
    break;
  case 'd':
    a = destino = cria_processo(bloqueado, "Sem remetente");
    esperado = 0;
    break;
  case 'l':
    a = cria_processo(limites_envia, "Limites emissor");
    destino = cria_processo(limites_recebe, "Limites receptor");
    esperado = 2;
    break;
  case 'p':
    a = cria_processo(ping, "Ping");
    destino = cria_processo(pong, "Pong");
    esperado = 200;
    break;
  default:
    return 2;
  }
  if (!a || !destino)
    return 2;
  rc = dispara_sistema();
  if (rc != (modo == 'd' ? 1 : 0) || getvect(8) != antiga ||
      enviados != esperado || recebidos != esperado)
    ++erros;
  f = fopen("MENSAG.TXT", "a");
  if (!f)
    return 2;
  fprintf(f, "modo=%c %s erros=%d retorno=%d enviados=%d recebidos=%d trocas=%lu\n",
          modo, erros ? "FALHOU" : "OK", erros, rc, enviados, recebidos, trocas);
  fclose(f);
  printf("TMENSAG %c: %s\n", modo, erros ? "FALHOU" : "OK");
  return erros ? 1 : 0;
}
