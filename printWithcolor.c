#include <stdio.h>
int main(int argc,char **argv)
{
	int i = 0;
	for(i= 31; i< 38; i++)
	{
		printf("\033[%dm hello world \033[0m\n",i);
		printf("i is %d\n",i);
		sleep(1);
	}
	return 0;
}
# if 0

		int color = 31;
		while(1)
		{
			int readed = read(fromfd , buff , DEFAULT_BUFFER_SIZE);
			if(readed < 0)
			{
				printf("\033[33m read err...break loop\033[0m \n");
				break;
			}

			/*if(readed == 0 || time(NULL) - starttime < 10) */
			if(readed == 0)
			{
				printf("\033[35m read 0,will be continue...\033[0m \n");
				continue;
			}
			
			if(time(NULL) - starttime > 0)
			{
				starttime = time(NULL);
				printf("\033[%dm readed :%d \033[0m\n",color++, readed);
				if(color  == 38)
					color = 41;
				if(color == 47)
					color = 31;
			}
#endif
