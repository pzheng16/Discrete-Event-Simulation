/*Author          : Peng Zheng, Pin Lyu
Created         : Oct 16th, 2019
Last Modified   : Nov 6th, 2019
Affiliation          : Georgia Institute of Technology
Description: appliction part of DES (discrete event simulation).
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "ReadEngineApplication.h"

// Define different Event Types: Generator QueueStation Exit
#define	G     0
#define Q     1
#define	E     2

// Define lists of exit_station ID, and last Generater ID
int* exit_station;
int G_index;

// Information of customers entering the queues
int ID = 0;
int Enter_Customers = 0;
int Exit_Customers = 0;
double Total_remaining_time = 0.0;
double Max_remaining_time = 0.0;
double Min_remaining_time = INFINITY;
double Total_waiting_time = 0.0;
double Max_waiting_time = 0.0;
double Min_waiting_time = INFINITY;

// Function prototypes for the Application
void EventHandler (void *data, void* All_prob, double* service_time, struct EventData** customers, double** Each_Station);
void Arrival (struct EventData *e, void* All_prob, double* service_time, struct EventData** customers);		// car arrival event
void Departure (struct EventData *e);	// car departure event
void Waiting_for_Service(struct EventData* e, void* All_prob, double* service_time, struct EventData** customers, double** Each_Station);
void free_config(struct Prob** pb,int num_components);
void free_queues(struct EventData** eve, int num_components);

int main(int argv, char** argc) {
    // command arguments
    if (argv != 4)
    {
        printf("command arguments invalid!\n");
        return -1;
    }
    double running_time = atof(argc[1]);
    char* input_file_name = argc[2];
    char* output_file_name = argc[3];
    FILE *fRead = fopen(input_file_name,"r");
    if (fRead == NULL)
    {
        printf("File cannot be opened.\n");
        return -1;
    }

    // create data for storing information of DES
    int num_components;
    fscanf(fRead,"%d",&num_components);
    struct Prob* All_prob[num_components];
    double service_time[num_components];
    exit_station = (int*) malloc(sizeof(int)*num_components);//exit_station[i] = 0 means it is not an exit statioin
    
    //Initialize the header of Queues
    for (int i = 0; i < num_components; ++i)
    {
        if ((All_prob[i] = (struct Prob*)malloc (sizeof(struct Prob))) == NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
        All_prob[i] -> prob[0] = 0.;
        All_prob[i] -> prob[1] = 0.;
        All_prob[i] -> stat = i;
        All_prob[i] -> next = NULL;
        exit_station[i] = 0;
    }
    //read configuration files
    int feedback = readConfigFile(fRead,All_prob,service_time,num_components,exit_station,&G_index);
    // if the function returns -1, there are some errors with the configuration files
    if (feedback == -1) return -1;
    fclose(fRead);

    srand((unsigned)time(NULL));

    //Start first event
    struct EventData *first_Event;
    struct EventData* customers[num_components];// customers for storing every station's customers who are waiting for service by using queues
    for (int i = 0; i < num_components; ++i) {
        customers[i] = (struct EventData*) malloc(sizeof(struct EventData));
        customers[i] -> next = NULL;
    }
    // Initialize the queue of each station to store customers who enter the station.
    // Using two-dimensional array. Num of stations is the row and two columns stores
    //    the number of customers enter and their waiting time in the station.
    double** Each_Station = (double**)malloc(sizeof(double*) * num_components);
    for(int i = 0; i < num_components; i++){
        Each_Station[i] = (double*)malloc(sizeof(double) * 2);
        for(int j = 0; j < 2; j++){
            Each_Station[i][j] = 0.0;
        }
    }
    // Generators create events at the beginning
    for(int i = 0; i <= G_index; ++i){
        if ((first_Event=malloc (sizeof(struct EventData))) == NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
        first_Event -> EventType = G;
        first_Event -> Station = 0;
        first_Event -> next = NULL;
        first_Event -> eventID = ID;
        first_Event -> arrival_time = service_time[i] * randexp();
        first_Event -> enqueue_time = first_Event -> arrival_time;
        first_Event -> waiting_time = 0.0;
        first_Event -> generator = i;
        ID++;
        Enter_Customers ++;
        Schedule(first_Event -> arrival_time, first_Event);
    }

    // Run the simulation
    RunSim(running_time, All_prob, service_time, customers, Each_Station);

    // Minus customers who are scheduled into FEL after the running_time.
    Enter_Customers -= (G_index + 1);

    // Write important statistics into output files; at the same time, print those out on the screen
    FILE *fWrite = fopen(output_file_name,"w");

    // print out Question (1), (2).
    printf("Total number of customers enter the system : %d\n"
                   "Total number of customers exit the system : %d\nAverage remaining time of customers : %.2f\n"
                   "Maximum remaining time of customers : %.2f\nMinimum remaining time of customers : %.2f\n", Enter_Customers,Exit_Customers,
            Total_remaining_time / (double)Exit_Customers,Max_remaining_time,Min_remaining_time);

    fprintf(fWrite,"Total number of customers enter the system : %d\n"
                   "Total number of customers exit the system : %d\nAverage remaining time of customers : %.2f\n"
                   "Maximum remaining time of customers : %.2f\nMinimum remaining time of customers : %.2f\n", Enter_Customers,Exit_Customers,
                   Total_remaining_time / (double)Exit_Customers,Max_remaining_time,Min_remaining_time);

    // Consider those who are in the service and have not exit the service.
    // Count the total waiting time of them and combine them with customers who have
    // exited to figure out the avg, max, min waiting time of the simulation.
    struct EventData* tmp;
    for(int i = G_index + 1; i < num_components - 1; i++){
        tmp = customers[i];
        if(tmp -> next == NULL)
            continue;
        else{
            tmp = tmp -> next;
            while(tmp != NULL){
                tmp -> waiting_time += CurrentTime() - tmp -> enqueue_time;
                Total_waiting_time += tmp -> waiting_time;
                if(Max_waiting_time < tmp -> waiting_time){
                    Max_waiting_time = tmp -> waiting_time;
                }
                if(Min_waiting_time > tmp -> waiting_time){
                    Min_waiting_time = tmp -> waiting_time;
                }
                tmp = tmp -> next;
            }
        }
        free(tmp);
    }


    // print out Question (3).
    printf("Average waiting time of customers : %.2f\nMaximum waiting time of customers : %.2f\n"
           "Minimum waiting time of customers : %.2f\n", Total_waiting_time / (double)Enter_Customers,Max_waiting_time,fabs(Min_waiting_time));
    fprintf(fWrite,"Average waiting time of customers : %.2f\nMaximum waiting time of customers : %.2f\n"
           "Minimum waiting time of customers : %.2f\n", Total_waiting_time / (double)Enter_Customers,Max_waiting_time,fabs(Min_waiting_time));

    // print out Question (4).
    for(int i = G_index + 1; i < num_components - 1; i++){
        printf("Average waiting time of Station %d : %.2f\n", i, fabs(Each_Station[i][1] / Each_Station[i][0]));
        fprintf(fWrite,"Average waiting time of Station %d : %.2f\n", i, fabs(Each_Station[i][1] / Each_Station[i][0]));
    }

    fclose(fWrite);

    // Free Memory
     for(int i = 0; i < num_components; i++)
    {
        free(Each_Station[i]);
    }
    free(Each_Station);
    free(exit_station);
    free_config(All_prob,num_components);
    free_queues(customers,num_components);

    return 0;
}

// General Event Handler Procedure define in simulation engine interface
// This function is called by the simulation engine to process an event removed from the future event list
void EventHandler (void *data, void* All_prob, double* service_time, struct EventData** customers, double** Each_Station)
{
    struct EventData *d;
    // coerce type so the compiler knows the type of information pointed to by the parameter data.
    d = (struct EventData *) data;
    // call an event handler based on the type of event
    if (d->EventType == G) Arrival (d, All_prob, service_time, customers);
    else if(d->EventType == Q) Waiting_for_Service(d, All_prob, service_time, customers, Each_Station);
    else if (d->EventType == E) Departure (d);
    else {fprintf (stderr, "Illegal event found\n"); exit(1); }
    free (d); // Release memory for event paramters
}

// event handler for arrival events
void Arrival (struct EventData *e, void* All_prob, double* service_time, struct EventData** customers) {
//    printf("Processing Arrival event(%d) at time %f at station %d\t",e->eventID, CurrentTime(),e -> Station);
    if (e->EventType != G) {
        fprintf(stderr, "Unexpected event type\n");
        exit(1);
    }

    // Arrival customers are scheduled to next station
    e -> next = NULL;
    e -> Station = choose_station_Q(All_prob, e -> Station);
    e -> EventType = Q;

    // Arrival Events scheduled to FEL if the station that the customer entered is empty.
    struct EventData* FEL_event;
    if((FEL_event=malloc(sizeof(struct EventData)))==NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
    FEL_event-> eventID = e -> eventID;
    FEL_event -> Station = e -> Station;
    FEL_event -> EventType = e -> EventType;
    FEL_event -> arrival_time = e -> arrival_time;
    FEL_event -> enqueue_time = CurrentTime();
    FEL_event -> service_time = service_time[FEL_event -> Station];
    FEL_event -> waiting_time = e -> waiting_time;
    FEL_event -> generator = e -> generator;
    FEL_event -> next = NULL;
    if (customers[e->Station] -> next == NULL)
    {
        Schedule(CurrentTime() + FEL_event -> service_time, FEL_event);
    }

    // Adding the customer to the queue of its station.
    struct EventData* queue_event = malloc(sizeof(struct EventData));
    queue_event -> eventID = FEL_event -> eventID;
    queue_event -> Station = FEL_event -> Station;
    queue_event -> EventType = FEL_event -> EventType;
    queue_event -> arrival_time = FEL_event -> arrival_time;
    queue_event -> enqueue_time = FEL_event -> enqueue_time;
    queue_event -> service_time = FEL_event -> service_time;
    queue_event -> waiting_time = FEL_event -> waiting_time;
    queue_event -> generator = FEL_event -> generator;
    queue_event -> next = NULL;

    // If didn't schedule, free the memory.
    if (customers[e->Station] -> next != NULL)
    {
        free(FEL_event);
    }

    struct EventData *find_last;
    for(find_last = customers[queue_event -> Station]; find_last -> next != NULL; find_last = find_last -> next){}
    find_last -> next = queue_event;

    // New events added from generators
    struct EventData *FEL_event2;
    if((FEL_event2=malloc(sizeof(struct EventData)))==NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
    FEL_event2 -> EventType = G;
    FEL_event2 -> Station = 0;
    FEL_event2 -> eventID = ID;
    FEL_event2 -> next = NULL;
    FEL_event2 -> arrival_time = CurrentTime() + service_time[e -> generator] * randexp();
    FEL_event2 -> waiting_time = 0.0;
    FEL_event2 -> generator = e -> generator;
    ID++;
    Schedule (FEL_event2 -> arrival_time, FEL_event2);

    Enter_Customers ++;
}

// event handler for waiting-for-service events
void Waiting_for_Service(struct EventData* e, void* All_prob, double* service_time, struct EventData** customers, double** Each_Station)
{
//    printf("Processing Servicing event(%d) at time %f at station %d\t", e->eventID,CurrentTime(),e -> Station);
    int station = e -> Station;
    struct EventData* FEL_event;
    if((FEL_event=malloc(sizeof(struct EventData)))==NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
    struct EventData* finished_event;
    // first one waiting in the queue(but haven't received service) pops out and it is scheduled to next station after the service time
    if(customers[station] -> next -> next != NULL){
        finished_event = customers[station] -> next;
        customers[station] -> next = customers[station] -> next -> next;
        struct EventData* tmp = customers[station] -> next;
        FEL_event -> Station = tmp -> Station;
        FEL_event -> eventID = tmp -> eventID;
        FEL_event -> arrival_time = tmp -> arrival_time;
        FEL_event -> EventType = Q;
        FEL_event -> next = NULL;
        FEL_event -> service_time = tmp -> service_time;
        FEL_event -> waiting_time = tmp -> waiting_time;
        FEL_event -> enqueue_time = tmp -> enqueue_time;
        FEL_event -> generator = tmp -> generator;
        Schedule(CurrentTime() + FEL_event -> service_time, FEL_event);
    }
    else{
        finished_event = customers[station] -> next;
        customers[station]->next = NULL;
        free(FEL_event);
    }
    free(finished_event);

    // Update the queue information of Each_Station after one customer exit the station.
    Each_Station[e -> Station][0] += 1.0;
    Each_Station[e -> Station][1] += CurrentTime() - e -> enqueue_time - e -> service_time;

    // Customer who had received service will be scheduled to next station
    struct EventData *FEL_event2;
    if((FEL_event2=malloc(sizeof(struct EventData)))==NULL) {fprintf(stderr, "malloc error\n"); exit(1);}

    FEL_event2 -> Station = choose_station_Q(All_prob, e -> Station);
    FEL_event2 -> eventID = e -> eventID;
    FEL_event2 -> next = NULL;
    FEL_event2 -> arrival_time = e -> arrival_time;
    FEL_event2 -> enqueue_time = CurrentTime();
    FEL_event2 -> service_time = service_time[FEL_event2 -> Station] * randexp();
    FEL_event2 -> waiting_time = e -> waiting_time + CurrentTime() - e -> enqueue_time - e -> service_time;
    FEL_event2 -> generator = e -> generator;

    // if the station is the exit, then finish scheduling
    if (exit_station[FEL_event2 -> Station] == 1){
        FEL_event2 -> EventType = E;
        Schedule(CurrentTime(), FEL_event2);
    }
    else{
        FEL_event2 -> EventType = Q;
        if(customers[FEL_event2 -> Station]-> next == NULL){
            Schedule(CurrentTime() + FEL_event2 -> service_time, FEL_event2);
        }
        // Added to queues of stations
        struct EventData* queue_event;
        if((queue_event=malloc(sizeof(struct EventData)))==NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
        queue_event -> eventID = FEL_event2 -> eventID;
        queue_event -> Station = FEL_event2 -> Station;
        queue_event -> EventType = FEL_event2 -> EventType;
        queue_event -> arrival_time = FEL_event2 -> arrival_time;
        queue_event -> enqueue_time = FEL_event2 -> enqueue_time;
        queue_event -> waiting_time = FEL_event2 -> waiting_time;
        queue_event -> service_time = FEL_event2 -> service_time;
        queue_event -> generator = FEL_event2 -> generator;
        queue_event -> next = NULL;

        if(customers[FEL_event2 -> Station]-> next != NULL){
            free(FEL_event2);
        }

        struct EventData* find_last;
        for(find_last = customers[queue_event -> Station]; find_last -> next != NULL; find_last = find_last -> next){}
        find_last -> next = queue_event;
    }
}

// event handler for departure events
void Departure (struct EventData *e)
{
//    printf("Processing departure event(%d) at time %f at station %d\t",e->eventID, CurrentTime(),e -> Station);
//    if (e->EventType != E) {fprintf (stderr, "Unexpected event type\n"); exit(1);}

    // calculate statistics related to time
    Total_remaining_time += CurrentTime() - e -> arrival_time;

    if(Max_remaining_time < (CurrentTime() - e -> arrival_time)){
        Max_remaining_time = CurrentTime() - e -> arrival_time;
    }
    if(Min_remaining_time > (CurrentTime() - e -> arrival_time)){
        Min_remaining_time = CurrentTime() - e -> arrival_time;
    }

    Total_waiting_time += e -> waiting_time;

    if(Max_waiting_time < e -> waiting_time){
        Max_waiting_time = e -> waiting_time;
    }
    if(Min_waiting_time > e -> waiting_time){
        Min_waiting_time = e -> waiting_time;
    }

    Exit_Customers ++;
    // If another vehicle waiting to use pump, allocate pump to vehicle, schedule its departure event
}

// free memory storing configuration files
void free_config(struct Prob** pb,int num_components)
{
    for (int i = 0; i < num_components ; ++i) {
        struct Prob* tmp = pb[i];
        for(tmp;tmp != NULL;)
        {
            struct Prob* del = tmp;
            tmp = tmp -> next;
            free(del);
        }
    }
    return;
}

//free memory storing queues of stations
void free_queues(struct EventData** eve, int num_components)
{
    for (int i = 0; i < num_components ; ++i) {
        struct EventData* tmp = eve[i];
        for(tmp;tmp != NULL;)
        {
            struct EventData* del = tmp;
            tmp = tmp -> next;
            free(del);
        }
    }
    return;
}