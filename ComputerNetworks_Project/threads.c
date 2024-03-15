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
#include <pthread.h>
#include <time.h>


#define MAX_TRAINS 10000
#define PORT 2908
extern int errno;

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

typedef struct thData{
    int idThread;
    int cl;
    struct Train_Stations *station;

}thData;

typedef struct Interface{
    void (*execute)(int descriptor,int *command, struct Train_Stations *station);
}Command;

typedef struct{
    Command command;

}ConcreteCommand;

typedef struct{
    Command *command;
}Invoker;

typedef struct CommandsQueue{
    int items[30];
    int front;
    int rear;
};



/*DEFINIRE FUNCTII*/

struct Train_Stations dataRegistration(xmlNodePtr root);

struct Train_Stations readXMLFile(const char* xmlFile);

void receiveLocation(int threadDescriptor, char *stationName);

void Change_Status(struct Train_Stations *Station);

void sendTodayTrainList(int socketDescriptor, struct Train_Stations *Station);

void sendNextHourList(int threadDescriptor, struct Train_Stations Station);

void ArrivalInfoCommmand_execute(int threadDescriptor,int *command, struct Train_Stations *station);

void DepartureInfoCommand_execute(int threadDescriptor,int *command, struct Train_Stations *station);

void UpdateCommand_execute(int threadDescriptor,int *command,struct Train_Stations *station);

void receiveCommand(int threadDescriptor, struct CommandsQueue *queue);

void processCommands(int threadDescriptor, struct CommandsQueue *queue, struct Train_Stations *station);

static void *treat(void *);

void raspunde(void*);

void initializeQueue(struct CommandsQueue *queue);

int *dequeue(struct CommandsQueue *queue);




/*INT MAIN*/

int main()
{
    struct Train_Stations StationA;
    const char* xmlFile1="StationA.xml";
    StationA=readXMLFile(xmlFile1);
    strcpy(StationA.stationName,"StationA");

 
  
    struct sockaddr_in server;	// structura folosita de server
    struct sockaddr_in from;	
    
    int sd;		//descriptorul de socket 
 
    pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
    int i=0;
  
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }
 
  
    int on=1;
    setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,& on,sizeof(on));

    bzero (&server, sizeof (server));
    bzero (&from, sizeof (from));
  
    server.sin_family = AF_INET;	
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    server.sin_port = htons (PORT);
  
    if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

    if (listen (sd, 2) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }
 
    /* servim in mod concurent clientii...folosind thread-uri */
    while (1)
    {
      int client;
      thData * td; 
      int length = sizeof (from);

      printf ("[server]Asteptam la portul %d...\n",PORT);
      fflush (stdout);

      if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
        {
        perror ("[server]Eroare la accept().\n");
        continue;
        }

      td=(struct thData*)malloc(sizeof(struct thData));	
      td->idThread=i++;
      td->cl=client;
      td->station=&StationA;
        
      pthread_create(&th[i], NULL, &treat, td);	    

				
	}    
    return 0;
}



/*IMPLEMENTARE FUNCTII*/

struct Train_Stations dataRegistration(xmlNodePtr root)
{//data deserialization
    xmlNodePtr node;
    struct Train_Stations station ;
    int count=0;
    for(node=root->children; node; node=node->next){
        if(node->type == XML_ELEMENT_NODE && xmlStrcmp(node->name,BAD_CAST"train")==0){
            
            strncpy(station.DataBase[count].trainNumber,(char*)xmlGetProp(node, BAD_CAST"number"),sizeof(station.DataBase[count].trainNumber)-1);
            strncpy(station.DataBase[count].arrivalTime,(char*)xmlGetProp(node, BAD_CAST"arrivalTime"),sizeof(station.DataBase[count].arrivalTime)-1);
            strncpy(station.DataBase[count].departureTime,(char*)xmlGetProp(node, BAD_CAST"departureTime"),sizeof(station.DataBase[count].departureTime)-1);
            strncpy(station.DataBase[count].delay,(char*)xmlGetProp(node, BAD_CAST"delay"),sizeof(station.DataBase[count].delay)-1);
            strncpy(station.DataBase[count].ArrivalUpdate,(char*)xmlGetProp(node, BAD_CAST"update"),sizeof(station.DataBase[count].ArrivalUpdate)-1);
            strncpy(station.DataBase[count].DepartureUpdate,(char*)xmlGetProp(node, BAD_CAST"Dupdate"),sizeof(station.DataBase[count].DepartureUpdate)-1);
            count++;
        }
    }

    count--;
    station.trainsNumber=count;
    return station;
}

struct Train_Stations readXMLFile(const char* xmlFile){
    xmlDocPtr doc;
    xmlNodePtr root;

    doc=xmlReadFile(xmlFile,NULL,0);

    if(doc==NULL){
        perror("[ServerError] Eroare la citirea fisierului XML.\n");
    }

    root=xmlDocGetRootElement(doc);

    if(xmlStrcmp(root->name, (const xmlChar*)"trains")!=0){
        perror("[ServerError] Fisierul XML nu contine date referitoare la trenuri.\n");
        xmlFreeDoc(doc);
    }

    struct Train_Stations station=dataRegistration(root);
  
    xmlFreeDoc(doc);
    xmlCleanupParser();

    return station;
}

void receiveLocation(int threadDescriptor, char *stationName)
{
    if(read(threadDescriptor,stationName, sizeof(stationName))<=0){
                perror ("[Server] Eroare la read() de la client.\n");
            }
    stationName[strcspn(stationName, "\n")] = '\0';

}

void Change_Status(struct Train_Stations *Station){
  
    time_t current_time;
     struct tm *timeinfo;

     time(&current_time);
     timeinfo=localtime(&current_time);

     char ora_curenta[6];
     snprintf(ora_curenta, sizeof(ora_curenta),"%02d:%02d",timeinfo->tm_hour, timeinfo->tm_min);

    for(int i=0;i<30;i++)
        if(strcmp(ora_curenta, Station->DataBase[i].departureTime)>0)
           strcpy(Station->DataBase[i].DepartureUpdate,"-left");
}

void sendTodayTrainList(int socketDescriptor, struct Train_Stations *Station)
{
    if(write(socketDescriptor,Station,sizeof(struct Train_Stations))<=0){
        perror("[Server] Eroare la write() spre client (dimensiune structura).\n");
    }    
   fsync(socketDescriptor);
}

void sendNextHourList(int threadDescriptor, struct Train_Stations Station){

    if(write(threadDescriptor,&Station, sizeof(struct Train_Stations))<=0){
        perror("[Server] Erroare la write spre client (dimensiune structura).\n");
    }
    printf("AM trimis\n");
    fsync(threadDescriptor);
}

void initializeQueue(struct CommandsQueue *queue){
    queue->front = -1;
    queue->rear = -1;
}

int *dequeue(struct CommandsQueue *queue){
    int command;
    command=queue->items[queue->front];

    if (queue->front == queue->rear) {
        initializeQueue(queue);
    } else {
        queue->front = (queue->front + 1) % 30;
    }
    return command;
}

void ArrivalInfoCommmand_execute(int threadDescriptor,int *command, struct Train_Stations *station){
    
    int k=0;
    struct Train_Stations cerere;

     time_t current_time;
     struct tm *timeinfo;

     time(&current_time);
     timeinfo=localtime(&current_time);

     char ora_curenta[6];
     snprintf(ora_curenta, sizeof(ora_curenta),"%02d:%02d",timeinfo->tm_hour, timeinfo->tm_min);

     char ora_limita[6];
     snprintf(ora_limita, sizeof(ora_curenta),"%02d:%02d",timeinfo->tm_hour+1, timeinfo->tm_min);
   
    for(int i=0;i<30;i++)
     {
        if(strcmp(station->DataBase[i].arrivalTime,ora_curenta)>=0 && strcmp(station->DataBase[i].arrivalTime,ora_limita)<=0){
            strcpy(cerere.DataBase[k].trainNumber,station->DataBase[i].trainNumber);
            strcpy(cerere.DataBase[k++].arrivalTime,station->DataBase[i].arrivalTime);
        }
     }
     cerere.trainsNumber=k-1;

}

void DepartureInfoCommand_execute(int threadDescriptor,int *command, struct Train_Stations *station){

     int k=0;
     struct Train_Stations cerere;
    
    
     time_t current_time;
     struct tm *timeinfo;

     time(&current_time);
     timeinfo=localtime(&current_time);

     char ora_curenta[6];
     snprintf(ora_curenta, sizeof(ora_curenta),"%02d:%02d",timeinfo->tm_hour, timeinfo->tm_min);

     char ora_limita[6];
     snprintf(ora_limita, sizeof(ora_curenta),"%02d:%02d",timeinfo->tm_hour+1, timeinfo->tm_min);

     for(int i=0;i<30;i++)
     {
        if(strcmp(station->DataBase[i].departureTime,ora_curenta)>=0 && strcmp(station->DataBase[i].departureTime,ora_limita)<=0){
            strcpy(cerere.DataBase[k].trainNumber,station->DataBase[i].trainNumber);
            strcpy(cerere.DataBase[k++].departureTime,station->DataBase[i].departureTime);
        }
     }

      cerere.trainsNumber=k-1;

}

void UpdateCommand_execute(int threadDescriptor,int *command, struct Train_Stations *station){

    
    char tren[5];
    int comanda=command;
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

void receiveCommand(int threadDescriptor, struct CommandsQueue *queue)
{
    if(read(threadDescriptor,queue,sizeof(struct CommandsQueue))<=0){
        perror("[Server] Eroare la read() de la client(comenzi).\n");
        return;
    }
}

void processCommands(int threadDescriptor, struct CommandsQueue *queue,struct Train_Stations *station){
 
   while(queue->front != queue->rear){
        int command=dequeue(queue);
        if(command==1){
           ConcreteCommand concreteCommand1;
            concreteCommand1.command.execute = ArrivalInfoCommmand_execute;
            concreteCommand1.command.execute(threadDescriptor,command,station);
        
        } else if(command==2){
           ConcreteCommand concreteCommand2;
            concreteCommand2.command.execute = DepartureInfoCommand_execute;
            concreteCommand2.command.execute(threadDescriptor,command,station);
        } else if(command!=4){
            ConcreteCommand concreteCommand3;
            concreteCommand3.command.execute = UpdateCommand_execute;
            concreteCommand3.command.execute(threadDescriptor,command,station);
        }
   }
}

static void *treat(void * arg)
{		
         
		struct thData tdL; 
		tdL= *((struct thData*)arg);	
		printf ("[Thread %d] - S-a conectat un client. Asteptam locatia lui...\n", tdL.idThread);
		fflush (stdout);		 
		pthread_detach(pthread_self());		

		raspunde((struct thData*)arg);

		close ((intptr_t)arg);
		return(NULL);	
  		
};

void raspunde(void *arg)
{
	struct thData tdL; 
	tdL= *((struct thData*)arg);
    
    char buf[10];
    receiveLocation(tdL.cl,&buf);
    printf ("[Thread %d] - Locatia clientului este: %s\n",tdL.idThread, buf);
    strcpy(tdL.station->stationName,buf);

    Change_Status(tdL.station);
    
    sendTodayTrainList(tdL.cl,tdL.station);   
    printf("[Thread %d] - Mersul trenurilor din ziua curenta a fost trimis catre client.\n",tdL.idThread);

    char trash[1000];
    read(tdL.cl,trash,sizeof(trash));

    struct CommandsQueue queue;
    receiveCommand(tdL.cl,&queue);

    if(queue.front==0 && queue.rear==0)
        {printf("[Thread %d] - Clientul nu are alte cereri. Urmeaza sa se deconecteze.\n",tdL.idThread);}
    else
        {
            printf("[Thread %d] - Am primit cererile de la client, urmeaza sa le procesam...\n",tdL.idThread);
            processCommands(tdL.cl,&queue,tdL.station);
            printf("[Thread %d] - Comenzile au fost procesate si trimise. Clientul urmeaza sa se deconecteze.\n",tdL.idThread);
        }

   

 
}
