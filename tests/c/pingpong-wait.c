// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// Teste do operador task_wait

#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"

#define WORKLOAD 20000

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
   int i, max ;

   max = task_id() * 2 ;

   printf ("%s: inicio\n", (char *) arg) ;
   for (i=0; i<max; i++)
   {
      printf ("%s: %d\n", (char *) arg, i) ;
      hardwork (WORKLOAD) ;
   }
   printf ("%s: fim\n", (char *) arg) ;
   task_exit (task_id()) ;
}

int main (int argc, char *argv[])
{
   int i, ec ;

   ppos_init () ;

   printf ("main: inicio\n");

   task_init (&Pang, Body, "    Pang") ;
   task_init (&Peng, Body, "        Peng") ;
   task_init (&Ping, Body, "            Ping") ;
   task_init (&Pong, Body, "                Pong") ;
   task_init (&Pung, Body, "                    Pung") ;

   for (i=0; i<2; i++)
   {
      printf ("main: %d\n", i) ;
      fflush(stdout);
      hardwork (WORKLOAD) ;
   }

   printf ("main: esperando Pang...\n") ;
   ec = task_wait (&Pang) ;
   printf ("main: Pang acabou com exit code %d\n", ec) ;

   printf ("main: esperando Peng...\n") ;
   ec = task_wait (&Peng) ;
   printf ("main: Peng acabou com exit code %d\n", ec) ;

   printf ("main: esperando Ping...\n") ;
   ec = task_wait (&Ping) ;
   printf ("main: Ping acabou com exit code %d\n", ec) ;

   printf ("main: esperando Pong...\n") ;
   ec = task_wait (&Pong) ;
   printf ("main: Pong acabou com exit code %d\n", ec) ;

   printf ("main: esperando Pung...\n") ;
   ec = task_wait (&Pung) ;
   printf ("main: Pung acabou com exit code %d\n", ec) ;

   printf ("main: fim\n");

   task_exit (0) ;
}