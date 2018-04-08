
/*************************************************************
le:        statusinfo.c
 *
 *    @brief:        ��linuxϵͳ��ȡcpu���ڴ�ʹ�����
 *
 *    @version    1.0
 *
 ***************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/vfs.h>
typedef struct PACKED         //����һ��cpu occupy�Ľṹ��
{
	char name[20];      //����һ��char���͵�������name��20��Ԫ��
	unsigned int user; //����һ���޷��ŵ�int���͵�user
	unsigned int nice; //����һ���޷��ŵ�int���͵�nice
	unsigned int system;//����һ���޷��ŵ�int���͵�system
	unsigned int idle; //����һ���޷��ŵ�int���͵�idle
	unsigned int lowait;
	unsigned int irq;
	unsigned int softirq;
}CPU_OCCUPY;

typedef struct __MEM
{
//	unsigned char name[20];
	float total;
	float free;
}MEM;

int get_meminfo()
{
	MEM meminfo;
	memset(&meminfo,0x00,sizeof(MEM));
	FILE* fp = fopen("/proc/meminfo","r");

	if(fp == NULL)
	{
		printf("Can not open file\r\n");
		return 0;
	}
	
	char buf[64];
	char name[32];
	memset(buf,0x00,sizeof(buf));
	fgets(buf,sizeof(buf),fp);
	sscanf(buf,"%s %f %s",name,&meminfo.total,name);
	memset(buf,0x00,sizeof(buf));
	fgets(buf,sizeof(buf),fp);
	sscanf(buf,"%s %f %s",name,&meminfo.free,name);
	printf("buf is %s  name is %s %f\r\n",buf,name,meminfo.free);
	float temp;

	sscanf(buf,"%s			%f %s",name,&temp,name);
	printf("temp is %f \r\n",temp);
	double rate = (meminfo.total - meminfo.free)/meminfo.total;
	printf("%f  %f	rate is %f\%\r\n",meminfo.total,meminfo.free,rate*100);
	fclose(fp);
	return 1;
}

int cal_cpuoccupy (CPU_OCCUPY *o, CPU_OCCUPY *n) 
{   
	unsigned long od, nd;    
	unsigned long id, sd;
	double cpu_use = 0;   

	od = (unsigned long) (o->user + o->nice + o->system +o->idle + o->lowait + o->irq + o->softirq);//��һ��(�û�+���ȼ�+ϵͳ+����)��ʱ���ٸ���od
	nd = (unsigned long) (n->user + n->nice + n->system +n->idle + n->lowait + n->irq + n->softirq);//�ڶ���(�û�+���ȼ�+ϵͳ+����)��ʱ���ٸ���od

	double sum = nd - od;
	double idle = n->idle - o->idle;
	cpu_use = idle/sum;


	printf("%f\%\r\n",cpu_use*100);

	idle = n->user + n->system + n->nice -o->user - o->system- o->nice;
	cpu_use = idle/sum;

	printf("%f\%\r\n",(cpu_use)*100);
	return 0;
}

void get_cpuoccupy (CPU_OCCUPY *cpust) //��������get��������һ���βνṹ����Ū��ָ��O
{   
	FILE *fd;         
	int n;            
	char buff[256]; 
	CPU_OCCUPY *cpu_occupy;
	cpu_occupy=cpust;

	fd = fopen ("/proc/stat", "r"); 
	fgets (buff, sizeof(buff), fd);
	printf("%s\r\n",buff);
	sscanf (buff, "%s %u %u %u %u %u %u %u", cpu_occupy->name, &cpu_occupy->user, &cpu_occupy->nice,&cpu_occupy->system, &cpu_occupy->idle,&cpu_occupy->lowait,&cpu_occupy->irq,&cpu_occupy->softirq);
	printf("%s %u %u %u %u %u %u %u\r\n", cpu_occupy->name,cpu_occupy->user, cpu_occupy->nice,cpu_occupy->system, cpu_occupy->idle,cpu_occupy->lowait,cpu_occupy->irq,cpu_occupy->softirq);
	printf("%s %u\r\n", cpu_occupy->name,cpu_occupy->user);
	fclose(fd);     
}

int getFlashInfo(void)
{
    struct statfs diskInfo;
    statfs("/", &diskInfo);
    unsigned long long totalBlocks = diskInfo.f_bsize;
    unsigned long long totalSize = totalBlocks * diskInfo.f_blocks;
    size_t mbTotalsize = totalSize>>20;
    unsigned long long freeDisk = diskInfo.f_bfree*totalBlocks;
    size_t mbFreedisk = freeDisk>>20;
    printf ("/  total=%dMB, free=%dMB\n", mbTotalsize, mbFreedisk);



}

int main()
{
	CPU_OCCUPY cpu_stat1;
	CPU_OCCUPY cpu_stat2;
//	MEM_OCCUPY mem_stat;
	int cpu;
	//��һ�λ�ȡcpuʹ�����
	while(1)
{
	get_cpuoccupy((CPU_OCCUPY *)&cpu_stat1);
	sleep(1);

	//�ڶ��λ�ȡcpuʹ�����
	get_cpuoccupy((CPU_OCCUPY *)&cpu_stat2);

	//����cpuʹ����
	cpu = cal_cpuoccupy ((CPU_OCCUPY *)&cpu_stat1, (CPU_OCCUPY *)&cpu_stat2);
	printf("%d \r\n",cpu);	 
	//��ȡ�ڴ�
	get_meminfo();
	getFlashInfo();
}
	return 0;
} 
