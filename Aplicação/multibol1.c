#include "sema.h"
#include <sys/shm.h>

#define JOGADORES   		100
#define LIFETIME    		30			
#define SHMKEY 			100
#define SHMKEY2 		102
#define SHMKEY4			104
#define SHMKEY5			105

#define TEMPOESPERA		1000500	

semaphore mutex, semJogadores[JOGADORES];

int *golos;
int *remates;
int *jogadores;

float *precisao;

int players, nBolas;

void nJogadores();
void numeroBolas();
void distribuirBolas();
void bolas();

void jogador(int i);

void rematar(int i);
void guardarGolos(int i);
void guardarRemates(int i);
void acrescentarBola(int i);
void removerBola(int i);

void calcularPrecisao();
void estatisticas();



main ()
{
	nJogadores();
	numeroBolas();

int shmid = shmget(SHMKEY, JOGADORES, 0777|IPC_CREAT);/* array de jogadores para guardar as bolas de cada jogador */
char *addr = (char *) shmat(shmid,0,0);
jogadores = (int*) addr;

int shmid2 = shmget(SHMKEY2, JOGADORES, 0777|IPC_CREAT);/* array para guardar os remates de cada jogador */
char *addr2 = (char *) shmat(shmid2,0,0);
remates = (int*) addr2;

int shmid4 = shmget(SHMKEY4, JOGADORES, 0777|IPC_CREAT);/* array para guardar os golos de cada jogador */
char *addr4 = (char *) shmat(shmid4,0,0);
golos = (int*) addr4;

float shmid5 = shmget(SHMKEY5, JOGADORES, 0777|IPC_CREAT);/* array para guardar a precisao de remate de cada jogador */
char *addr5 = (char *) shmat(shmid5,0,0);
precisao = (float*) addr5;


	int	child_pid [players],				/* process ID's */
		wait_pid;					/* pid of terminated child */
	int	i, j,						/* loop variables */
		child_status;					/* return status of child */

	for(i = 0; i < players; ++i){ 
	semJogadores[i] = init_sem(0); /* iniciar o semafero de cada jogador a 0 */
			}

	mutex = init_sem(1);/* iniciar semafero a 1 */	

	srand(time(NULL));
	
	distribuirBolas();/* atribuir bolas a jogadores */
	bolas();/* imprimir quem tem quantas bolas */

	sleep(2);/* espera 2 segundos antes de começar o jogo */	
		
	for (i = 0; i < players; ++i)
	{
		child_pid [i] = fork ();
		switch (child_pid [i])
		{
			case -1:			/* error: no process created	*/
				perror ("fork failed");
				exit (1);
				break;
			case 0:				/* child process		*/
				
			if(i < JOGADORES){

				alarm(LIFETIME);
				while(1){
					jogador(i);

					}	
				}

				break;
			default:			/* parent process		*/
				if (i == (players - 1))/* all childs created ?	*/
				{			/* yes -> wait for termination	*/
					for (j = 0; j < players; ++j)
					{
						wait_pid = wait (&child_status);
						if (wait_pid == -1)
						{
							perror ("wait failed");
						} else {
							printf("Child %d terminated with code %d\n", wait_pid, child_status);
						};
					};
					printf ("All child processes have terminated.\n");

					printf("JOGO ACABOU\n");
						
						calcularPrecisao();
						estatisticas();	

					rel_sem (mutex);

					for(i = 0; i < players; i++){
						rel_sem(semJogadores[i]);

							}

					shmdt(addr);
					shmctl(shmid, IPC_RMID, NULL);

					shmdt(addr2);
					shmctl(shmid2, IPC_RMID, NULL);
		
					shmdt(addr4);
					shmctl(shmid4, IPC_RMID, NULL);
				
					shmdt(addr5);
					shmctl(shmid5, IPC_RMID, NULL);
	

			};
		};					/* end switch			*/
	};					/* end for			*/
}

void nJogadores()/* pede o numero de jogadores ao utilizador */
{
	do{
	printf("Quantos jogadores quer?\n");
	scanf("%d", &players);
	}while( players > 100 || players < 3);
}

void numeroBolas()/* pede o numero de bolas ao utilizador */
{
	do{
	printf("Quantas bolas ha em jogo?\n");
	scanf("%d", &nBolas);
	}while(nBolas > players && nBolas < 1);
}

void distribuirBolas()/* metodo para dar bolas a cada jogador aleatorio */
{
	int q;
	for(q = 0; q < nBolas; ++q)
		{
		int jogadorAleatorio = rand() % players;/* escolhe jogador aleatorio */
		int valor = *(jogadores + jogadorAleatorio); /* guarda o numero de bolas no jogadorAleatorio */
		*(jogadores + jogadorAleatorio) = valor + 1; /* acrescenta uma bola ao jogadorAleatorio */
		V(semJogadores[jogadorAleatorio]);
		}
}

void bolas()/* imprimi as bolas que cada jogador tem */
{
	int q;
	for(q = 0; q < players; ++q){
	if(*(jogadores + q) > 0){
	printf("O jogador %d tem %d bolas\n", q, *(jogadores + q));
		}
	}
}

void jogador(int i)/* metodo que chama cada jogador para jogar*/
{
	while(1){

	P(semJogadores[i]);// se == 1 entra para 

	usleep((unsigned int) rand() % TEMPOESPERA);
	
	rematar(i);
	}
}

void rematar(int i)/* metodo para o jogador rematar a bola */
{	
	int receptor;

	do{
	receptor = rand() % players; /* escolhe o jogador para quem vai rematar */
	}while(receptor == i);

	int golo = rand() % 2;/* remata e tem 50% de hipotesse */

	if(golo == 1){
		printf("O Jogador %d marcou 1 golo na baliza do jogador %d\n", i, receptor);
		guardarGolos(i);
	}else{
		printf("O Jogador %d nao marcou golo na baliza do jogador %d\n", i, receptor);
		}

	guardarRemates(i);
	removerBola(i);
	acrescentarBola(receptor);
}

void guardarGolos(int i)/* guarda os golos do jogador i */
{
	P(mutex);
	int valor = *(golos + i);//verifica quantos golos tem o jogador que rematou
	*(golos + i) = valor + 1;/* incrementa um golo para o jogador que rematou */
	V(mutex);
}

void guardarRemates(int i)/* guarda o remate que foi feito no respectivo jogador */
{
	P(mutex);
	int valor = *(remates + i);/* quantos remates já fez o que rematou */
	*(remates + i) = valor + 1;/* incrementa um remate no jogador que rematou*/
	V(mutex);
}

void removerBola(int i)/* remove uma bola */
{
	P(mutex);
	int valor = *(jogadores + i);//vai buscar o numero de bola do jogador i
	*(jogadores + i) = valor - 1;//remove uma bola					
	V(mutex);
}

void acrescentarBola(int i)/* acrescenta uma bola ao jogador i */
{
	P(mutex);
	int valor = *(jogadores + i);/* Quantas bolas tem o jogador que sofreu remate */
	*(jogadores + i) = valor + 1;/* acrescenta um bola ao jogador que sofreu o remate */
	V(mutex);
	V(semJogadores[i]);
}

void estatisticas()/* imprimi os resultados finais */
{
	int k;
	for(k = 0; k < players; ++k){
		printf("O jogador %d, marcou %d golos com %d remates e uma precisao de %.2f\n", k, *(golos + k), *(remates + k), *(precisao + k)); 		
			}		
}

void calcularPrecisao()/* calcula a precisao de remates de cada jogador */
{
	int x;
	for(x = 0; x < players; ++x){

		P(mutex);

		int valorRemates = *(remates + x);
		int valorGolos = *(golos + x);
		if(valorGolos > 0 && valorRemates > 0){

		*(precisao + x) =  ((float) valorRemates / (float) valorGolos);

		}else{

		*(precisao + x) = 0;

		}
		V(mutex);

		}
}

