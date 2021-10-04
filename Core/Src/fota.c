#include "main.h"

FOTA_STRUC fota={0};

void FOTA_Serv(void)
{
  if ((fota.update_program)&&(fota.download_ok)) 
  {
    GSM_PWR_OFF();
    GSM_PWRKEY_OFF();
    while (1)
    {
    }
  }
}

