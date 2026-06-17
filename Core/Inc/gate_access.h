#ifndef __GATE_ACCESS_H__
#define __GATE_ACCESS_H__

#include "main.h"
#include <stdint.h>

#define GATE_ACCESS_PASSWORD_LEN    6U    /* 6位数字密码 */
#define GATE_ACCESS_AUTH_TIMEOUT_MS  30000U /* 授权后30秒超时 */
#define GATE_ACCESS_DEFAULT_PASSWORD "123456"  /* 默认密码 */

void GateAccess_Init(void);
uint8_t GateAccess_VerifyPassword(const char *password);
uint8_t GateAccess_IsAuthorized(void);
void GateAccess_OpenGate(void);
void GateAccess_CloseGate(void);
uint8_t GateAccess_TryOpenGate(void);
const char *GateAccess_GetStatusString(void);

#endif /* __GATE_ACCESS_H__ */
