#ifndef __CURTAIN_H__
#define __CURTAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

typedef enum
{
    CURTAIN_STATE_STOP = 0,
    CURTAIN_STATE_FORWARD,
    CURTAIN_STATE_REVERSE
} CurtainState_t;

void Curtain_Init(void);
void Curtain_Forward(void);
void Curtain_Reverse(void);
void Curtain_Stop(void);
CurtainState_t Curtain_GetState(void);

#ifdef __cplusplus
}
#endif

#endif
