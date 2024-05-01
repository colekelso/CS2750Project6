#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>

void parent();
void child();

#define SHMKEY 8575309
#define BUFF_SZ sizeof(int)

int main(int argc, char **argv){
    if (argc < 2) {
        printf("Please enter a series of positive integers. EX: ./mathwait.out 32 9 10 5\n");
        return 1;
    }
    switch (fork()){
        case -1:
            fprintf(stderr, "Error: Fork failed.\n");
            return 1;

        case 0:
            child(argc, argv);
            break;

        default:
            parent();
            break;
    }
    return 0;
}

void parent(){
    int shmid = shmget(SHMKEY, BUFF_SZ, 0777 | IPC_CREAT );
    if (shmid == -1){
        fprintf(stderr, "Parent: Error in shmget.\n");
        exit(1);
    }
    int* pint = (int*)(shmat(shmid,0,0));

    // initialize shared memory to -2, -2
    pint[0] = -2;
    pint[1] = -2;

    wait();

    if (pint[0] == -2) {
        printf("Some error occured.\n");
    } else if (pint[0] == -1) {
        printf("No Pairs Found.\n");
    } else {
        printf("Pair found by child: %d %d\n", pint[0], pint[1]);
    }

    shmdt(pint);
    shmctl(shmid, IPC_RMID, NULL);
}

void child(int argc, char **argv){
    int shmid = shmget(SHMKEY, BUFF_SZ, 0777 | IPC_CREAT );
    if (shmid == -1){
        fprintf(stderr, "Child: Error in shmget.\n");
        exit(1);
    }
    int* cint = (int*)(shmat(shmid,0,0));
    // read integers from command line input
    int N = argc - 1;
    int* nums = (int*)malloc((N+1) * sizeof(int));
    for (int i=0; i<N; i++) {
        nums[i] = atoi(argv[1+i]);
    }
    // find pairs
    int pairs = 0;
    for (int i=0; i<N; i++) {
        for (int j=i+1; j<N; j++) {
            if (nums[i] + nums[j] == 19) {
                cint[0] = nums[i];
                cint[1] = nums[j];
                pairs++;
            }
        }
    }
    free(nums);
    if (!pairs) {
        cint[0] = -1;
        cint[1] = -1;
    }
    shmdt(cint);
}