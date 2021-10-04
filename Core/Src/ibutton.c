#include "main.h"

IBUTTON_STRUC ibutton;
static uint64_t last_key=0;
const uint8_t iButton_crcTable[] = {
	0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
	157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
	35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
	190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
	70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
	219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
	101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
	248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
	140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
	17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
	175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
	50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
	202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
	87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
	233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
	116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53
};

void iButton_Serv(void)
{
  ibutton.ms_timer+=t1_ms;
  if ((ibutton.ms_timer<iBUTTON_DURATION)&&(!ibutton.processing))                           Start_Reading();
  if ((ibutton.ms_timer>iBUTTON_DURATION)&&(ibutton.processing)&&(ibutton.cmd<READ_ROM))    Stop_Reading();
  if (ibutton.ms_timer>iBUTTON_PERIOD) ibutton.ms_timer=0;
  if (ibutton.led_timer)
  {
    
    if (ibutton.led_timer>=t1_ms) 
      ibutton.led_timer-=t1_ms;
    if (ibutton.led_timer<t1_ms) 
    {
      LL_GPIO_ResetOutputPin(LED_G_GPIO_Port, LED_G_Pin);
      ibutton.led_timer=0;
    } 
  }
  while ((ibutton.processing)&&(ibutton.cmd>PRESENCE_END)) {};
}

uint8_t iButtonCRC(void)
{
	uint8_t i,crc,tmp;
	for(i=0,crc=0;i<7;i++)
	{
		tmp = crc ^ ibutton.rx_byte[i];
		crc = iButton_crcTable[tmp];
	}
	/*if(crc==iButtonData[7])
	return(1);
	return(0); */ //this can be used to accept only valid codes read since
	// iButtonData[7] is actually CRC data read from iButton that should be the
	//same number as calculated CRC using provided function
	return(crc);
}
void Start_Reading(void)
{
  ibutton.processing=1;
  ibutton.cmd=NOT_RUNNING;
  ONE_WIRE_TX_PULL_UP();
  LL_TIM_DisableCounter(iBUTTON_TIMER);
  LL_EXTI_DisableIT_0_31(iBUTTON_RX_LINE);
  LL_TIM_SetAutoReload(iBUTTON_TIMER, 500);
  LL_TIM_SetCounter(iBUTTON_TIMER, 1);
  iBUTTON_TIMER->SR=0;
  LL_TIM_EnableIT_UPDATE(iBUTTON_TIMER);
  LL_TIM_EnableCounter(iBUTTON_TIMER);
}
void Stop_Reading(void)
{
  LL_TIM_DisableCounter(iBUTTON_TIMER);
  LL_EXTI_DisableIT_0_31(iBUTTON_RX_LINE);
  ibutton.cmd=0;
  ibutton.processing=0;
}
void iButton_Timer_Handler(void)
{
  if (LL_TIM_IsActiveFlag_UPDATE(iBUTTON_TIMER))
  {
    iBUTTON_TIMER->SR=0;
    LL_TIM_SetCounter(iBUTTON_TIMER, 1);
    switch (ibutton.cmd)
    {
      case NOT_RUNNING:
        ibutton.cmd=PRESENCE_BEGIN;
        ONE_WIRE_TX_PULL_DOWN();
        LL_EXTI_DisableIT_0_31(iBUTTON_RX_LINE);
        LL_TIM_SetAutoReload(iBUTTON_TIMER, 500);
        break;
      case PRESENCE_BEGIN:
        ibutton.rx_changed=0;
        LL_EXTI_EnableIT_0_31(iBUTTON_RX_LINE);
        ibutton.cmd=PRESENCE_END;
        LL_TIM_SetAutoReload(iBUTTON_TIMER, 60);
        ONE_WIRE_TX_PULL_UP();
        break;
      case PRESENCE_END:
        LL_EXTI_DisableIT_0_31(iBUTTON_RX_LINE);
        LL_TIM_SetAutoReload(iBUTTON_TIMER, 200);
        if ((!ibutton.rx)&&(ibutton.rx_changed))
        {
          ibutton.rx_changed=0;
          ibutton.cmd=READ_ROM;
          ibutton.tx_byte=0x33;
          ibutton.tx_cnt=0;
          ibutton.rx_byte[0]=0;
          ibutton.rx_byte[1]=0;
          ibutton.rx_byte[2]=0;
          ibutton.rx_byte[3]=0;
          ibutton.rx_byte[4]=0;
          ibutton.rx_byte[5]=0;
          ibutton.rx_byte[6]=0;
          ibutton.rx_byte[7]=0;
          ibutton.rx_bite_cnt=0;
          ibutton.rx_byte_cnt=0;
        }
        else
        {
          ibutton.rx_changed=0;
          ibutton.cmd=NOT_RUNNING;
        }
        break;
      case READ_ROM:
        ibutton.cmd=TX_WAIT;
        ONE_WIRE_TX_PULL_DOWN();
        LL_TIM_SetAutoReload(iBUTTON_TIMER, 10);
        break;
      case TX_WAIT:
        if ((ibutton.tx_byte>>ibutton.tx_cnt)&0x01)
           ONE_WIRE_TX_PULL_UP();
        else
          ONE_WIRE_TX_PULL_DOWN();
        if (++ibutton.tx_cnt>8) 
        {
          ibutton.cmd=READ_INIT;
        }
        else
          ibutton.cmd=TX_RESET;
        LL_TIM_SetAutoReload(iBUTTON_TIMER, 120);
        break;
      case TX_RESET:
        ONE_WIRE_TX_PULL_UP();
        ibutton.cmd=READ_ROM;
        break;
      case READ_INIT:
        ONE_WIRE_TX_PULL_DOWN();
        ibutton.cmd=READ;
        LL_TIM_SetAutoReload(iBUTTON_TIMER, 7);
        break;
      case READ:
        ONE_WIRE_TX_PULL_UP();
        ibutton.cmd=READING;
        LL_TIM_SetAutoReload(iBUTTON_TIMER, 8);
        break;
      case READING:
        ibutton.rx=(uint8_t)LL_GPIO_IsInputPinSet(ONE_WIRE_RX_GPIO_Port, ONE_WIRE_RX_Pin);
        ibutton.rx_byte[ibutton.rx_byte_cnt]|=(ibutton.rx<<ibutton.rx_bite_cnt);
        ibutton.cmd=READ_END;
        if (++ibutton.rx_bite_cnt==8)
        {
          ibutton.rx_bite_cnt=0;
          if (++ibutton.rx_byte_cnt==8)
            ibutton.cmd=END;
        }
        LL_TIM_SetAutoReload(iBUTTON_TIMER, 150);
        break;
      case READ_END:
        ONE_WIRE_TX_PULL_UP();
        LL_TIM_SetAutoReload(iBUTTON_TIMER, 50);
        ibutton.cmd=READ_INIT;
        break;
      case END:
        LL_TIM_DisableCounter(iBUTTON_TIMER);
        LL_EXTI_DisableIT_0_31(iBUTTON_RX_LINE);
        if (ibutton.rx_byte[7]==iButtonCRC())
        {
          ibutton.led_timer=1000;
          LL_GPIO_SetOutputPin(LED_G_GPIO_Port, LED_G_Pin); 
          ibutton.key=0;
          for (ibutton.rx_byte_cnt=0; ibutton.rx_byte_cnt<5; ibutton.rx_byte_cnt++)
            ibutton.key|=((uint64_t)ibutton.rx_byte[ibutton.rx_byte_cnt+1])<<(8*ibutton.rx_byte_cnt);
          if (last_key!=ibutton.key)
          {
            ibutton.changed=1;
            last_key=ibutton.key;
          }
        }
        ibutton.rx_byte_cnt=0;
        ibutton.cmd=0;
        ibutton.processing=0;
        Stop_Reading();
        break;
      default:
        break;
    }
  }
}
void iButton_EXTI_Handler(void)
{
  ibutton.rx=(uint8_t)LL_GPIO_IsInputPinSet(ONE_WIRE_RX_GPIO_Port, ONE_WIRE_RX_Pin);
  ibutton.rx_changed=1;
}



