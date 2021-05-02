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
sem_t *santaSem = NULL; // Santa waits on santaSem until either an elf or a reindeer signals him  
sem_t *santaHelping = NULL; // Semaphore is used for helping elves
sem_t *santaHelped = NULL;  // Semaphore used for signaling that all evles were helped
sem_t *reindeerSem = NULL; // The reindeers waits on reindeerSem until Santa signals them to enter the paddock and get hitched 
sem_t *elfTex = NULL;      // Used to prevent additional elves from entering while three elves are being helped
sem_t *finishSem= NULL; // Semaphore signals santa that all reindeers are back home
sem_t *mutex = NULL;       // Protects the elves and raindeers
sem_t *fileWrite = NULL;   // Protection for writing into file

FILE **file = NULL; // Variable for the proj2.out
int fileID = 0;     // ID variable for the shared memory usage

int rowID = 0; // ID variable for the shared memory usage
int *row = NULL; // Shared variable for numbers of actual actions

int christmasID = 0; // ID variable for the shared memory usage
int *christmasCheck = NULL; // Shared variable for checking whether all reindeers came home and chritmas should be started

int finishID = 0; // ID variable for the shared memory usage
int *finishCheck = NULL; // Shared variable used as switch, when the value is set to 1 the cycle of elves requesting help and getting helped ends and they will be taking holidays

int elfCid = 0; // ID variable for the shared memory usage
int *elfCount = NULL; // Shared variable which holds the actual number of waiting elves and after there are 3 elves waiting, controls if all elves were helped

int reindeerID = 0; // ID variable for the shared memory usage
/* reindeerCount => Shared variable used to control how many reindeers have returned home. After all have returned home, variable is used
to control if all reindeers are hitched*/
int *reindeerCount = NULL; 

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

    if ((elfCid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
    {
        fprintf(stderr, "Shared memory segment could not be created\n");
        return 1;
    }

    if ((elfCount = shmat(elfCid, NULL, 0)) == NULL)
    {
        fprintf(stderr, "Error occured while attaching the shared memory segment\n");
        return 1;
    }

    if ((christmasID = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
    {
        fprintf(stderr, "Shared memory segment could not be created\n");
        return 1;
    }

    if ((christmasCheck = shmat(christmasID, NULL, 0)) == NULL)
    {
        fprintf(stderr, "Error occured while attaching the shared memory segment\n");
        return 1;
    }

    if ((finishID = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
    {
        fprintf(stderr, "Shared memory segment could not be created\n");
        return 1;
    }

    if ((finishCheck = shmat(finishID, NULL, 0)) == NULL)
    {
        fprintf(stderr, "Error occured while attaching the shared memory segment\n");
        return 1;
    }

    if ((reindeerID = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
    {
        fprintf(stderr, "Shared memory segment could not be created\n");
        return 1;
    }

    if ((reindeerCount = shmat(reindeerID, NULL, 0)) == NULL)
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

    if (elfCount != NULL)
    {
        shmctl(elfCid, IPC_RMID, NULL);
    }
    
    if (christmasCheck != NULL)
    {
        shmctl(christmasID, IPC_RMID, NULL);
    }

    if (finishCheck != NULL)
    {
        shmctl(finishID, IPC_RMID, NULL);
    }

    if (reindeerCount != NULL)
    {
        shmctl(reindeerID, IPC_RMID, NULL);
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
        destSemaphore(mutex, "mutex");
    if (santaSem != NULL)
        destSemaphore(santaSem, "santa_sem");
    if (santaHelping != NULL)
        destSemaphore(santaHelping, "santaHelping_sem");
    if (santaHelped != NULL)
        destSemaphore(santaHelped, "santaHelped_sem");    
    if (elfTex != NULL)
        destSemaphore(elfTex, "elfTex");
    if (reindeerSem != NULL)
        destSemaphore(reindeerSem, "reindeer_sem");
    if (finishSem != NULL)
        destSemaphore(finishSem, "finish_sem");
    if (fileWrite != NULL)
        destSemaphore(fileWrite, "fileWrite");
}

/*Function for output printing for the elves function*/
void writeOut(char *write, int actionN, int id, int val)
{
    if (val == 1)
    {
        fprintf(*file, write, actionN, id); // Output print for elves and reindeers
    } else
    {
        fprintf(*file, write, actionN); // Santa has no id so he requires diffrent output print
    }
    (*row)++; // Increment action number
    sem_post(fileWrite);
}


/*Santa function at the start prints that sansta is going to sleep. Next there is endless cycle for santa helping elves.
When all reindeers have returned home, the christmasCheck is set to 1 and the cycle ends and santa prints closing workshop.*/
void santa()
{
    while(1) {
        sem_wait(fileWrite);
        writeOut("%d: Santa: going to sleep\n", *row, 0, 2);
        
        sem_wait(santaSem);
        sem_wait(mutex);
        if((*christmasCheck) == 1) {
            break;
        }
        sem_wait(fileWrite);
        writeOut("%d: Santa: helping elves\n", *row, 0, 2);
        
        sem_post(santaHelping);
        sem_post(mutex);
        sem_wait(santaHelped);
    }
    sem_wait(fileWrite);
    writeOut("%d: Santa: closing workshop\n", *row, 0, 2);
    
    sem_post(reindeerSem);
    sem_wait(finishSem);
    sem_wait(fileWrite);
    writeOut("%d: Santa: Christmas started\n", *row, 0,2);
    
    (*finishCheck) = 1;
    sem_post(mutex);
    sem_post(santaHelping);
}

void elf(int id, int TE)
{
    sem_wait(fileWrite);
    writeOut("%d: Elf %d: started\n", *row, id, 1);
    int rndE =rand() % (TE + 1) * 1000;
    while(1) 
    {
        usleep(rndE);
        sem_wait(fileWrite);
        writeOut("%d: Elf %d: need help\n", *row, id, 1);
        
        sem_wait(elfTex);
        sem_wait(mutex);
        if((*finishCheck) == 1) {
            sem_post(mutex);
            break;
        }
        (*elfCount)++;
        if((*elfCount) == 3) {
            sem_post(santaSem);
        }
        sem_post(mutex);
        sem_wait(santaHelping);
        if((*finishCheck) == 1) {
            break;
        }
        (*elfCount)--;
        sem_wait(fileWrite);
        writeOut("%d: Elf %d: get help\n", *row, id, 1);
        
        if((*elfCount) == 0) {
            sem_post(santaHelped);
            sem_post(elfTex);
            sem_post(elfTex);
            sem_post(elfTex);
        } else {
            sem_post(santaHelping);
        }
    }
    sem_wait(reindeerSem);
    sem_wait(fileWrite);
    writeOut("%d: Elf %d: taking holidays\n", *row, id, 1);
    
    sem_post(reindeerSem);
    sem_post(elfTex);
    sem_post(santaHelping);
}

void reinDeer(int id, int TR, int NR)
{
    sem_wait(fileWrite);
    writeOut("%d: RD  %d: rstarted\n", *row, id, 1);
    
    int rndR = (rand() % ((TR + 1) + (TR + 1 / 2)) * 1000);
    usleep(rndR);
    sem_wait(fileWrite);
    writeOut("%d: RD  %d: return home\n", *row, id, 1);
    
    sem_wait(mutex);
    (*reindeerCount)--;
    if((*reindeerCount) == 0) {
        (*christmasCheck) = 1;
        sem_post(santaSem);
    }
    sem_post(mutex);

    sem_wait(reindeerSem);
    sem_wait(fileWrite);
    writeOut("%d: RD  %d: get hitched\n", *row, id, 1);
    (*reindeerCount)++;
    if((*reindeerCount) == NR) {
        sem_post(finishSem);
    }
    sem_post(reindeerSem);

}

/*Function for generating reuired processes. The main process creates one Santa process immediately after startup, NO elf processes 
and NR reindeer processes.*/
void processGenerator()
{
pid_t mainProcess = fork();
    if(mainProcess == 0) {
        santa();
        exit(0);
    }else if (mainProcess > 0)
    {
        // parent
    }else // Error check
    {
        fprintf(stderr, "Error while creating process\n");
        destSemaphores();
        destMemory();
        fclose(*file);
        exit(1);
    }
    
    for(int i = 1; i < NE + 1; i++) {
        mainProcess = fork();
        if(mainProcess == 0) {
            elf(i, TE);
            exit(0);
        }else if (mainProcess > 0)
        {
            // parent
        }else // Error check
        {
            fprintf(stderr, "Error while creating process\n");
            destSemaphores();
            destMemory();
            fclose(*file);
            exit(1);
        }
        
    }

    for(int i = 1; i < NR + 1; i++) {
        mainProcess = fork();
        if(mainProcess == 0) {
            reinDeer(i, TR, NR);
            exit(0);
        }else if (mainProcess > 0)
        {
            // parent
        }else // Error check
        {
            fprintf(stderr, "Error while creating process\n");
            destSemaphores();
            destMemory();
            fclose(*file);
            exit(1);
        }
     
    }

}

int main(int argc, char **argv)
{
    /*Control check if all arguments were passed correctly, if not, returns 1 and the programm ends with the specific error*/
    if (argCheck(argc, argv) == 1)
    {
        exit(1);
    }

    /*Parsing each one of the arguments to the specific variables*/
    char *ptr;
    NE = strtol(argv[1], &ptr, 10);
    NR = strtol(argv[2], &ptr, 10);
    TE = strtol(argv[3], &ptr, 10);
    TR = strtol(argv[4], &ptr, 10);

    //printf("%d %d %d %d\n", NE, NR, TE, TR); // TO BE REMOVED!!! (control output for arguments)    

    int memCheck = initMemory(); // Initializing the shared memory
    /*Control check if the shared memory initialization was successfull, if not, 
     destroying all successfully initialized memory, returning 1 and ending the programm with specific error*/
    if (memCheck == 1)
    {
        destMemory();
        exit(1);
    }

    *elfCount = 0; // Setting number of waiting elves to 0
    *reindeerCount = NR; // Setting number of reaindeers to be returned home to number specified in argument 2
    *row = 1; // Setting the initial value of serial number of the action being currently carried out to 1

    *file = fopen("proj2.out", "w"); // Opening the output file for writing
    /*Control check if file was opened successfully, if wan't returns 1*/
    if (file == NULL)
    {
        fprintf(stderr, "Error while opening file proj2.out\n");
        exit(1);
    }
    setbuf(*file, NULL); // Sets the file buffer to NULL

    /*Creating semaphore and  checking for errors, if error occures prints error message to the stderr, destroys semaphores, destroys initialized memory  and closes the file for outpu writing*/
    if ((mutex = sem_open("mutex", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED 
        || (santaSem = sem_open("santa_sem", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED
        || (santaHelping = sem_open("santaHelping_sem", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED
        || (santaHelped = sem_open("santaHelped_sem", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED 
        || (elfTex = sem_open("elfTex", O_CREAT | O_EXCL, 0666, 3)) == SEM_FAILED 
        || (reindeerSem = sem_open("reindeer_sem", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED 
        || (finishSem = sem_open("finish_sem", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED
        || (fileWrite = sem_open("fileWrite", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED)
    {
        fprintf(stderr, "Error while creating semaphores\n");
        destSemaphores();
        destMemory();
        fclose(*file);
        exit(1);
    }

    processGenerator(); // Calling function for generating required precesses

    destSemaphores(); // Destroying created semaphores
    destMemory(); // Destroying initialized shared memmory
    fclose(*file); // Closing out file

    exit(0); // Correctly ending the programm with no errors ocured
}
