#include "main.h"

DUT_STRUC dut[DUT_COUNT]={0};
LLS_STRUC lls={0};
DUT_STRUC current_dut={0};


void LLS_Serv(void)
{
  if (lls.pars_index!=rs485.rx_index)
  {
    LLS_Parsing(rs485.buf_in[lls.pars_index]);
    if (++lls.pars_index>=RS485_BUF_SIZE)
      lls.pars_index=0;
  }
  if (++lls.processing_timer<LLS_PROCESSING_PERIOD) return;
  lls.processing_timer=0;
  switch (lls.cmd)
  {
    case NET_POLLING:
      Net_Polling();
      break;
    case ANSWER_WAITING:
      if (lls.cmd_timeout>LLS_PROCESSING_PERIOD)
        lls.cmd_timeout-=LLS_PROCESSING_PERIOD;
      else
      {
        lls.cmd=NET_POLLING;
        if (++dut[lls.dut_num].timeout>=DUT_TIMEOUT)
        {
          dut[lls.dut_num].timeout=0;
          dut[lls.dut_num].level=0;
          dut[lls.dut_num].freq=0;
          dut[lls.dut_num].temp=0;
        }
        if (++lls.dut_num>=kp.dut_count)
          lls.dut_num=0;
      }
      break;
    default:
      break;
  }
  
}
void Net_Polling(void)
{
  lls.crc_out=0;
  rs485.buf_out[0]=0x31;
  rs485.buf_out[1]=kp.dut_nums[lls.dut_num];
  rs485.buf_out[2]=0x06;
  for (uint8_t i=0; i<3; i++)
    lls.crc_out=LLS_CRC8(rs485.buf_out[i], lls.crc_out);
  rs485.buf_out[3]=lls.crc_out;
  rs485.buf_out[4]=0;
  pusk_485(rs485.buf_out, 4);
  lls.cmd=ANSWER_WAITING;
  lls.cmd_timeout=ANSWER_WAITING_TIMEOUT;
}
void LLS_Parsing(uint8_t b)
{
  switch (lls.state)
  {
    case 0:
      if (b==0x3E) 
      {
        lls.crc_in=0;
        lls.state++;
      }
      break;
    case 1:
      if (b==kp.dut_nums[lls.dut_num])
        lls.state++;
      else
        lls.state=0;
      break;
    case 2:
      if (b==0x06)
        lls.state++;
      else
        lls.state=0;
      break;
    case 3:
      current_dut.temp=(int8_t)(b);
      lls.state++;
      break;
    case 4:
      current_dut.level=(uint16_t)(b);
      lls.state++;
      break;
    case 5:
      current_dut.level|=(uint16_t)(b<<8);
      lls.state++;
      break;
    case 6:
      current_dut.freq=(uint16_t)(b);
      lls.state++;
      break;
    case 7:
      current_dut.freq|=(uint16_t)(b<<8);
      lls.state++;
      break;
    case 8:
      if (lls.crc_in==b)
      {
				dut[lls.dut_num].temp=current_dut.temp;
				dut[lls.dut_num].level=current_dut.level;
				dut[lls.dut_num].freq=current_dut.freq;
        dut[lls.dut_num].timeout=0;
        lls.cmd=NET_POLLING;
        lls.cmd_timeout=0;
        if (++lls.dut_num>=kp.dut_count)
          lls.dut_num=0;
        lls.state=0;
      }
      break;
    default:
      lls.state=0;
      break;
  }
  lls.crc_in=LLS_CRC8(b, lls.crc_in);
}
uint8_t LLS_CRC8(uint8_t data, uint8_t crc)
{
  uint8_t i = data ^ crc;
  crc = 0;
  if(i & 0x01) crc ^= 0x5e;
  if(i & 0x02) crc ^= 0xbc;
  if(i & 0x04) crc ^= 0x61;
  if(i & 0x08) crc ^= 0xc2;
  if(i & 0x10) crc ^= 0x9d;
  if(i & 0x20) crc ^= 0x23;
  if(i & 0x40) crc ^= 0x46;
  if(i & 0x80) crc ^= 0x8c;
  return crc;
}


