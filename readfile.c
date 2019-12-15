/*Author          : Perkz Zheng
Created         : Oct 16th, 2019
Last Modified   : Nov 6th, 2019
Affiliation          : Georgia Institute of Technology
Description: reading-configuration-files for DES (discrete event simulation).
*/

#include "ReadEngineApplication.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Read configuration files
int readConfigFile(FILE* fRead,struct Prob** All_prob,double* service_time,int num_components,int* exit_station,int* G_index)
{
    // When if statements have "//**//" on the right of it,
    // if statements are used to check the validation of configuration files
    // has_G_Q_E is used to check whether configuration files have at one Generator, Queue(Station), Exit(Station);
    // e.g. if it has one G, then has_G_Q_E[0] = 1; has one Q, has_G_Q_E[1] = 1; has one E, has_G_Q_E[2] = 1;
    int has_G_Q_E[3] = {0,0,0};//**//
    int direct;
    int station_ID;
    char Type;
    *G_index = 0;
    for (int i = 0; i < num_components; ++i)
    {
        station_ID = -1;
        fscanf(fRead,"%d %c",&station_ID,&Type);

        if (station_ID == -1)//**//
        {// check whether configuration files are valid or not
            printf("Wrong input of Event Type or (not enough or extra) destinations of Prob or ID in one Q!!\n");
            return -1;// errors happen when inputting configuration files: Type invalid or (not enough or extra) destinations in last Q.
        }

        if (Type == 'Q' && All_prob[station_ID] -> next != NULL)
        {// check whether configuration files are valid or not
            printf("Station ID already existed!! \n");
            return -1;// errors happen when inputting configuration files: Station ID already existed.
        }
        double average_time;

        if (Type != 'Q' && Type != 'G' && Type != 'E')//**//
        {// check whether configuration files are valid or not
            printf("Wrong input of Event Type or extra destinations of Prob or ID in one Q!!\n");
            return -1;// errors happen when inputting configuration files: Type invalid or extra destinations of Prob or ID in one Q.
        }

        if(Type != 'E')
        {
            if(Type == 'G')
            {
                has_G_Q_E[0] = 1;
            }
            else has_G_Q_E[1] = 1;
            All_prob[station_ID]-> stat = station_ID;
            fscanf(fRead,"%lf %d",&average_time,&direct);
            service_time[station_ID] =  average_time;

        }
        else
        {
            has_G_Q_E[2] = 1;
            exit_station[station_ID] = 1;
        }
        double last_prob = 0.;
        if (Type == 'G')
        {
            *G_index = station_ID;
            struct Prob* Gen;
            if ((Gen = (struct Prob*)malloc (sizeof(struct Prob))) == NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
            Gen -> stat = direct;
            Gen -> prob[0] = 0.;
            Gen -> prob[1] = 1.;
            Gen -> next = NULL;
            struct Prob* tmp;
            for (tmp = (struct Prob *) All_prob[station_ID]; tmp -> next != NULL; tmp = tmp -> next){}
            tmp-> next = Gen;
        }
        else if (Type == 'Q')
        {
            struct Prob* last = All_prob[station_ID];
            for (int j = 0; j < direct; ++j) {
                double cur_prob;
                struct Prob* goal;
                if ((goal = (struct Prob*)malloc (sizeof(struct Prob))) == NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
                fscanf(fRead,"%lf",&cur_prob);//**//
                goal -> prob[0] = last_prob;
                last_prob += cur_prob;
                if (j == direct -1 && last_prob > 1.0)//**//
                {// check whether configuration files are valid or not
                    printf("Wrong input of Probabilities: the sum != 1!!\n");
                    return -1;// errors happen when inputting configuration files: sum of probabilities != 1;
                }
                goal -> prob[1] = last_prob;
                goal -> next = NULL;
                last -> next = goal;
                last = goal;
            }

            int next_stat = - 1;
            struct Prob* cur = All_prob[station_ID] -> next;
            for (int j = 0; j < direct; ++j) {
                fscanf(fRead,"%d",&next_stat);
                if (next_stat > num_components -1 || next_stat == -1)//**//
                {// check whether configuration files are valid or not
                    printf("Invalid Station ID or wrong number of destinations of Prob or ID!!\n");
                    return -1;// errors happen when inputting configuration files: no such station ID;
                }
                cur -> stat = next_stat;
                cur = cur -> next;
            }

        }
    }

    if (has_G_Q_E[0] == 0 || has_G_Q_E[1] == 0 || has_G_Q_E[2] == 0 )//**//
    {// check whether configuration files are valid or not
        printf("No Generator or No stations for queuing or No stations for exiting!!\n");
        return -1;// errors happen when inputting configuration files: no Generator or No stations for queuing or No stations for exiting;
    }
    return 0;
}

double rand_P()
{
    double ans = ((double) rand()/((double)RAND_MAX + 1));
    return ans;
}

double randexp()
{
    return -log(1.0-rand_P());
}

int choose_station_Q(struct Prob** pb,int cur_station)
{
    double p = rand_P();
    struct Prob* tmp = pb[cur_station] -> next;
    for (tmp; tmp != NULL; tmp = tmp -> next)
    {
        if ((tmp -> prob[0] <= p) && (p <= tmp -> prob[1]))
        {
            return tmp -> stat;
        }
    }
}
