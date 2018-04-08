#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <sys/time.h>



int    time_substract(struct timeval *result, struct timeval *begin,struct timeval *end)

{

    if(begin->tv_sec > end->tv_sec)    return -1;

    if((begin->tv_sec == end->tv_sec) && (begin->tv_usec > end->tv_usec))    return -2;



    result->tv_sec    = (end->tv_sec - begin->tv_sec);

    result->tv_usec    = (end->tv_usec - begin->tv_usec);

    

    if(result->tv_usec < 0)

    {

        result->tv_sec--;

        result->tv_usec += 1000000;

    }



    return 0;

}





int main(int argc, char **argv)

{

    struct timeval start,stop,diff;



    memset(&start,0,sizeof(struct timeval));

    memset(&stop,0,sizeof(struct timeval));

    memset(&diff,0,sizeof(struct timeval));



    gettimeofday(&start,0);



    //做你要做的事...
    
        printf("hello world\n");
    
    
            gettimeofday(&stop,0);
    
    
                time_substract(&diff,&start,&stop);
    
                    printf("Total time : %d s,%d us\n",(int)diff.tv_sec,(int)diff.tv_usec);
    
                   }
    
