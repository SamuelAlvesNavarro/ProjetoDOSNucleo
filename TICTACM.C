#include "STDIO.H"
#include "SYSTEM.H"

typedef struct ESCALATOR_NODE{
    PTR_DESC process;
    struct ESCALATOR_NODE *next;
}ESCALATOR_NODE;

typedef struct{
        ESCALATOR_NODE *head;
        ESCALATOR_NODE *tail;
}ESCALATOR_LIST;

void initialize_list(ESCALATOR_LIST *list){
    list->head = NULL;
    list->tail = NULL;
}


ESCALATOR_NODE* create_node(PTR_DESC desc){
    ESCALATOR_NODE *node = (ESCALATOR_NODE*) malloc(sizeof(ESCALATOR_NODE));

    node->next = NULL;
    node->process = desc;

    return node;
}

void create_process(ESCALATOR_LIST *list, PTR_DESC desc){
    ESCALATOR_NODE *temp = create_node(desc);

     if(list->head == NULL){
             list->head = create_node(desc);
             list->tail = list->head;
             list->tail->next = list->head;

             return;
     }

     temp->next = list->head;
     list->tail->next = temp;
     list->tail = temp;
}
void far tic(){

     while(1){
            printf("tic1");
     }

}
void far tac(){

     while(1){
            printf("tac2\n");
     }

}

PTR_DESC descMain;
PTR_DESC descTic;
PTR_DESC descTac;
PTR_DESC descEscalator;
ESCALATOR_LIST process;

void far escalator(){
    ESCALATOR_NODE *curr_node = process.head;
    p_est->num_vetor = 8;
    p_est->p_origem = descEscalator;

    do{
       p_est->p_destino = curr_node->process;
       iotransfer();
       curr_node = curr_node->next;
    }while(1);

}

int main(){
    initialize_list(&process);

    descMain = cria_desc();
    descTic = cria_desc();
    descTac = cria_desc();
    descEscalator = cria_desc();

    newprocess(tic, descTic);
    newprocess(tac, descTac);
    newprocess(escalator, descEscalator);

    create_process(&process, descTic);
    create_process(&process, descTac);

    transfer(descMain, descEscalator);

    return 0;
}