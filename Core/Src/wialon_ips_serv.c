#include "main.h"

uint16_t  msg_crc=0;
char dop_buff[50], dop_buff2[50];
static uint8_t in_k=0, out_k=0;
uint16_t  log_crc=0;



void Wialon_IPS_serv(void)
{
	if ((kp.server_protocol!=WIALON_IPS_PROTOCOL)&&(kp.server_protocol!=SHORT_OUT)) return;
  if (gsm.pusk)
  {
    gsm.pusk=0;
    //GENERAL
    sprintf(gsm.msg,"#SD#%s;%s;%s;%s;%s;%s;%s;", 
		blackbox.date, blackbox.time, blackbox.fix_koord, blackbox.speed, blackbox.course, blackbox.altitude, blackbox.sats);//сокращённый пакет
		if (kp.server_protocol!=SHORT_OUT)
		{
			//HDOP
			sprintf(dop_buff, "%s;", blackbox.hdop);
			strncat(gsm.msg, dop_buff, strlen(dop_buff));
			//INPUTS OUTPUTS
			Add_InOuts();
			//IBUTTON
			if (blackbox.ibutton_key)
				sprintf(dop_buff, "%llu;", blackbox.ibutton_key);
			else
				sprintf(dop_buff, "NA;");
			strncat(gsm.msg, dop_buff, strlen(dop_buff));
			//PARAMS
			Add_LLS();
			sprintf(dop_buff, "msg_num:1:%d,", blackbox.read_lines);
			strncat(gsm.msg, dop_buff, strlen(dop_buff));
			sprintf(dop_buff, "SNR:1:%d,", blackbox.SNR);
			strncat(gsm.msg, dop_buff, strlen(dop_buff));
			sprintf(dop_buff, "qdrive:1:%d,", blackbox.qdrive);
			strncat(gsm.msg, dop_buff, strlen(dop_buff));
			sprintf(dop_buff, "pwr:2:%.1f,", 0.1*(float)blackbox.pwr);
			strncat(gsm.msg, dop_buff, strlen(dop_buff));
			sprintf(dop_buff, "temp:2:%.1f,", 0.1*(float)blackbox.temperature);
			strncat(gsm.msg, dop_buff, strlen(dop_buff));
			sprintf(dop_buff, "move:1:%d,", accel.move);
			strncat(gsm.msg, dop_buff, strlen(dop_buff));
			sprintf(dop_buff, "src:1:%d,", blackbox.point_make_src);
			strncat(gsm.msg, dop_buff, strlen(dop_buff));
			if (config.error)
			{
				sprintf(dop_buff, "error:1:%d,", config.error);
				strncat(gsm.msg, dop_buff, strlen(dop_buff)); 
			}
			sprintf(dop_buff, "version:1:%d;", VERSION);
			strncat(gsm.msg, dop_buff, strlen(dop_buff));   
		}
    msg_crc=crc16((uint8_t*)&gsm.msg[4], strlen(gsm.msg)-4);
    sprintf(dop_buff, "%X", msg_crc);
    strncat(gsm.msg, dop_buff, strlen(dop_buff));
		strncat (gsm.msg, "\r\n",strlen("\r\n"));

    gsm.send_length=strlen(gsm.msg);
  }
}
void Add_InOuts(void)
{
  in_k=0;
  out_k=0;
  if (in_kontakt.sk1) in_kontakt.sk1=1;
  if (in_kontakt.sk2) in_kontakt.sk2=1;
  switch (kp.mode_in1)
  {
    case DIGITAL_IN_MODE:
      in_k=in_kontakt.sk1;
      break;
    case FREQ_IN_MODE:
      break;
    case DIGITAL_OUT_MODE:
      out_k=in_kontakt.sk1;
      break;
    default:
      break;
  }
  switch (kp.mode_in2)
  {
    case DIGITAL_IN_MODE:
      in_k|=(in_kontakt.sk2<<1);
      break;
    case FREQ_IN_MODE:
      break;
    case DIGITAL_OUT_MODE:
      out_k|=(in_kontakt.sk2<<1);
      break;
    default:
      break;
  }
  sprintf(dop_buff, "%d;%d;%.3f,%.3f;", in_k, out_k, in_kontakt.ain1, in_kontakt.ain2);// 
  strncat(gsm.msg, dop_buff, strlen(dop_buff));  
}
void Add_LLS(void)
{
  for (uint8_t cc=0; cc<kp.dut_count; cc++)
  {
    sprintf(dop_buff, "lls_temp%d:1:%d,", cc, blackbox.temp[cc]);
    strncat(gsm.msg, dop_buff, strlen(dop_buff));
    sprintf(dop_buff, "lls_level%d:1:%d,", cc, blackbox.level[cc]);
    strncat(gsm.msg, dop_buff, strlen(dop_buff));
    sprintf(dop_buff, "lls_freq%d:1:%d,", cc, blackbox.freq[cc]);
    strncat(gsm.msg, dop_buff, strlen(dop_buff));
  }
  
}
uint16_t crc16 (uint8_t *data, uint16_t data_size)
{
  unsigned short crc = 0;
  unsigned char* buf = (unsigned char*)data;
  
  if (!data || !data_size)
    return 0;
  
  while (data_size--)
    crc = (crc >> 8) ^ crc16_table[(unsigned char)crc ^ *buf++];
  return crc;
}
uint16_t Wialon_IPS_Init_Login(void)
{
	sprintf((char*)gsm.msg,"#L#2.0;%s;%s;", gsm.imei, kp.pwd_serv);
	log_crc=crc16((uint8_t*)&gsm.msg[3], strlen((char*)gsm.msg)-3);
	sprintf(dop_buff, "%X", log_crc);
	strncat((char*)gsm.msg, dop_buff, strlen(dop_buff));
	strncat ((char*)gsm.msg, "\r\n",strlen("\r\n"));	
	return(strlen((char*)gsm.msg));
}


