#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int leitores, escritores, verbose = 0;
int texto = -1;


// Estrutura básica
int leitores_atuando;
sem_t catraca, registrando_leitura, sala_vazia;  // Semaforo binários

// Escritor
void *escritor(void *arg){
  int index_escritor = *((int *) arg);
  sem_wait(&catraca);
  if(verbose){
    printf("Escritor %d trava a catraca\n",index_escritor);
    printf("Escritor %d aguarda a sala ficar vazia\n",index_escritor);
  }
  sem_wait(&sala_vazia);
  texto = index_escritor;
  printf("\033[0;32mEscritor %d alterou o texto\033[0m\n", texto);
  sem_post(&sala_vazia);
  if(verbose)
    printf("Escritor %d sai da sala\n",index_escritor);
  sem_post(&catraca);
  if(verbose)
    printf("Escritor %d libera a catraca\n",index_escritor);
  return NULL;
}

// Leitor
void *leitor(void *arg){
  int index_leitor = *((int *) arg);
  sem_wait(&catraca);
  sem_post(&catraca);
  if(verbose)
    printf("Leitor %d passa pela catraca\n", index_leitor);
  sem_wait(&registrando_leitura);
  leitores_atuando++;
  if(verbose)
    printf("Leitor %d registra sua entrada na sala. Leitores na sala: %d\n", index_leitor,leitores_atuando);
  if(leitores_atuando == 1){
    sem_wait(&sala_vazia);
  }
  sem_post(&registrando_leitura);
  printf("\033[0;34mLeitor %d leu:\033[0m %d\n",index_leitor,texto);
  sem_wait(&registrando_leitura);
  leitores_atuando--;
  if(verbose)
    printf("Leitor %d registra sua saida da sala. Leitores na sala: %d\n", index_leitor,leitores_atuando);
  if(leitores_atuando == 0){
    sem_post(&sala_vazia);
  }
  sem_post(&registrando_leitura);
  return NULL;
}

int main(int argc, char *argv[]){
  if(argc >= 3){
    escritores = atoi(argv[1]);
    leitores = atoi(argv[2]);

    if (argc >= 4){
      if (!strcmp(argv[3], "-v"))
        verbose = 1;
    }
  }else{
    printf("Parametros inválidos, digite o número de escritores e leitores.\n");
    return -1;
  }



  pthread_t t_leitores[leitores];
  pthread_t t_escritores[escritores];
  sem_init(&catraca, 0,1);
  sem_init(&registrando_leitura, 0,1);
  sem_init(&sala_vazia, 0,1);

  int maior = leitores > escritores ? leitores : escritores;

  for(int i = 0; i< maior; i++){
    int *index = malloc(sizeof(*index));
    *index = i;
    if(i < escritores)
      pthread_create(&t_escritores[i], NULL, escritor, (void *) index);
    if(i < leitores)
      pthread_create(&t_leitores[i], NULL, leitor, (void *) index);
  }
  for(int i = 0; i<leitores; i++){
    pthread_join(t_leitores[i], NULL);
  }
  for(int i = 0; i<escritores; i++){
    pthread_join(t_escritores[i], NULL);
  }
  if(!verbose)
    printf("Adicione o paramentro -v para visualizar todas as ações.\n");
  return 0;
}

