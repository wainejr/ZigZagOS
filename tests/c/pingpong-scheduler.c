// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// Teste do escalonador por prioridades dinâmicas

#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"

task_t Pang, Peng, Ping, Pong, Pung ;

// corpo das threads
void Body (void * arg)
{
  int i ;

  printf ("%s: inicio (prioridade %d)\n", (char *) arg, task_getprio(NULL)) ;

  for (i=0; i<10; i++)
  {
    printf ("%s: %d\n", (char *) arg, i) ;
    task_yield ();
  }
  printf ("%s: fim\n", (char *) arg) ;
  task_exit (0) ;
}

int main (int argc, char *argv[])
{
  printf ("main: inicio\n");

  ppos_init () ;

  task_init (&Pang, Body, "    Pang") ;
  task_setprio (&Pang, 0);

  task_init (&Peng, Body, "        Peng") ;
  task_setprio (&Peng, 2);

  task_init (&Ping, Body, "            Ping") ;
  task_setprio (&Ping, 4);

  task_init (&Pong, Body, "                Pong") ;
  task_setprio (&Pong, 6);

  task_init (&Pung, Body, "                    Pung") ;
  task_setprio (&Pung, 8);

  printf ("main: fim\n");
  task_exit (0) ;
}