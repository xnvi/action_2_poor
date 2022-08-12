#ifndef __HIMM_H
#define __HIMM_H

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

#include "hi_type.h"

HI_S32 memopen(void);
HI_VOID memclose(void);
HI_S32 Linux_ReadReg(HI_U32 u32Addr);
HI_VOID Linux_SetReg(HI_U32 u32Addr,HI_S32 s32Value);

#define himm(address, value) Linux_SetReg(address, value)
#define himd(address)        Linux_ReadReg(address)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
