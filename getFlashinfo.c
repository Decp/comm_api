#include <stdio.h>                                                                                                                  
#include <stdlib.h>
#include <unistd.h>
#include <sys/vfs.h>

int main()
{
    struct statfs diskInfo;  
    statfs("/mnt/card/usb1", &diskInfo);  
    unsigned long long totalBlocks = diskInfo.f_bsize;  
    unsigned long long totalSize = totalBlocks * diskInfo.f_blocks;  
    size_t mbTotalsize = totalSize>>20;  
    unsigned long long freeDisk = diskInfo.f_bfree*totalBlocks;  
    size_t mbFreedisk = freeDisk>>20;  
    printf ("/  total=%dMB, free=%dMB\n", mbTotalsize, mbFreedisk);  

}
