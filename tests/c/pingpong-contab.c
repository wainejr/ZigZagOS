// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// Teste da contabilização - tarefas de mesma prioridade

#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"

#define WORKLOAD 40000

task_t Pang, Peng, Ping, Pong, Pung ;

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
   printf ("%s: inicio em %4d ms\n", (char *) arg, systime()) ;
   fflush(stdout);
   hardwork (WORKLOAD) ;
   printf ("%s: fim    em %4d ms\n", (char *) arg, systime()) ;
   fflush(stdout);
   task_exit (0) ;
}

int main (int argc, char *argv[])
{
   printf ("main: inicio\n");

   ppos_init () ;

   task_init (&Pang, Body, "    Pang") ;
   task_init (&Peng, Body, "        Peng") ;
   task_init (&Ping, Body, "            Ping") ;
   task_init (&Pong, Body, "                Pong") ;
   task_init (&Pung, Body, "                    Pung") ;

   printf ("main: fim\n");
   task_exit (0);
}