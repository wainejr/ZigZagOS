// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// Teste do task_sleep()

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ppos.h"

task_t Pang, Peng, Ping, Pong, Pung ;

// corpo das threads
void Body (void * arg)
{
   int i, timeSleep, timeBefore, timeAfter ;
   char *status ;

   printf ("%5d ms: %s: inicio\n", systime(), (char *) arg) ;
   for (i=0; i<20; i++)
   {
      // sorteia tempo entre 0 e 2000 ms (2s), em saltos de 100 ms
      timeSleep = 100 * (random() % 21) ;

      // informa o quanto vai dormir
      printf ("%5d ms: %s vai dormir %d ms\n",
              systime(), (char *) arg, timeSleep) ;

      // registra tempo antes e depois de dormir
      timeBefore = systime () ;
      task_sleep (timeSleep) ;
      timeAfter  = systime () ;

      // verifica se dormiu o intervalo especificado
      status = (timeAfter - timeBefore) == timeSleep ? "ok" : "ERROR" ;

      // informa o quanto efetivamente dormiu
      printf ("%5d ms: %s dormiu     %d ms (%s)\n", systime(),
              (char *) arg, timeAfter - timeBefore, status) ;
   }
   printf ("%5d ms: %s: fim\n", systime(), (char *) arg) ;
   task_exit (0) ;
}

int main (int argc, char *argv[])
{
   // inicializacao do SO
   ppos_init () ;

   printf ("%5d ms: main: inicio\n", systime()) ;

   // lanca as tarefas
   task_init (&Pang, Body, "    Pang") ;
   task_init (&Peng, Body, "        Peng") ;
   task_init (&Ping, Body, "            Ping") ;
   task_init (&Pong, Body, "                Pong") ;
   task_init (&Pung, Body, "                    Pung") ;

   // aguarda tarefas concluirem
   printf ("%5d ms: main: espera Pang...\n", systime()) ;
   task_wait (&Pang) ;
   printf ("%5d ms: main: Pang acabou\n", systime()) ;

   printf ("%5d ms: main: espera Peng...\n", systime()) ;
   task_wait (&Peng) ;
   printf ("%5d ms: main: Peng acabou\n", systime()) ;

   printf ("%5d ms: main: espera Ping...\n", systime()) ;
   task_wait (&Ping) ;
   printf ("%5d ms: main: Ping acabou\n", systime()) ;

   printf ("%5d ms: main: espera Pong...\n", systime()) ;
   task_wait (&Pong) ;
   printf ("%5d ms: main: Pong acabou\n", systime()) ;

   printf ("%5d ms: main: espera Pung...\n", systime()) ;
   task_wait (&Pung) ;
   printf ("%5d ms: main: Pung acabou\n", systime()) ;

   // main encerra
   printf ("%5d ms: main: fim\n", systime()) ;
   task_exit (0) ;
}