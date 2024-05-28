#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#define BUFFER_SIZE 100
#define READ_END 0
#define WRITE_END 1
#define MAX_ITEMS 9

void displayMenu()
{
    // Declare the file pointer
    FILE *filePointer;

    // Declare the variable for the data to be read from
    // file
    char dataToBeRead[100];

    // Open the existing file GfgTest.c using fopen()
    // in read mode using "r" attribute
    filePointer = fopen("menu.txt", "r");

    // Check if this filePointer is null
    // which maybe if the file does not exist
    if (filePointer == NULL)
    {
        printf("menu.txt file failed to open.");
    }
    else
    {

        // Read the dataToBeRead from the file
        // using fgets() method
        while (fgets(dataToBeRead, 100, filePointer) != NULL)
        {

            // Print the dataToBeRead
            printf("%s", dataToBeRead);
        }

        // Closing the file using fclose()
        fclose(filePointer);
    }
}
int getMenu(int menu[])
{
    FILE *file;
    char line[100];
    int itemCount = 0;
    // Open the menu.txt file
    file = fopen("menu.txt", "r");

    // Check if the file is opened successfully
    if (file == NULL)
    {
        printf("Error opening the file.\n");
        return 1;
    }

    // Read each line from the file and extract item number and price
    while (fgets(line, sizeof(line), file) != NULL && itemCount < (MAX_ITEMS + 1))
    {
        int itemNumber;
        int price;

        // Use sscanf to extract item number and price from the line
        if (sscanf(line, "%d. %*[^0-9] %d INR", &itemNumber, &price) == 2)
        {
            // Store the price at the index corresponding to the item number
            menu[itemNumber] = price;

            itemCount++;
        }
    }

    // Close the file
    fclose(file);

    return itemCount; //size of menu
}

int main()
{

    printf("Enter Table Number:");
    int table_number;
    scanf("%d", &table_number);
    struct shmid_ds buf;
    key_t key; // key to identify shared memory segment
    // Generate a key for the shared memory segment
    if ((key = ftok("table.c", 'A' + table_number)) == -1)
    {
        perror("Error in ftok\n");
        return 1;
    }
    // printf("Key is: %d\n", key);
    int shmid;   // shared memory segment identifier
    int *shmPtr; // Pointer to the shared memory segment
    // Table process is writer
    //  Create a shared memory segment
    shmid = shmget(key, (BUFFER_SIZE), 0644 | IPC_CREAT); // IPC_PRIVATE as 1st arg.
    if (shmid == -1)
    {
        perror("Error in shmget in creating/ accessing shared memory\n");
        return 1;
    }
    // Attach the shared memory segment to the Table process
    shmPtr = shmat(shmid, NULL, 0);
    if (shmPtr == (void *)-1)
    {
        perror("Error in shmPtr in attaching the memory segment\n");
        return 1;
    }
    shmPtr[0] = 0;
    while (1)
    {

        printf("Enter Number of Customers at Table (maximum no. of customers can be 5): ");
        int n;
        scanf("%d", &n);
        if (n == -1)
        {
            shmPtr[0] = -4;
            break;
        }
        else
        {
            shmPtr[0] = 0; // handling if shmptr[0] not initialised then if it is -4 waiter may exit
        }
        int i;
        int pipe_fd[5][2];

        int flag = 0; // to keep track of valid orders

        int menu[MAX_ITEMS + 1];
        menu[0] = 0;
        int itemCount = 0;
        itemCount = getMenu(menu);
        // Display the menu
        displayMenu();
        

        while (1)
        {
            char all_orders[n][BUFFER_SIZE];
            flag = 0;
            for (i = 0; i < n; i++)
            {

                if (pipe(pipe_fd[i]) == -1)
                { // Create pipes for communication
                    perror("pipe");
                    return 1;
                }
                else
                {
                    // printf("\npipe created for child : %d\n", i + 1);
                }

                pid_t child_pid = fork();

                if (child_pid == -1)
                {
                    // Error handling
                    perror("fork");
                    return 1;
                }
                else if (child_pid == 0) // This is the child process
                {
                    close(pipe_fd[i][READ_END]); // Close unused read end
                                                 //  printf("\nChild %d created by Parent %d\n", getpid(), getppid());

                    // Child takes an order
                    char order_choice[BUFFER_SIZE];
                    printf("Enter the serial number(s) of the item(s) to order from the menu. Enter -1 when done: ");
                    
                    int idx = 0;
                    while (1)
                    {
                        int x;
                        scanf("%d", &x);
                        if (x == -1)
                        {
                            order_choice[idx] = '\0';
                            break;
                        }
                        if (x > itemCount)
                            x = itemCount + 1;
                        order_choice[idx] = x + '0'; // input is taken in the form of int and converted to char
                        idx++;
                    }

                    // Example: Child writes the order to the pipe
                    write(pipe_fd[i][WRITE_END], order_choice, sizeof(order_choice));

                    // Close the write end of the pipe before exiting
                    close(pipe_fd[i][WRITE_END]);

                    return 0;
                    // break; // The child should not create further processes
                }

                else
                {
                    // Parent process
                    wait(NULL);
                    close(pipe_fd[i][WRITE_END]); // Close unused write end
                    read(pipe_fd[i][READ_END], all_orders[i], BUFFER_SIZE);
                    close(pipe_fd[i][READ_END]);
                }
            }

            // printing all orders customer-wise
            // printf("Read:\n");
            // for (int k = 0; k < n; k++)
            // {
            //     printf("Customer %d: %s\n", k + 1, all_orders[k]);
            // }

            // convert all orders to a single 1D char array to be sent to waiter
            char msg[100];
            memset(msg, '\0', 100 * sizeof(char));
            for (int k = 0; k < n; k++)
            {
                strcat(msg, all_orders[k]); // when concatenation happened all the null characters (\0) in between got removed and only remains at last
            }
            // printf("Message to be sent:%s\n", msg);

            int k = 0;
            shmPtr[k] = msg[k] - '0';
            k++;
            while (msg[k - 1] != '\0')
            {
                shmPtr[k] = msg[k - 1] - '0';
                k++;
            }
            shmPtr[k] = -1;
            shmPtr[0] = -3;
            

            while (1)
            {
                if (shmPtr[0] == -1)
                {
                    printf("The total bill amount is %d INR.\n", shmPtr[1]);
                    break;
                }
                else if (shmPtr[0] == -2)
                {
                    flag = 1;
                    break;
                }
            }
            if (flag == 0) // all orders are valid
            {
                break;
            }
            else
            {
                printf("\n");
            }
        }
    }
    while (shmPtr[0] != -5)
    {
    }
    // Detach the shared memory segment from table's address space
    if (shmdt(shmPtr) == -1)
    {
        perror("Error in shmdt in detaching the memory segment\n");
        return 1;
    }

    if (shmctl(shmid, IPC_RMID, 0) == -1) // don't delete before reading is done
    {
        perror("Error in shmctl\n");
        return 1;
    }
    return 0;
} 