#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/shm.h>
#include <stdbool.h>
#include <sys/sem.h>

#if defined(GNU_LIBRARY) && !defined(_SEM_SEMUN_UNDEFINED)
#else
union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
	struct seminfo *__buf;
};
#endif

#define DELAY 50000
#ifdef MUTEX
    pthread_mutex_t exclusion;
#endif


int enemy[30];

int menu=0;
int seconds=0;
int i = 0;
int distance = 0;
int middleY = 0;
bool initPlay=true;
bool p1_can_enter,p2_can_enter;

struct sembuf P1;
struct sembuf P2;
key_t lightKey;
int lightId;

void initSharedMemory();
void selection();
void paintFrame();
void informationPlayers();
void left();
void right();
void control();
void *attack (void *parametro);
void *writeP2(void *parametro);
void *readP1(void *parametro);
void *timer(void *parametro);

int x=0,y=0,maxX=0,maxY=0;

FILE *sharedFile2;
key_t keyShared;
int memory = 0,
currentThread=0;
char *sharedMemory = NULL;
pthread_t aThread;
pthread_t bThread;
pthread_t cThread;
pthread_t dThread;

int TURN =0, live=5, ptos=0, flagVida=5, player=0,OP=0,liveOP=5,ptoOP=0;
bool point=false;


void informationPlayers(){
    
    paintFrame();

    char concat[20];
    int positionX = 0;
    int positionY = 0;    

    positionX = maxX - 13;
    positionY = maxY/2;

    mvprintw(positionY, positionX,"time_");
  
    positionY++;

    sprintf(concat,"%d%s", seconds," s");
    mvprintw(positionY,positionX,concat);

	char score[20];
	sprintf(score,"%s%d","Score ",ptos);

	char scoreEnemy[20];
	sprintf(scoreEnemy,"%s%d","Score ",ptoOP);
    *concat = '\0';

    positionY = maxY-2;

	if(player==1)
	{
	    sprintf(concat,"%s%d","Defensor ", live);
	    mvprintw(positionY, positionX, concat);
        positionY++;
	    mvprintw(positionY, positionX, score);
        *concat = '\0';

	    
        sprintf(concat,"%s%d", "Invasor ", liveOP);
	    mvprintw(0,positionX,concat);
	    mvprintw(1,positionX,scoreEnemy);
	}
	else
	{
        sprintf(concat,"%s%d","Invasor ", live);
	    mvprintw(positionY, positionX, concat);
        positionY++;
	    mvprintw(positionY, positionX, score);
        *concat = '\0';
	    
        sprintf(concat,"%s%d", "Defensor ", liveOP);
	    mvprintw(0,positionX,concat);
	    mvprintw(1,positionX, scoreEnemy);
	}
}

void paintEnemy(){
    for(i = 0; i < 20; i ++)
    {
        if(i < 5)
        {
        if(enemy[i]==1)
            {
                mvprintw(middleY + 2, (distance * (i+5)) + 1,"\\-.-/");
                /*if (i!=0)
                {
                    mvprintw(middleY + 2, (distance * (i+5)) + 1,"\\-.-/");
                }
                else
                {
                    mvprintw(middleY + 2, (distance * (i+5)) + 1,"(/-1-\\)");  
                }*/
            }
            else
            {
                mvprintw(middleY + 2, (distance * (i+5)) + 1,"     ");
            }
        }
        else if (i >= 5 && i < 10)
        {
            if(enemy[i]==1)
            {
                mvprintw(middleY + 1, (distance * i) + 1,"\\-.-/");
                /*if (i!=6)
                {
                    mvprintw(middleY + 1, (distance * i) + 1,"\\-.-/");
                }       
                else
                {
                    mvprintw(middleY + 1, (distance * i) + 1,"(/-2-\\)");
                }*/   
            }
            else
            {
                mvprintw(middleY + 1, (distance * i) + 1,"     ");
            }
        }
        else if (i >= 10 && i < 15)
        {
            if(enemy[i]==1)
            {
                mvprintw(middleY, (distance * (i-5)) + 1,"\\-.-/");
                /*if(i=!12)
                {
                    mvprintw(middleY, (distance * (i-5)) + 1,"\\-.-/");
                }
                else
                {
                    mvprintw(middleY, (distance * (i-5)) + 1,"(/-3-\\)");
                }*/
            }
            else
            {
                mvprintw(middleY, (distance * (i-5)) + 1,"     ");
            }
        }
        else if (i >= 15 && i < 20)
        {
            if(enemy[i]==1)
            {
                mvprintw(middleY - 1, (distance * (i - 10)) + 1,"\\-.-/");
                /*if(i=!12)
                {
                    mvprintw(middleY - 1, (distance * (i - 10)) + 1,"\\-.-/");
                }
                else
                {
                    mvprintw(middleY - 1, (distance * (i - 10)) + 1,"(/-4-\\)");   
                }*/
            }
            else
            {
                mvprintw(middleY - 1, (distance * (i - 10)) + 1,"     ");
            }
        }
      //  mvprintw(middleY + 3,  i, "%d", enemy[i]);
    }
}

void control()
{
    #ifdef MUTEX
	pthread_mutex_lock (&exclusion);
    #endif
    clear();

    initscr();			        
	raw();				         
	keypad(stdscr, TRUE);		 
	noecho();			         
    

    if((liveOP == 1) || (live == 1))
    {
            printw("Juego Finalizado");
            menu = getch();
    }

    clear();
    informationPlayers();
   
    distance= (maxX / 5) - 20;
    middleY= maxY / 2;
    y = maxY - 2;
	mvprintw(2, maxX - OP - 25,"<--->");  
    mvprintw(y, x,"<--->"); 

    paintEnemy(); 

    menu = getch();
	switch(menu)
	{
	    case KEY_UP:
            currentThread= pthread_create (&aThread, NULL, attack, NULL);
            break;
	    case KEY_LEFT:
            left();
            break;
	    case KEY_RIGHT:
            right();
            break;
	    case 'a':
            shmdt ((char *)sharedMemory);
            shmctl (memory, IPC_RMID,(struct shmid_ds *)NULL); 
            unlink ("\tmp\sharedFile2"); 
            endwin();
            return;
            break;
         case KEY_DOWN:
            clear();
            printw("Charts \n");
            
            char *point="Puntos : ";
            char *time_="Tiempo : ";

            char ptos1[20];
            char ptos2[20];
            char tiempov[20];
            sprintf(tiempov,"%s%d\n",time_,seconds);
            if(player==1)
            {
                sprintf(ptos1,"%s%d\n",point,ptos);
                sprintf(ptos2,"%s%d\n",point,ptoOP);
            }else
            {
                sprintf(ptos2,"%s%d\n",point,ptos);
                sprintf(ptos1,"%s%d\n",point,ptoOP);
            }

            if(player==1)
            {
                    printw("Invasor \n");
                    printw(ptos2);
                    printw("Defensor \n");
                    printw(ptos1);
                
            }
            else
            {
                    printw("Defensor \n");
                    printw(ptos2);
                    printw("Invasor \n");
                    printw(ptos1);
            }
            printw(tiempov);
            menu = getch();
            break;
	}

	refresh();	
    #ifdef MUTEX
	pthread_mutex_unlock (&exclusion);
    #endif
	control();
}

void left()
{
        if(x!=1)
        {
            mvprintw(y, x, "<--->"); 
            refresh();
            usleep(DELAY); 
            x--; 
        }
        return;
}
void right()
{
        if(x!=maxX-20)
        {
            mvprintw(y, x, "<--->");
            refresh();
            usleep(DELAY); 
            x++;
        }
        return;
}
void *attack (void *parametro)
{
    int ballX=x;
    int ballY=y;
    int i=0;
    bool hit=false;
    getmaxyx(stdscr, maxY, maxX);

    while(ballY>0){
        
        #ifdef MUTEX
		pthread_mutex_lock (&exclusion);
        #endif
        clear();

        //paintFrame();
        
        mvprintw(y, x, "<--->");         
        mvprintw(ballY, ballX, "*");   
        refresh();
        usleep(DELAY);                   
        ballY--;                        

        if(ballY == middleY + 2)
        {
            for(i = 0; i < 5; i++)
            {
                if((ballX == (distance * (i + 5)) + 1) && (enemy[i] == 1 ))
                {
                    hit=true;
                    enemy[i]  =0;
                    sharedMemory[i + 8] = 0;
                    ptos= ptos + 10;
                    mvprintw(ballY, ballX, "BAMM");
                }
            }
        }

        else if(ballY == middleY + 1)
        {
            for(i = 5; i < 10;i++)
            {
                if((ballX == (distance * i) + 1) && (enemy[i] == 1 ))
                {
                    hit=true;
                    enemy[i]  =0;
                    sharedMemory[i + 8] = 0;
                    ptos= ptos + 10;
                    mvprintw(ballY, ballX, "BAMM");
                }
            }
        }

        else if(ballY == middleY)
        {
            for(i = 10; i < 15;i++)
            {
                if((ballX == (distance * (i - 5)) + 1) && (enemy[i] == 1 ))
                {
                    hit=true;
                    enemy[i]  =0;
                    sharedMemory[i + 8] = 0;
                    ptos= ptos + 10;
                    mvprintw(ballY, ballX, "BAMM");
                }
            }
        }

        else if(ballY == middleY -1)
        {
            for(i = 15; i < 20; i++)
            {
                if((ballX == (distance * (i - 10)) + 1) && (enemy[i] == 1 ))
                {
                    hit=true;
                    enemy[i]  =0;
                    sharedMemory[i + 8] = 0;
                    ptos= ptos + 10;
                    mvprintw(ballY, ballX, "BAMM");
                }
            }
        }

        else if(ballY==2)
        {
            if(ballX==maxX - 25 - OP)
            {
                flagVida = flagVida - 1;
                mvprintw(ballY, ballX, "BAMM");
                hit=true;
                point=true;
            }
        }
        if(hit==true)
        {
            refresh();
            mvprintw(ballY, ballX, "*"); 
            usleep(DELAY);       
            break;
        }
    
    #ifdef MUTEX
    pthread_mutex_unlock (&exclusion);
    #endif
    }
}

void *writeP2(void *parametro)
{
    while( true )
    {
        p1_can_enter = true;
        while( p2_can_enter )
        {
            if( TURN == 2 )
            {
            p1_can_enter = false;
            while( TURN == 2 )
            {   }
            p1_can_enter = true;
            }
        }
        
        
        sharedMemory[8] = enemy[0];
        sharedMemory[9] = enemy[1];
        sharedMemory[10] = enemy[2];
        sharedMemory[11] = enemy[3];
        sharedMemory[12] = enemy[4];
        sharedMemory[13] = enemy[5];
        sharedMemory[14] = enemy[6];
        sharedMemory[15] = enemy[7];
        sharedMemory[16] = enemy[8];
        sharedMemory[17] = enemy[9];
        sharedMemory[18] = enemy[10];
        sharedMemory[19] = enemy[11];
        sharedMemory[20] = enemy[12];
        sharedMemory[21] = enemy[13];
        sharedMemory[22] = enemy[14];
        sharedMemory[23] = enemy[15];
        sharedMemory[24] = enemy[16];
        sharedMemory[25] = enemy[17];
        sharedMemory[26] = enemy[18];
        sharedMemory[27] = enemy[19];
        
        
        if(player==1)
        {
            if(point==true)
            {
                sharedMemory[6]=flagVida;
                point=false;
            }

            sharedMemory[2]= live;
            sharedMemory[1]= x;
            sharedMemory[3]= ptos;
        }
        
        else
        {
            if(point==true)
            {
                sharedMemory[2]=flagVida;
                point=false;
            }

            sharedMemory[5]=x;
            sharedMemory[6]=live;
            sharedMemory[7]=ptos;
        }

        TURN = 2;
        p1_can_enter = false;
    }
}

void *readP1(void *parametro)
{
    while( true )
    {
        p2_can_enter = true;
        while( p1_can_enter )
        {
            if( TURN == 1 )
            {
            p2_can_enter = false;
            while( TURN == 1 ){}
            p2_can_enter = true;
            }
        }
        
        
        enemy[0]=sharedMemory[8]; 
        enemy[1]=sharedMemory[9]; 
        enemy[2]=sharedMemory[10]; 
        enemy[3]=sharedMemory[11]; 
        enemy[4]=sharedMemory[12]; 
        enemy[5]=sharedMemory[13]; 
        enemy[6]=sharedMemory[14]; 
        enemy[7]=sharedMemory[15]; 
        enemy[8]=sharedMemory[16]; 
        enemy[9]=sharedMemory[17]; 
        enemy[10]=sharedMemory[18];
        enemy[11]=sharedMemory[19]; 
        enemy[12]=sharedMemory[20]; 
        enemy[13]=sharedMemory[21]; 
        enemy[14]=sharedMemory[22]; 
        enemy[15]=sharedMemory[23]; 
        enemy[16]=sharedMemory[24]; 
        enemy[17]=sharedMemory[25]; 
        enemy[18]=sharedMemory[26]; 
        enemy[19]=sharedMemory[27];
         

        if(player == 1)
        {
            liveOP = sharedMemory[6];
            ptoOP = sharedMemory[7];
            OP = sharedMemory[5];
        }
        else
        {
            liveOP = sharedMemory[2];
            ptoOP = sharedMemory[3];
            OP = sharedMemory[1];
        }
        
        TURN = 1;
        p2_can_enter = false;
    }
}

    /*
        boolean p1_puede_entrar, p2_puede_entrar;
        int turno;


        Proceso1()
        {
        while( true )
        {
        [REALIZA_TAREAS_INICIALES]
        p1_puede_entrar = true;
        while( p2_puede_entrar )
        {
        if( turno == 2 )
        {
            p1_puede_entrar = false;
            while( turno == 2 ){}
            p1_puede_entrar = true;
        }
        }
        [REGION_CRITICA]
        turno = 2;
        p1_puede_entrar = false;
        [REALIZA_TAREAS_FINALES]
        }
        }
        
        Proceso2()
        {
        while( true )
        {
        [REALIZA_TAREAS_INICIALES]
        p2_puede_entrar = true;
        while( p1_puede_entrar )
        {
        if( turno == 1 )
        {
            p2_puede_entrar = false;
            while( turno == 1 ){}
            p2_puede_entrar = true;
        }
        }
        [REGION_CRITICA]
        turno = 1;
        p2_puede_entrar = false;
        [REALIZA_TAREAS_FINALES]
        }
        }
        
        iniciar()    private HomeKeyLocker mHomeKeyLocker;

        {
        p1_puede_entrar = false;
        p2_puede_entrar = false;
        turno = 1;
        Proceso1();
        Proceso2();
    }*/


void paintFrame(){
    for(i = 0; i < maxY; i++)
	{
	    mvprintw(i, maxX - 20, "|");
	}
}

void selection(){
    
    int option = 0;

    //Alive enemy
    /*for(i = 8; i <= 27; i++){
        sharedMemory[i] = 1;
        enemy[i-8] = 1;
    }*/
    //Alive enemy

    clear();
    printw("         --------------------------------------\n");
    printw("        |           player 1 Defensor         |\n");
    printw("         --------------------------------------\n\n\n");

    printw("         --------------------------------------\n");
    printw("        |           player 2 Invasor          |\n");
    printw("         --------------------------------------\n\n\n");
    printw("      Debes esperar mientras se conecta tu oponente\n");
    printw("\n positions %d%d",sharedMemory[27],enemy[19]);


    option = getch();
    
    switch(option){  
        case '1':
            clear();
            printw("Seleccionaste 1: \nDefensor \nEspera mientras se conecta el Invasor");
            break;  
        case '2':
            clear();
            printw("Seleccionaste 2: \nInvasor \nEspera mientras se conecta el Defensor"); 
            break;         
    }
    
    getch();
    lightKey = ftok ("/bin/ls", 33);
    lightId = semget (lightKey, 10, 0600 | IPC_CREAT);
    
    switch(option){  
        case '1':
    
            semctl (lightId, 0, SETVAL, 0);
            P1.sem_num = 0;
            P1.sem_op = -1;
            P1.sem_flg = 0;
            semop (lightId, &P1, 1);
            
             
            /*clear();
            printw("option1");
            getch();*/

            player=1;
            sharedMemory[0] = 1;
            sharedMemory[1] = x;
            sharedMemory[2] = live;
            sharedMemory[3] = ptos;    

            /*clear();
            printw("option1-2");
            getch();*/  
         
            currentThread= pthread_create (&bThread, NULL, readP1, NULL);
            currentThread=pthread_create(&cThread,NULL, writeP2,NULL);
            currentThread= pthread_create (&dThread, NULL, timer, NULL);

            /*clear();
            printw("option1-3");
            getch();*/ 

            break;  

        case '2':
   
            P2.sem_num = 0;
            P2.sem_op = 1;
            P2.sem_flg = 0;

            for (i = 0; i<1; i++)
            {
                semop (lightId, &P2, 1);
                sleep (1);
            }

            /*clear();
            printw("option1");
            getch();*/
            
            player=2;
            sharedMemory[4] = 1;
            sharedMemory[5] = x;
            sharedMemory[6] = live;
            sharedMemory[7] = ptos;
            
            currentThread= pthread_create (&bThread, NULL, readP1, NULL);
            currentThread= pthread_create (&cThread, NULL, writeP2, NULL);
            currentThread= pthread_create (&dThread, NULL, timer, NULL);
            
            break;

            default:
            break;    
    }


    #ifdef MUTEX
	pthread_mutex_init (&exclusion, NULL);
    #endif
    
    /*clear();
    printw("option1-delay");
    getch();*/ 

    usleep(DELAY); 
    control();

    /*clear();
    printw("option1-Postdelay");
    getch();*/ 

    shmdt ((char *)sharedMemory);
    shmctl (memory, IPC_RMID,(struct shmid_ds *)NULL); 
    unlink ("\tmp\sharedFile2");
}

void initSharedMemory()
{
    sharedFile2 = fopen("/tmp/sharedFile2","w+");
    keyShared = ftok ("/tmp/sharedFile2",33);
    memory = shmget(keyShared,sizeof(int *)*100,0777 | IPC_CREAT);
    sharedMemory = (int *) shmat(memory,(char *)0,0);

    //Alive enemy
   /* for(i = 8; i <= 27; i++){
        sharedMemory[i] = 1;
        enemy[i-8] = 1;
    }
    */
    //Alive enemy

    initPlay = false;
    TURN = 1;
    p1_can_enter=false;
    p2_can_enter=false;
}

void *timer(void *parametro)
{
    while(true)
    {  
        #ifdef MUTEX
		pthread_mutex_lock (&exclusion);
        #endif

        seconds++;


        #ifdef MUTEX
		pthread_mutex_unlock (&exclusion);
        #endif

        sleep(1);
    }
}

int main()
{
    initscr();
    curs_set(FALSE);
    getmaxyx(stdscr, maxY, maxX);

    sharedFile2 = fopen("/tmp/sharedFile2","w+");
    keyShared = ftok ("/tmp/sharedFile2",33);
    memory = shmget(keyShared,sizeof(int *)*100,0777 | IPC_CREAT);
    sharedMemory = (int *) shmat(memory,(char *)0,0);

    sharedMemory[8] = 1;
    sharedMemory[9] = 1;
    sharedMemory[10] = 1;
    sharedMemory[11] = 1;
    sharedMemory[12] = 1;
    sharedMemory[13] = 1;
    sharedMemory[14] = 1;
    sharedMemory[15] = 1;
    sharedMemory[16] = 1;
    sharedMemory[17] = 1;
    sharedMemory[18] = 1;
    sharedMemory[19] = 1;
    sharedMemory[20] = 1;
    sharedMemory[21] = 1;
    sharedMemory[22] = 1;
    sharedMemory[23] = 1;
    sharedMemory[24] = 1;
    sharedMemory[25] = 1;
    sharedMemory[26] = 1;
    sharedMemory[27] = 1;

    enemy[0] = 1;
    enemy[1] = 1;
    enemy[2] = 1;
    enemy[3] = 1;
    enemy[4] = 1;
    enemy[5] = 1;
    enemy[6] = 1;
    enemy[7] = 1;
    enemy[8] = 1;
    enemy[9] = 1;
    enemy[10] = 1;
    enemy[11] = 1;
    enemy[12] = 1;
    enemy[13] = 1;
    enemy[14] = 1;
    enemy[15] = 1;
    enemy[16] = 1;
    enemy[17] = 1;
    enemy[18] = 1;
    enemy[19] = 1;

    TURN = 1;
    p1_can_enter=false;
    p2_can_enter=false;
    //initSharedMemory();

    printw("                 --------------------- \n");
    printw("                *                     *\n");
    printw("                |    Space Invaders   |\n");
    printw("                *                     *\n");
    printw("                |      Bienvenido     |\n");
    printw("                *                     *\n");
    printw("                 --------------------- \n\n\n");


    printw("         --------------------------------------\n");
    printw("        |      Presiona Enter para continuar   |\n");
    printw("         --------------------------------------\n\n\n");

    printw("                Anicka Cercado - 201212859");
    printw("\n positions %d%d",sharedMemory[27],enemy[19]);

    menu=getch();

    switch(menu){
        default:
            selection();
    }

    return 0;
}



