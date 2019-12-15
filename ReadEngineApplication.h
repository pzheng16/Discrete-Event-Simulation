/*Author          : Peng Zheng, Pin Lyu
Created         : Oct 16th, 2019
Last Modified   : Nov 6th, 2019
Affiliation          : Georgia Institute of Technology
Description: header files for readfile,engine and application of DES(discrete event simulation).
*/

#ifndef HW6FINAL_READENGINEAPPLICATION_H
#define HW6FINAL_READENGINEAPPLICATION_H

#include <stdio.h>

// EventData Structures for getting statistics of time
struct EventData {
    int EventType;
    int Station;
    int eventID;
    struct EventData* next;
    double arrival_time;
    double service_time;
    double enqueue_time;
    double waiting_time;
    int generator;
};

// Structures for storing configuration files including destinations, probabilities
struct Prob {
    int stat;
    double prob[2];//prob belongs to [prb[0],prob[1])
    struct Prob* next;//next prob
};


// Call this procedure to run the simulation indicating time to end simulation
void RunSim (double EndTime, void* All_prob, double* service_time, struct EventData** customers, double** Each_Station);

// Schedule an event with timestamp ts, event parameters *data
void Schedule (double ts, void *data);

// This function returns the current simulation time
double CurrentTime (void);

//
// Function defined in the simulation application called by the simulation engine
//
//  Event handler function: called to process an event
void EventHandler (void *data, void* All_prob, double* service_time, struct EventData** customers, double** Each_Station);

// Read Configuration files and store it in struct Prob
int readConfigFile(FILE* fRead,struct Prob** All_prob,double* service_time,int num_components,int* exit_station,int* G_index);

// According to configuration files and current station, choose next station for service
int choose_station_Q(struct Prob** pb,int cur_station);

// urand() return the value [0,1)
double rand_P();

// Generate random numbers from an exponential distribution with mean U
double randexp();

#endif //HW6FINAL_READENGINEAPPLICATION_H
