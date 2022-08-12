#include "himm.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

static HI_S32 s_s32MemDev = 0;
HI_S32 memopen(void)
{
    if (s_s32MemDev <= 0)
    {
        s_s32MemDev = open ("/dev/mem", O_CREAT|O_RDWR|O_SYNC);
        if (s_s32MemDev <= 0)
        {
            return -1;
        }
    }
    return 0;
}

HI_VOID memclose(void)
{
    close(s_s32MemDev);
}

void *memmap(HI_U32 u32PhyAddr, HI_U32 u32Size)
{
    HI_U32 u32Diff;
    HI_U32 u32PagePhy;
    HI_U32 u32PageSize;
    HI_U8 * pPageAddr;

    u32PagePhy = u32PhyAddr & 0xfffff000;
    u32Diff    = u32PhyAddr - u32PagePhy;

    u32PageSize = ((u32Size + u32Diff - 1) & 0xfffff000) + 0x1000;
    pPageAddr   = mmap ((void *)0, u32PageSize, PROT_READ|PROT_WRITE,
                                    MAP_SHARED, s_s32MemDev, u32PagePhy);
    if (MAP_FAILED == pPageAddr )
    {
        perror("mmap error\n");
        return NULL;
    }
    //printf("%08x, %08x\n", (HI_U32)pPageAddr, u32PageSize);
    return (void *) (pPageAddr + u32Diff);
}

HI_S32 memunmap(HI_VOID* pVirAddr, HI_U32 u32Size)
{
    HI_U32 u32PageAddr;
    HI_U32 u32PageSize;
    HI_U32 u32Diff;

    u32PageAddr = (((HI_U32)pVirAddr) & 0xfffff000);
    u32Diff     = (HI_U32)pVirAddr - u32PageAddr;
    u32PageSize = ((u32Size + u32Diff - 1) & 0xfffff000) + 0x1000;

    return munmap((HI_VOID*)u32PageAddr, u32PageSize);
}

HI_S32 Linux_ReadReg(HI_U32 u32Addr)
{
    HI_S32 s32Value = 0;
    HI_U32 *pPortVirAddr;
    pPortVirAddr = memmap(u32Addr,sizeof(u32Addr));
    s32Value     = *pPortVirAddr;
    memunmap(pPortVirAddr,sizeof(u32Addr));
    return s32Value;
}

HI_VOID Linux_SetReg(HI_U32 u32Addr,HI_S32 s32Value)
{
    HI_U32 *pPortVirAddr;
    pPortVirAddr  = memmap(u32Addr,sizeof(u32Addr));
    *pPortVirAddr = s32Value;
    memunmap(pPortVirAddr,sizeof(u32Addr));
}

#define himm(address, value) Linux_SetReg(address, value)
#define himd(address)        Linux_ReadReg(address)
