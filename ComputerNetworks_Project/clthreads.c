#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <wait.h>
#include <time.h>

#define MAX_TRAINS 10000

extern int errno;
int port;

/*STRUCTURI*/

struct Trains_DataBase{
    char trainNumber[30];
    char arrivalTime[30];
    char departureTime[30];
    char delay[30];
    char ArrivalUpdate[30];
    char DepartureUpdate[30];
};

struct Train_Stations {
    char stationName[30];
    struct Trains_DataBase DataBase[MAX_TRAINS];
    int trainsNumber;
};

typedef struct CommandsQueue{
    int items[30];
    int front;
    int rear;
};




/*DEFINIREA FUNCTIILOR*/

void sendLocation(int socketDescriptor);

void receiveTodayTrainList(int socketDescriptor, struct Train_Stations *Station);

void printTodayTrainList(struct Train_Stations station);

void initializeQueue(struct CommandsQueue *queue);

void enqueue(struct CommandsQueue *queue, int command);

void sendCommand(int socketDescriptor, struct CommandsQueue *queue);

void queueFill(struct CommandsQueue *queue);

void receiveNextHourList(int socketDescriptor, struct Train_Stations *Station);

void printNextHourArrivals(struct Train_Stations station);

void printNextHourDepartures(struct Train_Stations station);

void prinBilet(struct Train_Stations station);

void UpdateSending(int update, struct Train_Stations *station );


/*INT MAIN*/

int main (int argc, char *argv[])
{
  int sd;		
  struct sockaddr_in server;	

  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  port = atoi (argv[2]);

  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(argv[1]);
  server.sin_port = htons (port);

  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }


  sendLocation(sd);
  
  struct Train_Stations station;
  receiveTodayTrainList(sd,&station);
  
  printTodayTrainList(station);
  
  struct CommandsQueue queue;
  queueFill(&queue);

  sendCommand(sd,&queue);

  while(queue.front != queue.rear){ 
      if(queue.items[queue.front]==1)
             printNextHourArrivals(station);
      else {
         if(queue.items[queue.front]==2)
              printNextHourDepartures(station);
         else{
            if(queue.items[queue.front]==4)
                printBilet(station);

          else
            {
               UpdateSending(queue.items[queue.front],&station);
            }
         }
          }
      queue.front++;
  }


  close (sd);
}



/*IMPLEMENTAREA FUNCTIILOR*/

void sendLocation(int socketDescriptor)
{
  char stationName[10];
  printf("\n\n\033[1;32m--> Specificati in ce gara va aflati: \033[0m");
  fflush (stdout);
  read (0, stationName, sizeof(stationName));
  
  if(write(socketDescriptor,&stationName,sizeof(stationName))<=0){
      perror ("[Client] Eroare la write() spre server.\n");
  }
  
}

void receiveTodayTrainList(int socketDescriptor, struct Train_Stations *Station)
{
    if (read(socketDescriptor, Station, sizeof(struct Train_Stations)) <= 0) {
        perror("[Client] Eroare la read() de la server (dimensiune structura).\n");
        return;
    }

}

void printTodayTrainList(struct Train_Stations station)
{
  printf("\n                                        \033[1;31m Mersul trenurilor din ziua curenta pentru %s \033[0m\n",station.stationName);
  printf("\n\033[1;34m_________________________________________________________________________________________________________________________________________\033[0m");
  printf("\n\033[1;34m|                      |                 |                      |                        |             |               |                |\033[0m");
  printf("\n\033[1;34m|     TRAIN NUMBER     |     STATION     |     ARRIVAL_TIME     |     DEPARTURE_TIME     |    DELAY    |    A_UPDATE   |    D_UPDATE    |\033[0m");
  printf("\n\033[1;34m|______________________|_________________|______________________|________________________|_____________|_______________|________________|\033[0m");
  for(int i=0;i< 30;i++){
            printf("\n|                      |                 |                      |                        |             |               |                |");
            printf("\n|        %s          |     %s    |         %s        |          %s         |     \033[1;31m%s\033[0m     |     \033[1;32m%s\033[0m     |      \033[1;32m%s\033[0m     |",station.DataBase[i].trainNumber,station.stationName,station.DataBase[i].arrivalTime,station.DataBase[i].departureTime,station.DataBase[i].delay,station.DataBase[i].ArrivalUpdate,station.DataBase[i].DepartureUpdate);
            printf("\n|______________________|_________________|______________________|________________________|_____________|_______________|________________|");
  }
  printf("\n");
}

void initializeQueue(struct CommandsQueue *queue){
    queue->front = -1;
    queue->rear = -1;
}

void enqueue(struct CommandsQueue *queue, int command){
  if(queue->front == -1 && queue->rear == -1){
     queue->front = queue->rear = 0;
  }
  else{
      queue->rear = (queue->rear + 1) % 30;
  }
  
   queue->items[queue->rear]=command;

}

void queueFill(struct CommandsQueue *queue){
  initializeQueue(queue);
  int i=0;
  char raspuns[]="da";
  printf("\n\n\033[1;31mMai aveti si alte cereri/update-uri?(da/nu):\033[0m");
  scanf("%s",raspuns);
  char comanda[30];
  while(strstr(raspuns,"da")!=0)
   {     
        printf("\n\033[1;32m--> \033[0m Pentru informatii despre plecari in urmatoarea ora, apasati:\033[1;31m 1 \033[0m\n\033[1;32m--> \033[0m Pentru informatii despre sosiri in urmatoarea ora, apasati:\033[1;31m 2 \033[0m\n\033[1;32m--> \033[0m Pentru a trimite informatii despre posibilele intarzieri apasati:\033[1;31m 3 \033[0m\n\033[1;32m--> \033[0m Pentru cumparare bilet, apasati:\033[1;31m 4 \033[0m\n");
        scanf("%s", comanda);
    
        if(strstr(comanda,"3")!=0)
        {
          char update[30];
          printf("\033[1;32m--> Transmiteti numarul trenului, numarul de minute in care intarzie \033[0m \033[1;31m(format [tren][min])\033[0m: ");
          scanf("%s",update);
          int tren,min;
          sscanf(update, "[%d][%d]", &tren,&min);
          tren=tren*100+min;
          enqueue(queue,tren);
        }
        else 
        {
           enqueue(queue,atoi(comanda));
        }
        
        printf("\n\n\033[1;31mMai aveti si alte cereri/update-uri?(da/nu):\033[0m");
        scanf("%s",raspuns);
    }
  strcpy(comanda,"1");
  enqueue(queue,-atoi(comanda));




}

void sendCommand(int socketDescriptor,struct CommandsQueue *queue)
{
 if(write(socketDescriptor,queue, sizeof(struct CommandsQueue))<=0){
    perror("[Client] Eroare la trimiterea comenzilor");
  
  }
  fsync(socketDescriptor);
}

void receiveNextHourList(int socketDescriptor, struct Train_Stations *Station)
{
    if (read(socketDescriptor, Station, sizeof(struct Train_Stations)) <= 0) {
        perror("[Client] Eroare la read() de la server (dimensiune structura).\n");
        return;
    }

}

void printNextHourArrivals(struct Train_Stations station)
{
 
      time_t current_time;
     struct tm *timeinfo;

     time(&current_time);
     timeinfo=localtime(&current_time);

     char ora_curenta[6];
     snprintf(ora_curenta, sizeof(ora_curenta),"%02d:%02d",timeinfo->tm_hour, timeinfo->tm_min);

     char ora_limita[6];
     snprintf(ora_limita, sizeof(ora_curenta),"%02d:%02d",timeinfo->tm_hour+1, timeinfo->tm_min);

  printf("\n                            \033[1;31m Trenurile care sosec in intervalul orar \033[1;32m%s - %s\033[0m \033[1;31m pentru %s \033[0m\n",ora_curenta,ora_limita,station.stationName);
 printf("\n\033[1;34m_________________________________________________________________________________________________________________________________________\033[0m");
  printf("\n\033[1;34m|                      |                 |                      |                        |             |               |                |\033[0m");
  printf("\n\033[1;34m|     TRAIN NUMBER     |     STATION     |     ARRIVAL_TIME     |     DEPARTURE_TIME     |    DELAY    |    A_UPDATE   |    D_UPDATE    |\033[0m");
  printf("\n\033[1;34m|______________________|_________________|______________________|________________________|_____________|_______________|________________|\033[0m");


   
    for(int i=0;i<30;i++)
     {
        if(strcmp(station.DataBase[i].arrivalTime,ora_curenta)>=0 && strcmp(station.DataBase[i].arrivalTime,ora_limita)<=0){
            printf("\n|                      |                 |                      |                        |             |               |                |");
           printf("\n|        %s          |     %s    |         %s        |          %s         |     \033[1;31m%s\033[0m     |      \033[1;32m%s\033[0m    |      \033[1;32m%s\033[0m     |",station.DataBase[i].trainNumber,station.stationName,station.DataBase[i].arrivalTime,station.DataBase[i].departureTime,station.DataBase[i].delay,station.DataBase[i].ArrivalUpdate,station.DataBase[i].DepartureUpdate);
           printf("\n|______________________|_________________|______________________|________________________|_____________|_______________|________________|");
      }
     }

  printf("\n");
  printf("\n");
  printf("\n");
}

void printNextHourDepartures(struct Train_Stations station){
  {


      time_t current_time;
     struct tm *timeinfo;

     time(&current_time);
     timeinfo=localtime(&current_time);

     char ora_curenta[6];
     snprintf(ora_curenta, sizeof(ora_curenta),"%02d:%02d",timeinfo->tm_hour, timeinfo->tm_min);

     char ora_limita[6];
     snprintf(ora_limita, sizeof(ora_curenta),"%02d:%02d",timeinfo->tm_hour+1, timeinfo->tm_min);
   
     printf("\n                         \033[1;31m Trenurile care pleaca in intervalul orar \033[1;32m%s - %s\033[0m \033[1;31m pentru %s \033[0m\n",ora_curenta,ora_limita,station.stationName);
 printf("\n\033[1;34m_________________________________________________________________________________________________________________________________________\033[0m");
  printf("\n\033[1;34m|                      |                 |                      |                        |             |               |                |\033[0m");
  printf("\n\033[1;34m|     TRAIN NUMBER     |     STATION     |     ARRIVAL_TIME     |     DEPARTURE_TIME     |    DELAY    |    A_UPDATE   |    D_UPDATE    |\033[0m");
  printf("\n\033[1;34m|______________________|_________________|______________________|________________________|_____________|_______________|________________|\033[0m");


    for(int i=0;i<30;i++)
     {
        if(strcmp(station.DataBase[i].departureTime,ora_curenta)>=0 && strcmp(station.DataBase[i].departureTime,ora_limita)<=0){
           printf("\n|                      |                 |                      |                        |             |               |                |");
          printf("\n|        %s          |     %s    |         %s        |          %s         |     \033[1;31m%s\033[0m     |      \033[1;32m%s\033[0m    |      \033[1;32m%s\033[0m     |",station.DataBase[i].trainNumber,station.stationName,station.DataBase[i].arrivalTime,station.DataBase[i].departureTime,station.DataBase[i].delay,station.DataBase[i].ArrivalUpdate,station.DataBase[i].DepartureUpdate);
               printf("\n|______________________|_________________|______________________|________________________|_____________|_______________|________________|");
      }
     }

  printf("\n");
  printf("\n");
  printf("\n");
}
}

void printBilet (struct Train_Stations station){

    char bilet[30];
    printf("\n\n\033[1;32m--> Transmiteti numarul trenului pentru care doriti sa cumparati bilet:  \033[0m");
    scanf("%s",bilet);

    for(int i=0;i<30;i++)
    {
      if(strcmp(station.DataBase[i].trainNumber,bilet)==0)
      {
        printf("\n\n");
        printf("\033[1;34m______________________________________________________________\033[0m\n");
        printf("\033[1;34m|                                                            |\033[0m\n");
        printf("\033[1;34m|  \033[1;31mTRAIN TICKET\033[0m                                             \033[1;34m |\033[0m\n");
                       printf("\033[1;34m|____________________________________________________________|\033[0m\n");
                       printf("\033[1;34m|                                                            |\033[0m\n");
        printf("\033[1;34m|\033[0m Train Number: \033[1;34m%s                                   \033[1;32mQR\033[0m   \033[1;34m |\033[0m\n",station.DataBase[i].trainNumber);
         printf("\033[1;34m|\033[0m Arrival Time: \033[1;34m%s                              \033[1;32m\u25A0\u25A0\u25A1\u25A0\u25A1\u25A0\u25A1\u25A0  \033[1;34m|\033[0m\n",station.DataBase[i].arrivalTime);
         printf("\033[1;34m|\033[0m Departure Time: \033[1;34m%s                            \033[1;32m\u25A1\u25A1\u25A0\u25A1\u25A0\u25A0\u25A1\u25A0  \033[1;34m|\033[0m\n",station.DataBase[i].departureTime);
       printf("\033[1;34m|\033[0m Delay: \033[1;34m%s                                       \033[1;32m\u25A1\u25A0\u25A0\u25A1\u25A1\u25A0\u25A0\u25A1  \033[1;34m|\033[0m\n",station.DataBase[i].delay);
         printf("\033[1;34m|\033[0m Arrival Update: \033[1;34m%s                            \033[1;32m\u25A0\u25A1\u25A0\u25A0\u25A0\u25A1\u25A0\u25A1  \033[1;34m|\033[0m\n",station.DataBase[i].ArrivalUpdate);
                       printf("\033[1;34m|____________________________________________________________|\033[0m\n");
        printf("\n\n");
    }  
}
}

void UpdateSending(int update, struct Train_Stations *station ){

   char tren[5];
    int comanda=update;
    int min=comanda%100;
    int tr=comanda/100;
    sprintf(tren,"%d",tr);

    struct tm timp={};
    char d[4];
    sprintf(d, "%+02d", min);

    for(int i=0;i<30;i++)
    {
        if(strcmp(station->DataBase[i].trainNumber,tren)==0)
         {
                       strcpy(station->DataBase[i].delay,d);

                       char ora[6];
                       sscanf(station->DataBase[i].arrivalTime,"%d:%d",&timp.tm_hour, &timp.tm_min);

                       timp.tm_min+=min;
                       timp.tm_hour += timp.tm_min/60;
                       timp.tm_min %=60;
                       if(timp.tm_hour==24)
                        timp.tm_hour=00;

                       snprintf(ora, sizeof(ora),"%02d:%02d",timp.tm_hour, timp.tm_min);

                       strcpy(station->DataBase[i].ArrivalUpdate,ora);
                        if(strcmp(station->DataBase[i].ArrivalUpdate,station->DataBase[i].departureTime)>0)
                                {strcpy(station->DataBase[i].DepartureUpdate, station->DataBase[i].ArrivalUpdate);}
                       else if(strcmp(station->DataBase[i].ArrivalUpdate,station->DataBase[i].departureTime)<=0)
                                {strcpy(station->DataBase[i].DepartureUpdate, station->DataBase[i].departureTime);}
         }
    }

}