//Otávio da Cruz Mello
//Trabalho II da Disciplina de Sistemas Operacionais
//Threads

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<time.h>
#include<pthread.h>

// estrutura para passarmos argumentos para threads
struct args{
  int *chunk;
  int tamanho,idThread;
};

// retorno das threads
struct retornos{
  int a,d,p;
};
// realiza funcao para sequencial
void func_f(int *vet,int worksize){
   int d=0,a=0,p=0,i=0,j=0,soma;
   for(i=0;i<worksize;i++){ // percorre todo o vetor de valores até chegar no workzise
      soma=0;
      for(j=1;j<vet[i];j++){ // percorre todos os valores menores que o valor de i atual
         if(vet[i]%j==0) // caso esse valor seja divisivel pelo i, soma ele na variavel de soma de divisores
            soma=soma+j;
      }
      if(soma<vet[i])
         d++; // Defectivo
      else if(soma==vet[i])
         p++; // Perfeito
         else
            a++; // Abundante
   }
   printf("Total   : [A] %d [D] %d [P] %d [WTot] %d ",a,d,p,worksize);
}

// realiza funcao para threads
void funcs_fchunks(struct args * arg){
   int i,j,a,b,c,soma;
   struct retornos * retorno = malloc(sizeof(struct retornos));
   struct timespec inicio,fim;
   double tempo;
   if(retorno==NULL){
      printf("Sem Memoria.\n");
      exit(1);
   }
   // inicializa variáveis de defectivo, perfeito e abundante para cada thread
   retorno->p=0;
   retorno->a=0;
   retorno->d=0;
   clock_gettime(CLOCK_MONOTONIC,&inicio);
   for(i=0;i < (arg->tamanho);i++){ // percorre todo o vetor de valores até chegar no tamanho maximo do vetor definido na struct
      soma=0;
      for(j=1;j < (arg->chunk[i]);j++){ // percorre todos os valores menores que o valor de i atual
         if(arg->chunk[i]%j == 0) // caso esse valor seja divisivel pelo i, soma ele na variavel de soma de divisores
            soma = soma + j;
      }
      if(soma < arg->chunk[i])
         retorno->d++; // Defectivo
      else if(soma == arg->chunk[i])
         retorno->p++; // Perfeito
         else
            retorno->a++; // Abundante
   }
   clock_gettime(CLOCK_MONOTONIC,&fim);
   tempo = (fim.tv_sec-inicio.tv_sec);
   tempo += (fim.tv_nsec-inicio.tv_nsec)/1000000000.0;
   printf("Thread %d: [A] %d [D] %d [P] %d [WTot] %d [Tempo] %fs\n",arg->idThread,retorno->a,retorno->d,retorno->p,arg->tamanho,tempo);
   pthread_exit(retorno); // retorna struct com valores de abundante, perfeito e defectivo quando executar join
}

// preenche argumentos e cria threads por chunks
void cria_threads_chunks(int *vet,int nthreads,int worksize){
   // tamanho chunk = tamanho de cada chunk, sobrou = caso tenha sobra na divisao sera distribuido entre as threads
   int n, m, tamanho_chunk = worksize/nthreads, sobrou = worksize%nthreads;
   int tamanho_chunks_sobrou=0,somaa=0,somad=0,somap=0;
   int * vet_chunk = malloc(tamanho_chunk*sizeof(int));
   if(vet_chunk==NULL){
      printf("Sem Memoria.\n");
      exit(1);
   }
   struct args * arg;
   struct retornos * retornado;
   pthread_t thread[nthreads];
   // cria n threads 
   for(n=0;n<nthreads;n++){
      tamanho_chunks_sobrou = 0;
      arg = malloc(sizeof(arg));
      if(arg==NULL){
         printf("Sem memoria.\n");
         exit(1);
      }
      // preenche vetor de cada thread igualmente
      for(m=0;m<tamanho_chunk;m++){
         vet_chunk[m]=vet[(n*tamanho_chunk)+m];
      }
      // se tiver sobra na divisao
      if(sobrou>0){
         // realoca vetor adicionando um espaco da memoria a mais (Esse sera o maximo que ele recebera a mais)
         vet_chunk = realloc(vet_chunk,(tamanho_chunk+1)*sizeof(int));
         if(&vet_chunk[tamanho_chunk+1]==NULL){
            printf("Falha na realocação.\n");
            exit(1);
         }
	 vet_chunk[m] = vet[worksize-sobrou];
         tamanho_chunks_sobrou = tamanho_chunk+1; // variavel auxiliar para sabermos o tamanho do vetor atual
         sobrou--; // diminuimos um dos que sobraram, pois ja foi colocado em um dos chunks
      }else{
         // raeloca vetor para tamanho normal se acabou a sobra, garantindo que este esteja em seu tamanho certo e nao com espaco adicional
         vet_chunk = realloc(vet_chunk,tamanho_chunk*sizeof(int));
         if(&vet_chunk[tamanho_chunk]==NULL){
            printf("Falha na realocação.\n");
            exit(1);
         }
         tamanho_chunks_sobrou = tamanho_chunk;
      }
      // aloca vetor da estrutura de argumentos e o preenche
      arg->chunk = malloc(tamanho_chunks_sobrou*sizeof(int));
      if(arg->chunk == NULL){
         printf("Sem Memoria.\n");
         exit(1);
      }
      for(m=0;m<tamanho_chunks_sobrou;m++){
         arg->chunk[m] = vet_chunk[m];
      }
      // preenche o restante dos argumentos
      arg->tamanho = tamanho_chunks_sobrou;
      arg->idThread = n;
      if(pthread_create(&thread[n],NULL,(void*) funcs_fchunks,arg)){
         printf("Erro ao criar thread.\n");
         exit(1);
      }
   }
   for(n=0;n<nthreads;n++){
      if(pthread_join(thread[n],(void**)&retornado)){
         printf("Erro ao executar join.\n");
         exit(1);
      }
      // realiza as somas dos resultados das threads
      somaa=somaa+retornado->a;
      somad=somad+retornado->d;
      somap=somap+retornado->p;
   }
   printf("Total   : [A] %d [D] %d [P] %d [WTot] %d",somaa,somad,somap,worksize);
   free(vet_chunk);
   free(arg);
   free(retornado);
}

// preenche argumentos e cria threads esparsa
void cria_threads_esparsa(int *vet,int nthreads,int worksize){
  int i,j,tamanho_vet=0,somaa=0,somap=0,somad=0;
  int *vet_esparsa=malloc(sizeof(int));
  if(vet_esparsa==NULL){
     printf("Sem Memoria.\n");
     exit(1);
  }
  struct args * arg;
  struct retornos * retornado;
  pthread_t thread[nthreads];
  // cria n threads
  for(i=0;i<nthreads;i++){
     tamanho_vet=0;
     arg=malloc(sizeof(arg));
     // percorre todos os valores ate o worksize, somando nthreads onde j=idThread para adquirirmos o proximo valor do vetor da esparsa
     for(j=i;j<worksize;j=j+nthreads){
       tamanho_vet++; // vai somando um para sabermos o tamanho de cada vetor de cada thread
       // realoca thread de acordo com esse valor
       vet_esparsa=realloc(vet_esparsa,tamanho_vet*sizeof(int));
       if(&vet_esparsa[tamanho_vet]==NULL){
          printf("Falha na realocacao.\n");
          exit(1);
       }
       vet_esparsa[tamanho_vet-1]=vet[j];
     }
     // aloca vetor da estrutura de argumentos e o preenche
     arg->chunk=malloc(tamanho_vet*sizeof(int));
     for(j=0;j<tamanho_vet;j++){
        arg->chunk[j]=vet_esparsa[j];
     }
     // preenche o resto dos argumentos
     arg->tamanho=tamanho_vet;
     arg->idThread=i;
     if(pthread_create(&thread[i],NULL,(void*) funcs_fchunks,arg)){
        printf("Erro ao criar thread.\n");
        exit(1);
     }
  }
  for(i=0;i<nthreads;i++){
     if(pthread_join(thread[i],(void**)&retornado)){
         printf("Erro ao executar join.\n");
         exit(1);
      }
      // soma o resultado das threads
      somaa=somaa+retornado->a;
      somad=somad+retornado->d;
      somap=somap+retornado->p;
  }
  printf("Total   : [A] %d [D] %d [P] %d [WTot] %d",somaa,somad,somap,worksize);
  free(retornado);
  free(vet_esparsa);
  free(arg);
}

int main(int argc, char *argv[]){
   int *vet,i;
   struct timespec inicio,fim;
   double tempo;
   // condicoes de inicio do programa
   if(argc!=3){
      printf("O programa exige a insercao de dois argumentos.\n");
      exit(1);
   }
   if(atoi(argv[1]) <= 0 || atoi(argv[2]) <=0){
      printf("Os argumentos devem ser positivos e maiores que zero.\n");
      exit(1);
   }
   if(atoi(argv[1]) > atoi(argv[2])){
      printf("Worksize nao pode ser menor que o numero de threads.\n");
      exit(1);
   }
   printf("Quantidade de threads a serem criadas: %d.\n",atoi(argv[1]));
   printf("Worksize total: %d.\n\n",atoi(argv[2]));
   vet = malloc (atoi(argv[2])*sizeof(int));
   if(vet==NULL){
      printf("Sem Memoria.");
      exit(1);
   }
   for(i=0;i<atoi(argv[2]);i++){
      vet[i]=i;
   }
   //sequencial
   printf("# Sequencial\n");
   clock_gettime(CLOCK_MONOTONIC,&inicio);
   func_f(vet,atoi(argv[2]));
   clock_gettime(CLOCK_MONOTONIC,&fim);
   tempo = (fim.tv_sec-inicio.tv_sec);
   tempo += (fim.tv_nsec-inicio.tv_nsec)/1000000000.0;
   printf("[Tempo] %fs\n",tempo);
   printf("\n");
   //threads chunks
   printf("# Chunks\n");
   clock_gettime(CLOCK_MONOTONIC,&inicio);
   cria_threads_chunks(vet,atoi(argv[1]),atoi(argv[2]));
   clock_gettime(CLOCK_MONOTONIC,&fim);
   tempo = (fim.tv_sec-inicio.tv_sec);
   tempo += (fim.tv_nsec-inicio.tv_nsec)/1000000000.0;
   printf(" [Tempo] %fs\n",tempo);
   printf("\n");
   //threads esparsa
   printf("# Esparsa\n");
   clock_gettime(CLOCK_MONOTONIC,&inicio);
   cria_threads_esparsa(vet,atoi(argv[1]),atoi(argv[2]));
   clock_gettime(CLOCK_MONOTONIC,&fim);
   tempo = (fim.tv_sec-inicio.tv_sec);
   tempo += (fim.tv_nsec-inicio.tv_nsec)/1000000000.0;
   printf(" [Tempo] %fs\n",tempo);
   printf("\n");
   free(vet);
}

