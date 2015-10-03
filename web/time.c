#include <stdio.h>
#include <time.h>
 
int main(void)
{
    time_t now;
    time(&now);
    printf("now time is %d\n", now);
 
    return(0);
}
