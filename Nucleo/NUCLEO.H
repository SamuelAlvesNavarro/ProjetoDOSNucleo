#ifndef NUCLEO_H
#define NUCLEO_H

#include "../SYSTEM.H"

#define TAM_MSG 80
#define MSG_OK 0
#define MSG_ERRO -1
#define MSG_LONGA -2

typedef struct desc_p {
  char nome[35];
  enum { ativo, bloq_P, bloq_envia, bloq_recebe, terminado } estado;
  PTR_DESC contexto;
  struct desc_p *prox_desc;
  struct desc_p *fila_sem;
  void far (*entrada)();
  struct desc_p *parceiro, *fila_msg;
  struct desc_p *envios, *ultimo_envio;
  char mensagem[TAM_MSG];
  char *recepcao;
  struct desc_p **remetente;
  int resultado_msg;
} DESCRITOR_PROC;

typedef DESCRITOR_PROC *PTR_DESC_PROC;
#define QUALQUER ((PTR_DESC_PROC)0)

typedef struct {
  int s;
  PTR_DESC_PROC Q;
} semaforo;

extern PTR_DESC_PROC prim;
extern unsigned long trocas;

PTR_DESC_PROC far cria_processo(void far (*entrada)(), char *nome);
int far dispara_sistema(void);
void far escalador(void);
void far termina_processo(void);
void far volta_DOS(void);
PTR_DESC_PROC far procura_prox_ativo(void);

void far inicia_semaforo(semaforo *sem, int n);
void far P(semaforo *sem);
void far V(semaforo *sem);

int far Envia(PTR_DESC_PROC destino, char *texto);
int far Recebe(PTR_DESC_PROC origem, char *texto, PTR_DESC_PROC *remetente);

#endif
