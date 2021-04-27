#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <fcntl.h>

int NE; // The number of elves to be generated
int NR; // The number of reindeers to be generated
int TE; // The maximum time in milliseconds that the elf works independently
int TR; // The maximum time in milliseconds that a reindeer returns home from the vacation

/*Declaration of required semaphores*/
sem_t *santaSem = NULL;    // Santa waits on santaSem until either an elf or a reindeer signals him
sem_t *reindeerSem = NULL; // The reindeers waits on reindeerSem until Santa signals them to enter thepaddock and get hitched
sem_t *elfTex = NULL;      // Used to prevent additional elves from entering while three elves are being helped
sem_t *mutex = NULL;       // Protects the elves and raindeers
sem_t *fileWrite = NULL;   // Protection for writing into file

FILE **file = NULL; // Variable for the proj2.out
int fileID = 0;     // ID variable for the shared memory usage

int rowID = 0;
int *row = NULL;

/*Function for argument corectness check*/
int argCheck(int argc, char **argv)
{
    /*There must be exactly 5 arguments*/
    if (argc != 5)
    {
        fprintf(stderr, "Wrong number of arguments passed!!\n");
        return 1;
    }

    /*For loop used for going thru the arguments passed*/
    for (int i = 1; i < argc; i++)
    {
        char *ptr;
        int check = strtol(argv[i], &ptr, 10);

        /*If one of the passed arguments isn't interger, function returns 1*/
        if (ptr[0] == '.')
        {
            fprintf(stderr, "Argument is not an int type!!\n");
            return 1;
        }

        /*If one of the passed arguments isn't a number, function returns 1*/
        if (*ptr != '\0')
        {
            fprintf(stderr, "Argument is not a number!!\n");
            return 1;
        }

        /*If the first argument doesn't match specified range, function returns 1*/
        if (i == 1)
        {
            if (check >= 1000 || check <= 0)
            {
                fprintf(stderr, "Argument NE must be in range 0<NE<1000 !!\n");
                return 1;
            }
        }

        /*If the second argument doesn't match specified range, function returns 1*/
        if (i == 2)
        {
            if (check >= 20 || check <= 0)
            {
                fprintf(stderr, "Argument NR must be in range 0<NR<20 !!\n");
                return 1;
            }
        }

        /*If the third argument doesn't match specified range, function returns 1*/
        if (i == 3)
        {
            if (check > 1000 || check < 0)
            {
                fprintf(stderr, "Argument TE must be in range 0<=TE<=1000 !!\n");
                return 1;
            }
        }

        /*If the fourth argument doesn't match specified range, function returns 1*/
        if (i == 4)
        {
            if (check > 1000 || check < 0)
            {
                fprintf(stderr, "Argument TR must be in range 0<=TR<=1000 !!\n");
                return 1;
            }
        }
    }

    /*If all the argumnts were passed correctly, function returns 0*/
    return 0;
}

/*Function for shared memmory initialization.
 Function innitializes the shared memmory and at the same time checks if the process was successfull, if not, returns 1*/
int initMemory()
{
    if ((fileID = shmget(IPC_PRIVATE, sizeof(FILE *), 0666 | IPC_CREAT)) == -1)
    {
        fprintf(stderr, "Shared memory segment could not be created\n");
        return 1;
    }

    if ((file = shmat(fileID, NULL, 0)) == NULL)
    {
        fprintf(stderr, "Error occured while attaching the shared memory segment\n");
        return 1;
    }

    if ((rowID = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
    {
        fprintf(stderr, "Shared memory segment could not be created\n");
        return 1;
    }

    if ((row = shmat(rowID, NULL, 0)) == NULL)
    {
        fprintf(stderr, "Error occured while attaching the shared memory segment\n");
        return 1;
    }

    return 0; // If the process of shared memory initialization was successfull function returns 0
}

/*Function used for destroying the initialized shared memory*/
void destMemory()
{
    if (file != NULL)
    {
        shmctl(fileID, IPC_RMID, NULL);
    }

    if (row != NULL)
    {
        shmctl(rowID, IPC_RMID, NULL);
    }
}

/*Function for closing and unlinking semaphores*/
void destSemaphore(sem_t *sem, char *name)
{
    sem_close(sem);
    sem_unlink(name);
}

/*Function for destroying initialized semaphores*/
void destSemaphores()
{
    if (mutex != NULL)
        destSemaphore(mutex, "mutex_sem");
    if (santaSem != NULL)
        destSemaphore(santaSem, "santa_sem");
    if (elfTex != NULL)
        destSemaphore(elfTex, "elfTex");
    if (reindeerSem != NULL)
        destSemaphore(reindeerSem, "reindeer_sem");
    if (fileWrite != NULL)
        destSemaphore(fileWrite, "fileWrite");
}

/*Function for output printing for the santa function*/
void santaWrite(char *write, int action, int wait, int val)
{
    sem_wait(fileWrite);
    if (val == 1)
        fprintf(*file, write, action);
    else
        fprintf(*file, write, action, wait);
    (*row)++; // Increment action number
    sem_post(fileWrite);
}

void santa()
{
    santaWrite("%d: Santa: going to sleep\n", *row, 0, 1);

    destSemaphores();
    destMemory();
    fclose(*file);
}

/*Function for generating santa process*/
void genSanta()
{
pid_t santa_process = fork();    // Forking the santa process
    if (santa_process == 0)    // Child = one santa
    {
        santa();
    }
exit(0);    
}

/*Function for output printing for the elves function*/
void elfWrite(char *write, int action, int id, int wait, int val)
{
    sem_wait(fileWrite);
    if (val == 2)
        fprintf(*file, write, action, id);
    else
    {
        fprintf(*file, write, action, id, wait);
    }
    (*row)++; // Increment action number
    sem_post(fileWrite);
}

void elf(int id)
{
    sem_wait(mutex);
    elfWrite("%d: ELF %d: started\n", *row, id, 0, 2);
    sem_post(mutex);

    destSemaphores();
    destMemory();
    fclose(*file);
}

/*Function for generating elf processes*/
void genElves(int NE, int TE)
{
for (int i = 1; i < NE + 1; i++)
        {
            int rndE = rand() % TE * 1000; // Random time number between each processes
            usleep(rndE);              // Waiting until new process is created
            pid_t elves_process = fork();    // Forking the elves process
            if (elves_process == 0)    // Child = one elf
            {
                elf(i);
            }
        }
        exit(0);
}

/*Function for output printing for the reindeer function*/
void reindeerWrite(char *write, int action, int id, int wait, int val)
{
    sem_wait(fileWrite);
    if (val == 3)
        fprintf(*file, write, action, id);
    else
    {
        fprintf(*file, write, action, id, wait);
    }
    (*row)++; // Increment action number
    sem_post(fileWrite);

}

void reinDeer(int id)
{
    sem_wait(mutex);
    reindeerWrite("%d: RD  %d: rstarted\n", *row, id, 0, 3);
    sem_post(mutex);

    destSemaphores();
    destMemory();
    fclose(*file);
}

/*Function for generating reindeer processes*/
void genReinD(int NR, int TR)
{
for (int i = 1; i < NR + 1; i++)
        {
            int rndRD = rand() % TR * 1000; // Random time number between each processes
            usleep(rndRD);              // Waiting until new process is created
            pid_t reindeer_process = fork(); // Forking the reindeer process
            if (reindeer_process == 0) // Child = one reindeer
            {
                reinDeer(i);
            }
        }
        exit(0);
}

int main(int argc, char **argv)
{
    /*Control check if all arguments were passed correctly, if not, returns 1 and the programm ends with the specific error*/
    if (argCheck(argc, argv) == 1)
    {
        return 1;
    }

    setbuf(stdout, NULL); // Sets the stdout buffer to NULL
    /*Parsing each one of the arguments to the specific variables*/
    char *ptr;
    NE = strtol(argv[1], &ptr, 10);
    NR = strtol(argv[2], &ptr, 10);
    TE = strtol(argv[3], &ptr, 10);
    TR = strtol(argv[4], &ptr, 10);

    printf("%d %d %d %d\n", NE, NR, TE, TR); // TO BE REMOVED!!! (control output for arguments)    

    int memCheck = initMemory(); // Initializing the shared memory
    /*Control check if the shared memory initialization was successfull, if not, 
     destroying all successfully initialized memory, returning 1 and ending the programm with specific error*/
    if (memCheck == 1)
    {
        destMemory();
        return 1;
    }

    *row = 1;

    *file = fopen("proj2.out", "w"); // Opening the output file for writing
    /*Control check if file was opened successfully, if wan't returns 1*/
    if (file == NULL)
    {
        fprintf(stderr, "Error while opening file proj2.out\n");
        return 1;
    }
    setbuf(*file, NULL); // Sets the file buffer to NULL

    /*Creating semaphore and  checking for errors, if error occures prints error message to the stderr, destroys semaphores, destroys initialized memory  and closes the file for outpu writing*/
    if ((mutex = sem_open("mutex_sem", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED || (santaSem = sem_open("santa_sem", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED || (elfTex = sem_open("elfTex", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED || (reindeerSem = sem_open("reindeer_sem", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED || (fileWrite = sem_open("fileWrite", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED)
    {
        fprintf(stderr, "Error while creating semaphores\n");
        destSemaphores();
        destMemory();
        fclose(*file);
        return 1;
    }

    pid_t main_process = fork(); // Forking the main_process

    if (main_process == 0)
    {
        // Child
        pid_t santa_generator = fork();
        if (santa_generator == 0)
        {
            santa();
        }
        
        pid_t elf_generator = fork();
        if (elf_generator == 0)
        {
            genElves(NE, TE);
        }

        pid_t reindeer_generator = fork();
        if (reindeer_generator == 0)
        {
            genReinD(NR, TR);
        }
        exit(0);
    }
    else if (main_process > 0)
    {
        // Parent
        destSemaphores();
        destMemory();
        fclose(*file);
        exit(0);
        
    }
    else // Error check
    {
        fprintf(stderr, "Error while creating process\n");
        destSemaphores();
        destMemory();
        fclose(*file);
        return 1;
    }

    return 0; // Correctly ending the programm with no errors ocured
}
