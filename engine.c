/*Author          : Peng Zheng, Pin Lyu
Created         : Oct 16th, 2019
Last Modified   : Nov 6th, 2019
Affiliation          : Georgia Institute of Technology
Description: general engine part of DES (discrete event simulation).
*/

#include <stdio.h>
#include <stdlib.h>
#include "ReadEngineApplication.h"

// Structure for Events in FEL
struct Event {
    double timestamp;// event timestamp
    void *AppData;			// pointer to application defined event parameters
    struct Event *Next;		// priority queue pointer
};

// Simulation clock variable
double Now = 0.0;

// Future Event List
struct Event FEL ={-1.0, NULL, NULL};

// Function to print timestamps of events in event list
// we haven't used PrintList for printing out information, 
//but PrintList() can be a useful function for users to fetch more detailed information
void PrintList (void);

// Function to remove smallest timestamped event
struct Event *Remove (void);

// Remove smallest timestamped event from FEL, return pointer to this event
// return NULL if FEL is empty
struct Event *Remove (void)
{
    struct Event *e;

    if (FEL.Next==NULL) return (NULL);
    e = FEL.Next;		// remove first event in list
    FEL.Next = e->Next;
    return (e);
}

// Print timestamps of all events in the event list (used for debugging)
void PrintList (void)
{
    struct Event *p;

    printf ("Event List: ");
    for (p=FEL.Next; p!=NULL; p=p->Next) {
        printf ("%f ", p->timestamp);
    }
    printf ("\n");
}

// Return current simulation time
double CurrentTime (void)
{
    return (Now);
}

// Schedule new event in FEL
// queue is implemented as a timestamp ordered linear list
void Schedule (double ts, void *data)
{
    struct Event *e, *p, *q;

    // create event data structure and fill it in
    if ((e = malloc (sizeof (struct Event))) == NULL) exit(1);
    e->timestamp = ts;
    e->AppData = data;

    // insert into priority queue
    // p is lead pointer, q is trailer
    for (q=&FEL, p=FEL.Next; p!=NULL; p=p->Next, q=q->Next) {
        if (p->timestamp >= e->timestamp) break;
    }
    // insert after q (before p)
    e->Next = q->Next;
    q->Next = e;
}

// Function to execute simulation up to a specified time (EndTime)
void RunSim (double EndTime, void* All_prob, double* service_time, struct EventData** customers, double** Each_Station)
{
    struct Event *e;
//    printf ("Initial event list:\n");
//    PrintList ();

    // Main scheduler loop
    while ((e=Remove()) != NULL) {
        Now = e->timestamp;
        if (Now > EndTime) 
        {
            free(e -> AppData);
            free(e);
            while((e=Remove()) != NULL)
            {
                free(e -> AppData);
                free(e);
            }
            break;
        }
        EventHandler(e->AppData, All_prob, service_time, customers, Each_Station);
        free (e);	// it is up to the event handler to free memory for parameters
//      PrintList ();// PrintList() is up to the user to use or not
    }
}
