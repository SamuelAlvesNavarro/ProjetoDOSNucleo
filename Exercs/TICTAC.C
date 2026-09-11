#include "../SYSTEM.H"
#include "conio.h"
#include "stdio.h"

PTR_DESC c1, c2, ini;

void far corotina1() {
  while (1) {
    printf("tic-");
    transfer(c1, c2);
  }
}

void far corotina2() {
  while (1) {
    printf("tac\n");
    transfer(c2, c1);
  }
}

int main() {
  clrscr();

  c1 = cria_desc();
  c2 = cria_desc();
  ini = cria_desc();

  newprocess(corotina1, c1);
  newprocess(corotina2, c2);

  transfer(ini, c1);

  return 0;
}