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

int NE;
int NR;
int TE;
int TR;

sem_t *santaSem;
sem_t *reindeerSem;
sem_t *elfTex;
sem_t *mutex;

int argCheck(int argc, char **argv)
{

    if (argc != 5)
    {
        printf("Wrong number of arguments passed!!\n");
        return 1;
    }

    for (int i = 1; i < argc; i++)
    {
        char *ptr;
        int check = strtol(argv[i], &ptr, 10);

        if (ptr[0] == '.')
        {
            printf("Argument is not an int type!!\n");
            return 1;
        }

        if (*ptr != '\0')
        {
            printf("Argument is not a number!!\n");
            return 1;
        }

        if (i == 1)
        {
            if (check >= 1000 || check <= 0)
            {
                printf("Argument NE must be in range 0<NE<1000 !!\n");
                return 1;
            }
        }

        if (i == 2)
        {
            if (check >= 20 || check <= 0)
            {
                printf("Argument NR must be in range 0<NR<20 !!\n");
                return 1;
            }
        }

        if (i == 3)
        {
            if (check > 1000 || check < 0)
            {
                printf("Argument TE must be in range 0<=TE<=1000 !!\n");
                return 1;
            }
        }

        if (i == 4)
        {
            if (check > 1000 || check < 0)
            {
                printf("Argument TR must be in range 0<=TR<=1000 !!\n");
                return 1;
            }
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    char *ptr;
    NE = strtol(argv[1], &ptr, 10);
    NR = strtol(argv[2], &ptr, 10);
    TE = strtol(argv[3], &ptr, 10);
    TR = strtol(argv[4], &ptr, 10);

    printf("%d %d %d %d\n", NE, NR, TE, TR);

    if(argCheck(argc, argv) == 1)
    {
        return 1;
    }else
    {
        printf("Arguments correct");
    }
    
    
}