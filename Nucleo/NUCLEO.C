#include "NUCLEO.H"
#include <stdlib.h>
#include <string.h>

PTR_DESC_PROC prim = NULL;
PTR_DESC d_esc, d_main;
unsigned long trocas = 0;
static PTR_DESC_PROC lista = NULL;
static unsigned char far *indos;
static int instalado = 0, iniciou = 0, status = 0;

PTR_DESC_PROC far procura_prox_ativo(void) {
  PTR_DESC_PROC p;
  if (!prim)
    return NULL;

  p = prim->prox_desc;
  do {
    if (p->estado == ativo)
      return p;
    p = p->prox_desc;
  } while (p != prim->prox_desc);
  return NULL;
}

void far volta_DOS(void) {

  disable();
  if (instalado) {
    setvect(8, p_est->int_anterior);
    instalado = 0;
  }
  transfer(d_esc, d_main);
}

void far escalador(void) {
  PTR_DESC_PROC proximo, p;
  p_est->p_origem = d_esc;
  p_est->p_destino = prim->contexto;
  p_est->num_vetor = 8;
  for (;;) {
    instalado = 1;
    iotransfer();
    disable();
    if (!*indos) {
      proximo = procura_prox_ativo();
      if (!proximo) {
        p = lista;
        do {
          if (p->estado != terminado)
            status = 1;
          p = p->prox_desc;
        } while (p != lista);
        volta_DOS();
      }
      if (proximo != prim)
        ++trocas;
      prim = proximo;
      p_est->p_destino = prim->contexto;
    }
    enable();
  }
}

static void far executa(void) {
  prim->entrada();
  termina_processo();
}

PTR_DESC_PROC far cria_processo(void far (*entrada)(), char *nome) {
  PTR_DESC_PROC p, fim;
  if (iniciou || !entrada || !nome)
    return NULL;
  p = (PTR_DESC_PROC)calloc(1, sizeof(DESCRITOR_PROC));
  if (!p)
    return NULL;
  strncpy(p->nome, nome, 34);
  p->estado = ativo;
  p->entrada = entrada;
  p->contexto = cria_desc();
  newprocess(executa, p->contexto);
  if (!lista) {
    lista = p;
    p->prox_desc = p;
  } else {
    fim = lista;
    while (fim->prox_desc != lista)
      fim = fim->prox_desc;
    fim->prox_desc = p;
    p->prox_desc = lista;
  }
  prim = lista;
  return p;
}

int far dispara_sistema(void) {
  union REGS r;
  struct SREGS sr;
  PTR_DESC_PROC p, prox;
  if (iniciou)
    return -1;
  iniciou = 1;
  if (!lista)
    return 0;
  r.h.ah = 0x34;
  intdosx(&r, &r, &sr);
  indos = (unsigned char far *)MK_FP(sr.es, r.x.bx);
  d_esc = cria_desc();
  d_main = cria_desc();
  newprocess(escalador, d_esc);
  transfer(d_main, d_esc);
  enable();
  p = lista;
  do {
    prox = p->prox_desc;
    free(p->contexto);
    free(p);
    p = prox;
  } while (p != lista);
  free(d_esc);
  free(d_main);
  lista = prim = NULL;
  return status;
}

void far termina_processo(void) {
  PTR_DESC_PROC p, proximo;

  disable();
  prim->estado = terminado;
  p = prim->envios;
  while (p) {
    proximo = p->fila_msg;
    p->fila_msg = NULL;
    p->parceiro = NULL;
    p->resultado_msg = MSG_ERRO;
    p->estado = ativo;
    p = proximo;
  }
  prim->envios = prim->ultimo_envio = NULL;
  p = lista;
  do {
    if (p->estado == bloq_recebe && p->parceiro == prim) {
      p->parceiro = NULL;
      p->recepcao = NULL;
      p->remetente = NULL;
      p->resultado_msg = MSG_ERRO;
      p->estado = ativo;
    }
    p = p->prox_desc;
  } while (p != lista);
  enable();
  for (;;)
    ;
}

void far inicia_semaforo(semaforo *sem, int n) {
  sem->s = n < 0 ? 0 : n;
  sem->Q = NULL;
}

void far P(semaforo *sem) {
  PTR_DESC_PROC p, anterior, proximo;

  disable();
  if (sem->s > 0) {
    --sem->s;
    enable();
    return;
  }
  prim->fila_sem = NULL;
  if (!sem->Q) {
    sem->Q = prim;
  } else {
    p = sem->Q;
    while (p->fila_sem)
      p = p->fila_sem;
    p->fila_sem = prim;
  }
  prim->estado = bloq_P;
  anterior = prim;
  proximo = procura_prox_ativo();
  if (!proximo) {
    enable();
    for (;;)
      ;
  }
  prim = proximo;
  ++trocas;
  transfer(anterior->contexto, proximo->contexto);
  enable();
}

void far V(semaforo *sem) {
  PTR_DESC_PROC p;

  disable();
  if (!sem->Q) {
    ++sem->s;
  } else {
    p = sem->Q;
    sem->Q = p->fila_sem;
    p->fila_sem = NULL;
    p->estado = ativo;
  }
  enable();
}

static int processo_valido(PTR_DESC_PROC alvo) {
  PTR_DESC_PROC p;

  if (!alvo || !lista)
    return 0;
  p = lista;
  do {
    if (p == alvo)
      return 1;
    p = p->prox_desc;
  } while (p != lista);
  return 0;
}

static void espera_mensagem(void) {
  PTR_DESC_PROC anterior, proximo;

  anterior = prim;
  proximo = procura_prox_ativo();
  if (!proximo) {
    enable();
    for (;;)
      ;
  }
  prim = proximo;
  ++trocas;
  transfer(anterior->contexto, proximo->contexto);
  enable();
}

int far Envia(PTR_DESC_PROC destino, char *texto) {
  PTR_DESC_PROC eu;
  int tamanho;

  disable();
  if (!instalado || !prim || prim->estado != ativo || !texto ||
      !processo_valido(destino) || destino == prim ||
      destino->estado == terminado) {
    enable();
    return MSG_ERRO;
  }
  for (tamanho = 0; tamanho < TAM_MSG && texto[tamanho]; ++tamanho)
    ;
  if (tamanho == TAM_MSG) {
    enable();
    return MSG_LONGA;
  }
  eu = prim;
  eu->resultado_msg = MSG_OK;
  if (destino->estado == bloq_recebe &&
      (!destino->parceiro || destino->parceiro == eu)) {
    strcpy(destino->recepcao, texto);
    if (destino->remetente)
      *destino->remetente = eu;
    destino->recepcao = NULL;
    destino->remetente = NULL;
    destino->parceiro = NULL;
    destino->resultado_msg = MSG_OK;
    destino->estado = ativo;
  } else {
    strcpy(eu->mensagem, texto);
    eu->parceiro = destino;
    eu->fila_msg = NULL;
    if (destino->ultimo_envio)
      destino->ultimo_envio->fila_msg = eu;
    else
      destino->envios = eu;
    destino->ultimo_envio = eu;
    eu->estado = bloq_envia;
    espera_mensagem();
  }
  enable();
  return eu->resultado_msg;
}

int far Recebe(PTR_DESC_PROC origem, char *texto, PTR_DESC_PROC *remetente) {
  PTR_DESC_PROC eu, p, anterior;

  disable();
  if (!instalado || !prim || prim->estado != ativo || !texto ||
      origem == prim || (origem && (!processo_valido(origem) ||
                                   origem->estado == terminado))) {
    enable();
    return MSG_ERRO;
  }
  if (remetente)
    *remetente = NULL;
  eu = prim;
  anterior = NULL;
  p = eu->envios;
  while (p && origem && p != origem) {
    anterior = p;
    p = p->fila_msg;
  }
  if (p) {
    if (anterior)
      anterior->fila_msg = p->fila_msg;
    else
      eu->envios = p->fila_msg;
    if (eu->ultimo_envio == p)
      eu->ultimo_envio = anterior;
    strcpy(texto, p->mensagem);
    if (remetente)
      *remetente = p;
    p->fila_msg = NULL;
    p->parceiro = NULL;
    p->resultado_msg = MSG_OK;
    p->estado = ativo;
    enable();
    return MSG_OK;
  }
  eu->parceiro = origem;
  eu->recepcao = texto;
  eu->remetente = remetente;
  eu->resultado_msg = MSG_OK;
  eu->estado = bloq_recebe;
  espera_mensagem();
  return eu->resultado_msg;
}
