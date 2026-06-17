#include "gate_access.h"
#include "servo.h"
#include <string.h>

static char gate_access_password[GATE_ACCESS_PASSWORD_LEN + 1U];
static uint8_t gate_access_authorized = 0U;
static uint32_t gate_access_auth_timestamp = 0U;

void GateAccess_Init(void)
{
  strncpy(gate_access_password, GATE_ACCESS_DEFAULT_PASSWORD, GATE_ACCESS_PASSWORD_LEN);
  gate_access_password[GATE_ACCESS_PASSWORD_LEN] = '\0';
  gate_access_authorized = 0U;
  gate_access_auth_timestamp = 0U;
}

uint8_t GateAccess_VerifyPassword(const char *password)
{
  if (password == NULL)
  {
    return 0U;
  }

  if (strncmp(gate_access_password, password, GATE_ACCESS_PASSWORD_LEN) == 0)
  {
    gate_access_authorized = 1U;
    gate_access_auth_timestamp = HAL_GetTick();
    return 1U;
  }

  return 0U;
}

uint8_t GateAccess_IsAuthorized(void)
{
  if (gate_access_authorized != 0U)
  {
    uint32_t now = HAL_GetTick();
    if ((now - gate_access_auth_timestamp) >= GATE_ACCESS_AUTH_TIMEOUT_MS)
    {
      gate_access_authorized = 0U;
      return 0U;
    }
    return 1U;
  }
  return 0U;
}

void GateAccess_OpenGate(void)
{
  Servo_GateOpen();
}

void GateAccess_CloseGate(void)
{
  Servo_GateClose();
}

uint8_t GateAccess_TryOpenGate(void)
{
  if (GateAccess_IsAuthorized() != 0U)
  {
    GateAccess_OpenGate();
    return 1U;
  }
  return 0U;
}

const char *GateAccess_GetStatusString(void)
{
  return (GateAccess_IsAuthorized() != 0U) ? "UNLOCKED" : "LOCKED";
}
