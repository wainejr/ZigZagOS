// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.4 -- Janeiro de 2022
// Teste da implementação de fila genérica queue.c/queue.h.

// ESTE ARQUIVO NÃO DEVE SER MODIFICADO - ELE SERÁ SOBRESCRITO NOS TESTES

// operating system check
#if defined(_WIN32) || (!defined(__unix__) && !defined(__unix) && (!defined(__APPLE__) || !defined(__MACH__)))
#warning Este código foi planejado para ambientes UNIX (LInux, *BSD, MacOS). A compilação e execução em outros ambientes é responsabilidade do usuário.
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "queue.h"

#define N 100

// A estrutura "filaint_t" será usada com as funções de queue.c usando um
// casting para o tipo "queue_t". Isso funciona bem, se os campos iniciais
// de ambas as estruturas forem os mesmos. De acordo com a seção 6.7.2.1 do
// padrão C99: "Within a structure object, the non-bit-ﬁeld members and the
// units in which bit-ﬁelds reside have addresses that increase in the order
// in which they are declared.".

typedef struct filaint_t
{
   struct filaint_t *prev ;  // ptr para usar cast com queue_t
   struct filaint_t *next ;  // ptr para usar cast com queue_t
   int id ;
   // outros campos podem ser acrescidos aqui
} filaint_t ;

filaint_t item[N];
filaint_t *fila0, *fila1, *aux, *final ;
int ret ;

//------------------------------------------------------------------------------

// imprime na tela um elemento da fila (chamada pela função queue_print)
void print_elem (void *ptr)
{
   filaint_t *elem = ptr;

   if (!elem)
      return ;

   elem->prev ? printf ("%d", elem->prev->id) : printf ("*") ;
   printf ("<%d>", elem->id) ;
   elem->next ? printf ("%d", elem->next->id) : printf ("*") ;
   fflush(stdout);
}

//------------------------------------------------------------------------------

// retorna 1 se a estrutura da fila está correta, 0 senão
int fila_correta (filaint_t *fila)
{
   filaint_t *aux ;

   // uma fila vazia sempre está correta
   if (!fila)
      return 1 ;

   // fila com um só elemento e correta
   if ((fila->next == fila) && (fila->prev == fila))
      return 1 ;

   // fila com um só elemento, mas incorreta
   if ((fila->next == fila) || (fila->prev == fila))
   {
      printf ("ERRO: ponteiros errados na fila com um elemento\n") ;
      return 0 ;
   }

   // fila com mais elementos, percorrer e testar todos os ponteiros
   aux = fila ;
   do
   {
      // testa ponteiro next (avaliação em curto-circuito)
      if (aux->next && (aux->next->prev == aux))
         ; // ponteiro ok
      else
      {
         printf ("ERRO: ponteiros errados ->next ou ->next->prev\n") ;
         return 0 ;
      }

      // testa ponteiro prev (avaliação em curto-circuito)
      if (aux->prev && (aux->prev->next == aux))
         ; // ponteiro ok
      else
      {
         printf ("ERRO: ponteiros errados ->prev ou ->prev->next\n") ;
         return 0 ;
      }
      aux = aux->next ;
   }
   while (aux != fila) ;

   // passou por tudo, estrutura da fila parece estar ok
   return 1 ;
}

//------------------------------------------------------------------------------

int main (int argc, char **argv, char **envp)
{
   int i ;

   // inicializa os N elementos
   for (i=0; i<N; i++)
   {
      item[i].id = i ;
      item[i].prev = NULL ;
      item[i].next = NULL ;
   }

   // PARTE 1: queue_append e queue_size =======================================

   // Teste: inserir N elemementos na fila e verificar a estrutura
   printf ("Testando insercao de %d elementos...\n", N) ;
   fila0 = NULL ;
   for (i=0; i<N; i++)
   {
      assert (queue_size ((queue_t*) fila0) == i) ;
      queue_append ((queue_t **) &fila0, (queue_t*) &item[i]) ;
      assert (fila_correta (fila0)) ;
   }

   // Teste: contar o numero de elementos na fila e verificar a ordem
   // dos elementos inseridos
   printf ("Testando tamanho da fila e ordem dos %d elementos...\n", N) ;
   aux = fila0 ;
   i=0 ;
   do
   {
      assert (i == aux->id) ; // testa posição do elemento i
      i++ ;
      aux = aux->next ;
   } while (aux != fila0) ;

   assert (i == N) ;

   assert (queue_size ((queue_t*) fila0) == N) ;

   printf ("Testes de insercao funcionaram!\n") ;

   // PARTE 2: queue_remove ====================================================

   // esvazia fila0, retirando sempre o primeiro elemento
   printf ("Remocao %d vezes o primeiro elemento...\n", N) ;
   i=0 ;
   while (i<N)
   {
      aux = fila0 ;
      queue_remove ((queue_t**) &fila0, (queue_t*) aux) ;
      assert (fila_correta (fila0)) ;  // estrutura continua correta
      assert (aux->id == i) ;            // testa ordem do elemento removido
      assert (aux->prev == NULL) ;       // testa elemento removido
      assert (aux->next == NULL) ;       // testa elemento removido
      i++ ;
   }
   assert (fila0 == NULL) ;             // fila deve estar vazia
   printf ("Ok, apos %d remocoes a fila ficou vazia\n", N) ;

   // reconstroi fila de teste
   fila0 = NULL ;
   for (i=0; i<N; i++)
      queue_append ((queue_t**) &fila0, (queue_t*) &item[i]) ;

   // esvazia fila0, retirando sempre o segundo elemento
   printf ("Remocao %d vezes o segundo elemento...\n", N) ;
   i=0 ;
   while (i<N)
   {
      aux = fila0->next ;
      queue_remove ((queue_t**) &fila0, (queue_t*) aux) ;
      assert (fila_correta (fila0)) ;  // estrutura continua correta
      assert (aux->id == ((i+1)%N)) ;    // testa ordem do elemento removido
      assert (aux->prev == NULL) ;       // testa elemento removido
      assert (aux->next == NULL) ;       // testa elemento removido
      i++ ;
   }
   assert (fila0 == NULL) ;             // fila deve estar vazia
   printf ("Ok, apos %d remocoes a fila ficou vazia\n", N) ;

   // reconstroi fila de teste
   fila0 = NULL ;
   for (i=0; i<N; i++)
      queue_append ((queue_t**) &fila0, (queue_t*) &item[i]) ;

   // esvazia fila0, retirando sempre o último elemento
   printf ("Remocao %d vezes o último elemento...\n", N) ;
   i=0 ;
   while (i<N)
   {
      aux = fila0->prev ;
      queue_remove ((queue_t**) &fila0, (queue_t*) aux) ;
      assert (fila_correta (fila0)) ;  // estrutura continua correta
      assert (aux->id+i == N-1) ;        // testa ordem do elemento removido
      assert (aux->prev == NULL) ;       // testa elemento removido
      assert (aux->next == NULL) ;       // testa elemento removido
      i++ ;
   }
   assert (fila0 == NULL) ;             // fila deve estar vazia
   printf ("Ok, apos %d remocoes a fila ficou vazia\n", N) ;

   // reconstroi fila de teste
   fila0 = NULL ;
   for (i=0; i<N; i++)
      queue_append ((queue_t**) &fila0, (queue_t*) &item[i]) ;

   // remocoes aleatorias
   printf ("Remocao %d vezes um elemento aleatório...\n", N) ;
   while (fila0)
   {
      i = rand() % queue_size ((queue_t*) fila0) ;
      aux = fila0 ;
      while (i)
      {
         i-- ;
         aux = aux->next ;
      }
      queue_remove ((queue_t**) &fila0, (queue_t*) aux) ;
   }
   assert (fila0 == NULL) ;             // fila deve estar vazia
   printf ("Ok, apos %d remocoes aleatorias a fila ficou vazia\n", N) ;

   printf ("Testes de remocao funcionaram!\n") ;

   // PARTE 3: operações inválidas =============================================

   // inicializa os N elementos
   for (i=0; i<N; i++)
   {
      item[i].id = i ;
      item[i].prev = NULL ;
      item[i].next = NULL ;
   }

   // preparar filas de teste
   fila0 = NULL ;
   fila1 = NULL ;
   queue_append ((queue_t**) &fila0, (queue_t*) &item[0]) ;
   queue_append ((queue_t**) &fila1, (queue_t*) &item[1]) ;

   // tentar remover elemento que está em outra fila
   printf ("Testando remocao de elemento que está em outra fila...\n") ;
   queue_remove ((queue_t**) &fila0, (queue_t*) &item[1]) ;
   assert (fila0 == &item[0]) ;
   assert (item[0].prev == &item[0]) ;
   assert (item[0].next == &item[0]) ;
   assert (item[1].prev == &item[1]) ;
   assert (item[1].next == &item[1]) ;
   printf ("Ok, nao deixou remover um elemento de outra fila\n") ;

   // tentar remover elemento que não está em nenhuma fila
   printf ("Testando remocao de elemento que não está em nenhuma fila...\n") ;
   queue_remove ((queue_t**) &fila0, (queue_t*) &item[2]) ;
   assert (fila0 == &item[0]) ;
   assert (item[0].prev == &item[0]) ;
   assert (item[0].next == &item[0]) ;
   assert (item[2].prev == NULL) ;
   assert (item[2].next == NULL) ;
   printf ("Ok, nao deixou remover um elemento que não está em nenhuma fila\n") ;

   // tentar inserir algo que já está na mesma fila
   printf ("Testando insercao de elemento que já está na fila...\n") ;
   queue_append ((queue_t**) &fila0, (queue_t*) &item[0]) ;
   assert (fila0 == &item[0]) ;
   assert (item[0].prev == &item[0]) ;
   assert (item[0].next == &item[0]) ;
   printf ("Ok, não deixou inserir elemento que já estava na fila\n") ;

   // tentar inserir algo que está em outra fila
   printf ("Testando insercao de elemento que está em outra fila...\n") ;
   queue_append ((queue_t**) &fila0, (queue_t*) &item[1]) ;
   assert (fila0 == &item[0]) ;
   assert (item[0].prev == &item[0]) ;
   assert (item[0].next == &item[0]) ;
   assert (fila1 == &item[1]) ;
   assert (item[1].prev == &item[1]) ;
   assert (item[1].next == &item[1]) ;
   printf ("Ok, não deixou inserir elemento que está em outra fila\n") ;

   // criar uma grande fila com entradas dinamicas
   fila0 = NULL ;
   for (i=0; i< N*N; i++)
   {
      aux = (filaint_t*) malloc (sizeof (filaint_t)) ;
      aux->id = i ;
      aux->prev = aux->next = NULL ;
      queue_append ((queue_t**) &fila0, (queue_t*) aux) ;
      assert (fila_correta (fila0)) ;
   }
   printf ("Ok, criei uma fila com %d elementos ordenados\n", N*N) ;

   // retirar e destruir cada elemento da fila, em sequencia
   for (i=0; i< N*N; i++)
   {
      aux = fila0 ;
      queue_remove ((queue_t**) &fila0, (queue_t*) fila0) ;
      assert (fila_correta (fila0)) ;
      assert (aux->id == i) ;
      free (aux) ;
   }
   printf ("Ok, retirei e destrui em ordem %d elementos da fila\n", N*N) ;

   printf ("Testes de operações inválidas funcionaram!\n") ;

   // PARTE 4: queue_print =====================================================

   printf ("Teste do queue_print...\n");

   // inicializa os N elementos
   for (i=0; i<N; i++)
   {
      item[i].id = i ;
      item[i].prev = NULL ;
      item[i].next = NULL ;
   }

   // uma fila vazia
   fila0 = NULL ;

   // imprimir a fila
   printf ("Saida esperada: []\n") ;
   queue_print ("Saida gerada  ", (queue_t*) fila0, print_elem) ;

   // uma fila com 10 elementos
   for (i=0; i<10;i++)
      queue_append ((queue_t**) &fila0, (queue_t*) &item[i]) ;

   // imprimir a fila
   printf ("Saida esperada: [9<0>1 0<1>2 1<2>3 2<3>4 3<4>5 4<5>6 5<6>7 6<7>8 7<8>9 8<9>0]\n") ;
   queue_print ("Saida gerada  ", (queue_t*) fila0, print_elem) ;

   printf ("Testes concluidos!!!\n") ;

   exit(0) ;
}
