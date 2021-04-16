#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>


int NE;
int NR;
int TE;
int TR;

int main(int argc, char *argv[])
{
    char *ptr;
    NE = strtol(argv[1], &ptr, 10);
    NR = strtol(argv[2], &ptr, 10);
    TE = strtol(argv[3], &ptr, 10);
    TR = strtol(argv[4], &ptr, 10);

    if (argc > 0)
    {
     
        printf("%d %d %d %d\n", NE, NR, TE, TR);

        if (NE >= 1000 || NE <= 0)
        {
            printf("Argument NE must be in range 0<NE<1000 !!\n");
            return 1;
        }else if (NR >= 20 || NR <= 0)
        {
            printf("Argument NR must be in range 0<NR<20 !!\n");
            return 1;
        }else if (TE > 1000 || TE < 0)
        {
            printf("Argument TE must be in range 0<=TE<=1000 !!\n");
            return 1;
        }else if (TR > 1000 || TR < 0)
        {
            printf("Argument TR must be in range 0<=TR<=1000 !!\n");
            return 1;
        }
        
        

    }
}