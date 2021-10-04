#include "main.h"

volatile uint8_t t1_ms=0;
volatile uint16_t delay_ms=0;
eData kp;


void my_main(void)
{
  LL_IWDG_ReloadCounter(IWDG);
  Init_kp();
  LL_SYSTICK_EnableIT();
  RS485_Init();
	Service_9_Init();
  while (1)
  {
    if (t1_ms>=10)
    {
      LL_IWDG_ReloadCounter(IWDG);
      GSM_Serv();
      GNSS_Serv();
      Point_Make_Serv();
      Calendar_Serv();
      BlackBox_Serv();
      Wialon_IPS_serv();
			EGTS_Serv();
      Accel_serv();
      FOTA_Serv();
      iButton_Serv();
      LLS_Serv();
      Configurator_Serv();
			Service_9(); 
      t1_ms=0;
    }
  }
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
  if (hspi==&SPI_ACCEL)
  {
    accel.tx_complete=1;
  }
  if (hspi==&SPI_MEM)
  {
    blackbox.tx_complete=1;
  }
}
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
  if (hspi==&SPI_ACCEL)
  {
    accel.tx_complete=1;
  }
  if (hspi==&SPI_MEM)
  {
    blackbox.tx_complete=1;
  }
}
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  if (hspi==&SPI_ACCEL)
  {
    accel.tx_complete=1;
  }
  if (hspi==&SPI_MEM)
  {
    blackbox.tx_complete=1;
  }
}


