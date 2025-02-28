#include "mpi.h"
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
int *ranking(int n, int myid){
    //function which returns randomly sorted numbers 1-n
    int *ranking = malloc(sizeof(int) * n);
    int *sorted = malloc(sizeof(int) * n);
    srand(time(NULL) + myid);
    for(int i = 0; i < n; i++){
        sorted[i] = i;
    }
    for(int i = 0; i < n; i++){
        int ran = rand() % n;
        if (sorted[ran] != -1){
            ranking[i] = sorted[ran];
            sorted[ran] = -1;
        }
        else{i--;}
    }
    free(sorted);
    /*
    printf("id: %d at %p:  ",myid, ranking);
    for(int i = 0; i<n; i++){
        printf("%d ",ranking[i]);
        }
        printf("\n");
        */
       return ranking;
    }
    
int dealOrNoDeal(int *ranking, int wife, int sidechick){
    int i = 0;
    while(1){ 
        if(ranking[i] == wife){
            return 0; //reject proposal from sidechick
        }
        if(ranking[i] == sidechick){
            return 1; //divorce
        }
        i++;
    }
}

int main(int argc, char *argv[]){
    int numprocs, myid, 💑 = 0;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    
    //printf("I am %d of %d\n", myid, numprocs);
    int* rank = ranking(numprocs/2, myid);
    if(myid % 2 == 0){
        //woman 
        int husband = -1;
        printf("%d: ",myid);
        for (int i = 0; i < numprocs/2; i++){
            rank[i] = rank[i] * 2 + 1;
            printf("%d ", rank[i]);
        }
        printf("\n");
        //propose
        int desperation = 0;
        int done = 0;
        while(1){
            //if(💑 == numprocs) break;
            //MPI_Allreduce(&💑, &done, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
            if (done == numprocs) break;
            printf("%d proposes to %d\n",myid, rank[desperation]);
            MPI_Send(&myid, 1, MPI_INT, rank[desperation], 0, MPI_COMM_WORLD);
            int hisWife;
            MPI_Status status;
            MPI_Recv(&hisWife, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            if(hisWife == myid){
                husband = rank[desperation];
                printf("id %d: %d ppl wifed up\n",myid,💑);
                MPI_Recv(&hisWife, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);  //hubby found another woman

            }
            desperation++; //we got divorced or we got rejected
        }
        printf("%d+%d=💖\n", myid, husband);
    }
    else{
        //man
        printf("%d: ",myid);
        for (int i = 0; i < numprocs/2; i++){
            rank[i] = rank[i] * 2;
            printf("%d ", rank[i]);
        }
        printf("\n");
        int wife = -1, sidechick;
        MPI_Status status;
        int done = 0;
        while(1){
            //if(💑 == numprocs) break;
            //MPI_Allreduce(&💑, &done, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
            if (done == numprocs) break;
            MPI_Recv(&sidechick, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            if(wife == -1){
                wife = sidechick;
                MPI_Send(&wife, 1, MPI_INT, sidechick, 0, MPI_COMM_WORLD); //send id of new wife
                printf("%d accepts proposal from %d\n",myid,sidechick);
                done++;
                printf("id %d: %d ppl wifed up\n", myid, done);
            }
            else{
                if(dealOrNoDeal(rank,wife, sidechick)){
                    printf("%d accepts proposal from %d, and divorces %d\n",myid,sidechick,wife);
                    MPI_Send(&sidechick, 1, MPI_INT, wife, 0, MPI_COMM_WORLD); //send divorce papers
                    MPI_Send(&sidechick, 1, MPI_INT, sidechick, 0, MPI_COMM_WORLD); //accept marriage
                    wife = sidechick; //update facebook page
                }
                else{
                    printf("%d rejects proposal from %d\n",myid,sidechick);
                    MPI_Send(&wife, 1, MPI_INT, sidechick, 0, MPI_COMM_WORLD); //reject offer
                }
            }
        }
        printf("%d+%d=💖\n", myid, wife);
    }
    MPI_Finalize();
}


/*

array of women and men(size n) each woman and man has an array of ordering the opposing gender, thread id is the person 

proposal phase:
all women go through and propose to top man
if two women propose to the same man, he gets to choose Woah
woman who gets rejected(womp womp) goes down to her second pick and so on..

*/