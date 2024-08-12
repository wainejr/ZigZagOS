// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// Teste da preempção por tempo - teste de stress com muitas threads

#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"

#define WORKLOAD 5000
#define NUMTASKS 500

task_t task[NUMTASKS] ;

// simula um processamento pesado
int hardwork (int n)
{
   int i, j;
   unsigned long long soma;

   soma = 0 ;
   for (i=0; i<n; i++)
      for (j=0; j<n; j++)
         soma += j ;
   return (soma) ;
}

// corpo das threads
void Body (void * arg)
{
   int i ;

   for (i=0; i<3; i++)
   {
      printf ("task %3d: i=%d\n", task_id (), i) ;
      hardwork (WORKLOAD) ;
   }
   printf ("task %3d: fim\n", task_id ()) ;
   task_exit (0) ;
}

int main (int argc, char *argv[])
{
   int i ;
   printf ("main: inicio\n");

   ppos_init () ;

   for (i=0; i<NUMTASKS; i++)
   {
      printf ("main: iniciando tarefa %d\n", i+2) ;
      task_init (&task[i], Body, NULL) ;
   }

   printf ("main: fim\n");
   task_exit (0);
}