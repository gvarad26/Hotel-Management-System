#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define BUF_SIZE 100

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
    shmid_ad = shmget(key_ad, (BUF_SIZE), 0644 | IPC_CREAT); // IPC_PRIVATE as 1st arg.
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
    shmPtr_ad[0] = 'l';

    ///////////////////////////////////////////////////////////////////////////////////////////////////

    printf("Enter the Total Number of Tables at the Hotel:");
    int n_tables;
    scanf("%d", &n_tables);

    ///////////////////////////////////////////////////////////////////////////////////////////////////

    struct shmid_ds buf[n_tables + 1];
    key_t key[n_tables + 1]; // key to identify shared memory segment
    // Generate a key for the shared memory segment
    int shmid[n_tables + 1];   // shared memory segment identifier
    int *shmPtr[n_tables + 1]; // Pointer to the shared memory segment

    for (int i = 1; i <= n_tables; i++)
    {
        if ((key[i] = ftok("hotelmanager.c", 'A' + i)) == -1)
        {
            perror("Error in ftok\n");
            return 1;
        }
        // printf("Key is %d\n", key[i]);

        // Create a shared memory segment
        shmid[i] = shmget(key[i], (BUF_SIZE), 0644 | IPC_CREAT); // IPC_PRIVATE as 1st arg.
        if (shmid[i] == -1)
        {
            perror("Error in shmget in creating/ accessing shared memory\n");
            return 1;
        }
        // Attach the shared memory segment to the waiter process
        shmPtr[i] = shmat(shmid[i], NULL, 0); // NULL means kernel will find a place for the segment
        if (shmPtr[i] == (void *)-1)
        {
            perror("Error in shmPtr in attaching the memory segment\n");
            return 1;
        }
        // if (shmctl(shmid[i], IPC_STAT, &buf[i]) != -1)
        //{
        //     printf("\nSegment size is %lu\n", buf[i].shm_segsz);
        //     printf("\nNo. of processes attached %lu\n", buf[i].shm_nattch);
        //}
        // printf("Manager reading...\n");
    }

    // File Pointer declared
    FILE *fptr;

    // File opened
    fptr = fopen("./earnings.txt", "w+");

    // Failed Condition
    if (fptr == NULL)
    {
        printf("Error Occurred While writing to a text file !\n");
        exit(1);
    }
    double total_earnings = 0;
    int c = 0;

    while (1)
    {
        for (int i = 1; i <= n_tables; i++)
        {
            if (shmPtr[i][0] == -1)
            {
                total_earnings += shmPtr[i][1];
                char str[100]; // = "Earning from Table %d: %d INR"
                memset(str, '\0', 100 * sizeof(char));
                sprintf(str, "Earning from Table %d: %d INR\n", i, shmPtr[i][1]);
                fputs(str, fptr);
                shmPtr[i][0] = 0;
            }
            else if (shmPtr[i][0] == -2)
            {
                c++;
                shmPtr[i][0] = 0;
            }
        }
        if (c == n_tables)
        {
            break;
        }
    }

    for (int i = 1; i <= n_tables; i++)
    {
        // Detach the shared memory segment
        if (shmdt(shmPtr[i]) == -1)
        {
            perror("Error in shmdt in detaching the memory segment\n");
            return 1;
        }
        // Mark the shared memory segment for deletion (A control operation)
        if (shmctl(shmid[i], IPC_RMID, 0) == -1) // don't delete before reading is done
        {
            perror("Error in shmctl\n");
            return 1;
        }
    }

    while (1)
    {
        if (shmPtr_ad[0] == 'Y')
        {
            //shmPtr_ad[0] = 'f';
            break;
        }
    }

    double wages = (total_earnings * 4) / 10;
    double profit = total_earnings - wages;
    char str[100];
    printf("Total Earnings of Hotel: %0.2lf INR\nTotal Wages of Waiters: %0.2lf INR\nTotal Profit: %0.2lf INR\n", total_earnings, wages, profit);
    sprintf(str, "Total Earnings of Hotel: %0.2lf INR\nTotal Wages of Waiters: %0.2lf INR\nTotal Profit: %0.2lf INR\n", total_earnings, wages, profit);
    fputs(str, fptr);
    printf("Thank you for visiting the Hotel!\n");

    if (shmdt(shmPtr_ad) == -1)
    {
        perror("Error in shmdt in detaching the memory segment\n");
        return 1;
    }
    if (shmctl(shmid_ad, IPC_RMID, 0) == -1) // don't delete before reading is done
    {
        perror("Error in shmctl\n");
        return 1;
    }
    return 0;
}