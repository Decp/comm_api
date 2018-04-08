#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>

#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <iconv.h> 
#include <memory.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>         /* XXX */
#include <netdb.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <stddef.h>
#include <ifaddrs.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
	int fd, intrface;
	struct ifreq buf[16];
	struct ifconf ifc;

	//BOOL found = FALSE;

	if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		return 1;
	}
	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = (caddr_t)buf;

	if (ioctl (fd, SIOCGIFCONF, (char *) &ifc) < 0)
	{
		close(fd);
		return 1;
	}
printf("***eth name is %s\n",argv[1]);

	intrface = ifc.ifc_len/sizeof(struct ifreq);
	char ip[32] = {0};
	while(intrface-->0)
	{
		if (strcmp(buf[intrface].ifr_name, argv[1]) == 0)
		{
		//	found = TRUE;
			if(1)
			{
				if ((ioctl (fd, SIOCGIFADDR, (char*)&buf[intrface])) < 0)
				{
					syslog(LOG_INFO, "%s(%d) get ip failed !!!\n", __FUNCTION__ , __LINE__ );
				}
				sprintf(ip, "%s", inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr));
				printf("eth name is %s\n",buf[intrface].ifr_name);
				printf("ip : %s" , ip);
			}
#if 0
			if(mask)
			{
				if ((ioctl (fd, SIOCGIFNETMASK , (char*)&buf[intrface])) < 0)
				{
					syslog(LOG_INFO, "%s(%d) get mask failed !!!\n", __FUNCTION__ , __LINE__ );
				}

				sprintf(mask, "%s", inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_netmask))->sin_addr));
				printf("mask : %s\n" , mask);
			}
#endif
			break;
		}
	}

//	if(!found)
//	{
		/* not support */
//		close(fd);
//		return 1;
//	}

	close(fd);
	return 0;
}

