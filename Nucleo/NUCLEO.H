#ifndef NUCLEO_H
#define NUCLEO_H

#include "../SYSTEM.H"

typedef struct desc_p {
  char nome[35];
  enum { ativo, bloq_P, terminado } estado;
  PTR_DESC contexto;
  struct desc_p *prox_desc;
  struct desc_p *fila_sem;
  void far (*entrada)();
} DESCRITOR_PROC;

typedef DESCRITOR_PROC *PTR_DESC_PROC;

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

#endif
