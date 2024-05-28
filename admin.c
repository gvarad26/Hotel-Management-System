#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define buf_ad_SIZE 100

int main()
{

    struct shmid_ds buf_ad;
    key_t key_ad; // key_ad to identify shared memory segment
    // Generate a key_ad for the shared memory segment
    if ((key_ad = ftok("admin.c", 'A')) == -1)
    {
        perror("Error in ftok\n");
        return 1;
    }
    // printf("key_ad is %d\n", key_ad);
    int shmid_ad;    // shared memory segment identifier
    char *shmPtr_ad; // Pointer to the shared memory segment

    // Create a shared memory segment
    shmid_ad = shmget(key_ad, (buf_ad_SIZE), 0644 | IPC_CREAT); // IPC_PRIVATE as 1st arg.
    if (shmid_ad == -1)
    {
        perror("Error in shmget in creating/ accessing shared memory\n");
        return 1;
    }
    // Attach the shared memory segment to the waiter process
    shmPtr_ad = shmat(shmid_ad, NULL, 0); // NULL means kernel will find a place for the segment
    if (shmPtr_ad == (void *)-1)
    {
        perror("Error in shmPtr_ad in attaching the memory segment\n");
        return 1;
    }
    // if (shmctl(shmid_ad, IPC_STAT, &buf_ad) != -1)
    // {
    //     printf("\nSegment size is %lu\n", buf_ad.shm_segsz);
    //     printf("\nNo. of processes attached %lu\n", buf_ad.shm_nattch);
    // }

    char n, t;
    while (1)
    {
        printf("Do you want to close the hotel? Enter Y for Yes and N for No.");
        scanf("%c", &n);
        scanf("%c", &t);
        // fflush(stdin);

        if (n == 'Y')
            break;
    }

    shmPtr_ad[0] = n;
    // if (shmPtr_ad[0] == 'f')
    // {

    if (shmdt(shmPtr_ad) == -1)
    {
        perror("Error in shmdt in detaching the memory segment\n");
        return 1;
    }
    return 0;
//}
}
