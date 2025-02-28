#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#define MAX_THREADS 8

int position_index = 0;
omp_lock_t lck;
int valid_positions[92][8];

bool check_position(int *position){
    //first check same column
    bool ItsValid = true;
    for(int i = 0; i < 8; i++){
        for(int j = i + 1; j < 8; j++){
            if(position[i] == position[j]){
                ItsValid = false;
                break;  
            } 
            int distance = abs(i - j);
            if(position[i] == position[j] + distance){
                ItsValid = false;
                break;  
            }
            if(position[i] == position[j] - distance){
                ItsValid = false;
                break;  
            }
        }
        if(ItsValid == false) break;
    }
    return ItsValid;
}

bool generate_position(int *position, int index){
    if(index < 8){
        for(int i = 0; i < 8; i++){
            position[index] = i;
            generate_position(position,index + 1);
        }
    }
    else{
        bool correct = check_position(position);
        if(correct) {
            omp_set_lock(&lck);
            int index = position_index;
            position_index++;
            omp_unset_lock(&lck);
            for(int i = 0; i < 8; i++) valid_positions[index][i]=position[i];
        }
    }
}

int main(int argc, char *argv[])
{
    for(int nthreads = 1; nthreads < 9; nthreads++){
        printf("\n");
        for(int loops = 0; loops < 10; loops++){
            position_index = 0;
            omp_set_num_threads(nthreads);
            omp_init_lock(&lck);
            double start_time, run_time;
            start_time = omp_get_wtime();
            #pragma omp parallel
            {

                int id = omp_get_thread_num();
                for(int i = id; i < 8; i+=nthreads){
                    int pos[] = {i,0,0,0,0,0,0,0};
                    generate_position(pos,1);
                }
                
                //printf("Hello World from thread = %d", id);
                //printf(" with %d threads\n", omp_get_num_threads());
            }
            run_time = omp_get_wtime() - start_time;
            /*
            for (int j = 0; j < 92; j++){
                for (int i= 0; i < 8; i++) 
                    printf("%d ", valid_positions[j][i]);
                printf("\n");
            }
            */
           //printf("found %d positions in %fs, ",position_index, run_time);
            printf("%f, ", run_time);
            //printf("all done, with hopefully %d threads\n", nthreads);
        }
    }
}
//parallel generate positions -> parallel checking 
// master generate -> parallel checking
//

/*
median times:
1: 0.2445
2: 0.157
3: 0.137
4: 0.116
5: 0.117
6: 0.111
7: 0.106
8: 0.0805
*/

