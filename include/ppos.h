// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.6 -- Maio de 2023

// ESTE ARQUIVO NÃO DEVE SER MODIFICADO - ELE SERÁ SOBRESCRITO NOS TESTES

// Interface do núcleo para as aplicações

#ifndef __PPOS__
#define __PPOS__

// estruturas de dados necessárias neste projeto
#include "ppos_data.h"

// macros importantes ==========================================================

// habilita compatibilidade POSIX no MacOS X (para ucontext.h)
#define _XOPEN_SOURCE 600

// este código deve ser compilado em sistemas UNIX-like
#if defined(_WIN32) || (!defined(__unix__) && !defined(__unix) && (!defined(__APPLE__) || !defined(__MACH__)))
#warning Este codigo foi planejado para ambientes UNIX (LInux, *BSD, MacOS). A compilacao e execucao em outros ambientes e responsabilidade do usuario.
#endif

// otimizações podem atrapalhar código que manipula contexto
#ifdef __OPTIMIZE__
#error "Please do not use optimization (-O1, -O2, ...)"
#endif

// funções gerais ==============================================================

// Inicializa o sistema operacional; deve ser chamada no inicio do main()
void ppos_init () ;

// gerência de tarefas =========================================================

// Inicializa uma nova tarefa. Retorna um ID> 0 ou erro.
int task_init (task_t *task,			// descritor da nova tarefa
               void  (*start_func)(void *),	// funcao corpo da tarefa
               void   *arg) ;			// argumentos para a tarefa

// retorna o identificador da tarefa corrente (main deve ser 0)
int task_id () ;

// Termina a tarefa corrente com um status de encerramento
void task_exit (int exit_code) ;

// alterna a execução para a tarefa indicada
int task_switch (task_t *task) ;

// suspende a tarefa atual,
// transferindo-a da fila de prontas para a fila "queue"
void task_suspend (task_t **queue) ;

// acorda a tarefa indicada,
// trasferindo-a da fila "queue" para a fila de prontas
void task_awake (task_t *task, task_t **queue) ;

// operações de escalonamento ==================================================

// a tarefa atual libera o processador para outra tarefa
void task_yield () ;

// define a prioridade estática de uma tarefa (ou a tarefa atual)
void task_setprio (task_t *task, int prio) ;

// retorna a prioridade estática de uma tarefa (ou a tarefa atual)
int task_getprio (task_t *task) ;

// operações de gestão do tempo ================================================

// retorna o relógio atual (em milisegundos)
unsigned int systime () ;

// suspende a tarefa corrente por t milissegundos
void task_sleep (int t) ;

// operações de sincronização ==================================================

// a tarefa corrente aguarda o encerramento de outra task
int task_wait (task_t *task) ;

// inicializa um semáforo com valor inicial "value"
int sem_init (semaphore_t *s, int value) ;

// requisita o semáforo
int sem_down (semaphore_t *s) ;

// libera o semáforo
int sem_up (semaphore_t *s) ;

// "destroi" o semáforo, liberando as tarefas bloqueadas
int sem_destroy (semaphore_t *s) ;

// inicializa um mutex (sempre inicialmente livre)
int mutex_init (mutex_t *m) ;

// requisita o mutex
int mutex_lock (mutex_t *m) ;

// libera o mutex
int mutex_unlock (mutex_t *m) ;

// "destroi" o mutex, liberando as tarefas bloqueadas
int mutex_destroy (mutex_t *m) ;

// inicializa uma barreira para N tarefas
int barrier_init (barrier_t *b, int N) ;

// espera na barreira
int barrier_wait (barrier_t *b) ;

// destrói a barreira, liberando as tarefas
int barrier_destroy (barrier_t *b) ;

// operações de comunicação ====================================================

// inicializa uma fila para até max mensagens de size bytes cada
int mqueue_init (mqueue_t *queue, int max, int size) ;

// envia uma mensagem para a fila
int mqueue_send (mqueue_t *queue, void *msg) ;

// recebe uma mensagem da fila
int mqueue_recv (mqueue_t *queue, void *msg) ;

// destroi a fila, liberando as tarefas bloqueadas
int mqueue_destroy (mqueue_t *queue) ;

// informa o número de mensagens atualmente na fila
int mqueue_msgs (mqueue_t *queue) ;

//==============================================================================

// Redefinir principais funcoes POSIX como "FORBIDDEN",
// para impedir seu uso (gera erro ao compilar)

// threads
#define pthread_create(x)			FORBIDDEN
#define pthread_join(x)			FORBIDDEN
#define pthread_exit(x)			FORBIDDEN

// condvars
#define pthread_cond_init(x)			FORBIDDEN
#define pthread_cond_wait(x)			FORBIDDEN
#define pthread_cond_signal(x)		FORBIDDEN
#define pthread_cond_timedwait(x)		FORBIDDEN

// barriers
#define pthread_barrier_init(x)		FORBIDDEN
#define pthread_barrier_wait(x)		FORBIDDEN
#define pthread_barrier_destroy(x)		FORBIDDEN

// mutexes
#define pthread_mutex_init(x)			FORBIDDEN
#define pthread_mutex_lock(x)			FORBIDDEN
#define pthread_mutex_unlock(x)		FORBIDDEN
#define pthread_mutex_timedlock(x)		FORBIDDEN
#define pthread_mutex_trylock(x)		FORBIDDEN
#define pthread_mutex_destroy(x)		FORBIDDEN

// RW-locks
#define pthread_rwlock_init(x)		FORBIDDEN
#define pthread_rwlock_rdlock(x)		FORBIDDEN
#define pthread_rwlock_wrlock(x)		FORBIDDEN
#define pthread_rwlock_unlock(x)		FORBIDDEN
#define pthread_rwlock_tryrdlock(x)		FORBIDDEN
#define pthread_rwlock_tryrwlock(x)		FORBIDDEN
#define pthread_rwlock_timedrdlock(x)		FORBIDDEN
#define pthread_rwlock_timedrwlock(x)		FORBIDDEN
#define pthread_rwlock_destroy(x)		FORBIDDEN

// spinlocks
#define pthread_spin_init(x)			FORBIDDEN
#define pthread_spin_lock(x)			FORBIDDEN
#define pthread_spin_unlock(x)		FORBIDDEN
#define pthread_spin_trylock(x)		FORBIDDEN
#define pthread_spin_destroy(x)		FORBIDDEN

// semaphores
//#define sem_init(x)				FORBIDDEN
#define sem_post(x)				FORBIDDEN
#define sem_wait(x)				FORBIDDEN
#define sem_trywait(x)				FORBIDDEN

// message queues
#define mq_open(x)				FORBIDDEN
#define mq_send(x)				FORBIDDEN
#define mq_receive(x)				FORBIDDEN
#define mq_close(x)				FORBIDDEN

// time
#define asctime_r(x)				FORBIDDEN
#define asctime(x)				FORBIDDEN
#define clock_gettime(x)			FORBIDDEN
#define clock(x)				FORBIDDEN
#define ctime(x)				FORBIDDEN
#define difftime(x)				FORBIDDEN
#define ftime(x)				FORBIDDEN
#define gettimeofday(x,y)			FORBIDDEN
#define gmtime_r(x)				FORBIDDEN
#define gmtime(x)				FORBIDDEN
#define hwclock(x)				FORBIDDEN
#define localtime_r(x)				FORBIDDEN
#define localtime(x)				FORBIDDEN
#define mktime(x)				FORBIDDEN
#define settimeofday(x,y)			FORBIDDEN
//#define sleep(x)				FORBIDDEN
#define strftime(x)				FORBIDDEN
#define strptime(x)				FORBIDDEN
#define timegm(x)				FORBIDDEN
#define time(x)				FORBIDDEN
#define tzset(x)				FORBIDDEN
#define utime(x)				FORBIDDEN

// Process execution
#define execle(x,y)				FORBIDDEN
#define execlp(x,y)				FORBIDDEN
#define execl(x,y)				FORBIDDEN
#define execveat(x)				FORBIDDEN
#define execve(x,y,z)				FORBIDDEN
#define execvpe(x,y,z)				FORBIDDEN
#define execvp(x,y)				FORBIDDEN
#define execv(x,y)				FORBIDDEN
#define exec(x)				FORBIDDEN
#define fexecve(x)				FORBIDDEN
#define fork(x)				FORBIDDEN
#define ptrace(x)				FORBIDDEN
#define system(x)				FORBIDDEN

#endif