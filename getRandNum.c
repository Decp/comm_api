#include <stdio.h>
#include <stdlib.h>
#include <time.h>
int main(int argc, char *argv[])
{
	unsigned long ret = 0;
	while(1)
	{
		
		srand(time(NULL));
		ret = rand();
		//printf("%d\n",ret);
		printf("%d\n",ret>>12);
		//usleep(1000*1200);
		break;
	}
	return 0;
}
