#include "main.h"

volatile MEM_SPI_STRUC  blackbox={0};
BB_STRUC                bb;
static uint8_t          mem_rst=1;
uint32_t                bb_size=sizeof(bb);
uint8_t									unpack_res=0;
static uint32_t pos=0;
uint8_t	find_pos_cnt=0;
	
void BlackBox_Serv(void)
{
	if (blackbox.error&BLACKBOX_NOT_AVALIABLE) 
	{
		config.error|=BLACKBOX_NOT_WORKING;
		return;
	}
  if (mem_rst)
  {
		EraseMemory();
    FindPosition();
    return;
  }
  blackbox.timer+=t1_ms;
  if (blackbox.timer<BLACKBOX_PERIOD) return;
  blackbox.timer=0;
  
  if (gsm.ASD==1)
  {
    //пакет отправлен успешно
    gsm.ASD=0;
    bb.marker=SENDED;
    WriteMarker(blackbox.read_lines);
    if (++blackbox.read_lines>=BB_SIZE) blackbox.read_lines=0;
    blackbox.fail_count=0;
  }
  if ((!gsm.send_length)&&(!gsm.ASD)&&(gsm.log_in)&&(gsm.net_connected)&&(gsm.tcp_connected))
  {
    if (blackbox.read_lines!=blackbox.writed_lines)
		{
      ReadMarker(blackbox.read_lines);
      if (bb.marker==NOT_SENDED)
			{
        ReadBlackBox(blackbox.read_lines);
				unpack_res=Unpack_Struc();
				if (unpack_res)
				{
					gsm.pusk=1;
					gsm.ASD=2;
					gsm.asd_timeout=60000;
					if (++blackbox.fail_count>=10) 
					{
						gsm.ASD=1;
						blackbox.error|=SEND_MSG_ERROR;
					}
					
				} 
				else	if (++blackbox.fail_count>=10) 
				{
					gsm.ASD=1;
					blackbox.error|=BB_CRC_ERROR;
				}
      } 
			else 
			{
				gsm.ASD=1;
				blackbox.error|=MARKER_ERROR;
			}
    }
  }

  if ((blackbox.write)&&(calendar.actual))  
  {
    blackbox.write=0;
    Pack_Struct();
    WriteBlackBox(blackbox.writed_lines);
    if (++blackbox.writed_lines>=BB_SIZE) blackbox.writed_lines=0;
  }
}
void BlackBox_TimeOut(void)
{
  if (blackbox.timeout) blackbox.timeout--;
}
void FindPosition(void)
{
	switch (blackbox.find_pos_state)
	{
		case begin_find_position:
			pos=0;
			blackbox.writed_lines=0;
			blackbox.read_lines=0;
			ReadMarker(pos);
			blackbox.find_pos_state++;
			break;
		case finding1:
			while ((++find_pos_cnt<150)&&(blackbox.find_pos_state==finding1))
			{
				if ((bb.marker!=0xFF)&&(pos<BB_SIZE))
					ReadMarker(pos++);
				else
					blackbox.find_pos_state++;
			}
			find_pos_cnt=0;
			break;
		case finding2:
			if (!pos)
			{
				blackbox.writed_lines=0;
				blackbox.read_lines=0;
				mem_rst=0;
				return;
			}
			else
			{
				blackbox.writed_lines=pos-1;
				blackbox.read_lines=pos-2;
			}
			pos=0;
			ReadMarker(blackbox.read_lines);
			blackbox.find_pos_state++;
			break;
		case finding3:
			while ((++find_pos_cnt<150)&&(blackbox.find_pos_state==finding3))
			{
				if ((bb.marker==NOT_SENDED)&&(pos<BB_SIZE))
				{
					ReadMarker(--blackbox.read_lines);
					pos++;
					if (!blackbox.read_lines) blackbox.read_lines=BB_SIZE;
				} 
				else 
					blackbox.find_pos_state++;
			}
			find_pos_cnt=0;
			break;
		case finding_end:
			blackbox.read_lines++;
			blackbox.find_pos_state=0;
			mem_rst=0;
			break;
		default:
			blackbox.find_pos_state=0;
			break;
	}
}
void Write_Enable(void)
{
  blackbox.status=1;
	while ((blackbox.status)) CheckStatus();
	blackbox.buf_out[0]=0x06;
	MEM_Spi_Run(1);
  blackbox.status=1;
	while ((blackbox.status)) CheckStatus();
}
void Write_Disable(void)
{
  blackbox.status=1;
	while ((blackbox.status)) CheckStatus();
	blackbox.buf_out[0]=0x04;
	MEM_Spi_Run(1);
  blackbox.status=1;
	while ((blackbox.status)) CheckStatus();
}
void Pack_Struct(void)
{
  char buff[10];
  uint16_t H=0;
	
  bb.marker=NOT_SENDED;
  sprintf((char*)blackbox.date, "%02d%02d%02d", calendar.day, calendar.month, calendar.year);
  sprintf((char*)blackbox.time, "%02d%02d%02d", calendar.hour, calendar.min, calendar.sec);
 	bb.time=(uint32_t)atoi((char*)blackbox.time);
	bb.date=(uint32_t)atoi((char*)blackbox.date);
	bb.temperature=(int16_t)(in_kontakt.temperature*10);
  bb.qdrive=(uint16_t)accel.quality_drive;   
  bb.pwr=(uint16_t)(in_kontakt.pwr*10); 
	bb.Hdop=20; 

  if ((!my_str_cmp(point_make.fix_koord, strlen(point_make.fix_koord), "NA", strlen("NA")))&&(gnss.status=='A'))
	{
		cpy_bet_num(buff,point_make.fix_koord, 0,1); 
		bb.shirota=(uint32_t)(atof(buff)*10000);
		cpy_bet_num(buff,point_make.fix_koord, 1,2);
		if (buff[0]=='N') bb.shirota |= 0x80000000;
		cpy_bet_num(buff,point_make.fix_koord, 2,3);
		bb.lon=(uint32_t)(10000*atof(buff));
		cpy_bet_num(buff,point_make.fix_koord, 3,4);
		if (buff[0]=='E') bb.lon |= 0x80000000; 
		
		if (accel.move)
			bb.Vel=(uint8_t)(atof(gnss.speed)*1.85);
		else
			bb.Vel=0;
		bb.Asimut=(uint8_t)(atof(gnss.course)/2);
		H=(uint16_t)atoi(gnss.altitude);
		if (H<18000)
			bb.Hight=H;
		bb.Sputn=gnss.sats;
		bb.SNR=gnss.snr;
		if(bb.Sputn) { bb.Hdop=atof(gnss.hdop)*10; }
	}
	else
	{
		bb.shirota=0xFFFFFFFF;
		bb.lon=0xFFFFFFFF;
	}  
  for (uint8_t cc=0; cc<DUT_COUNT; cc++)
  {
    bb.temp[cc]=dut[cc].temp;
    bb.level[cc]=dut[cc].level;
    bb.freq[cc]=dut[cc].freq;
  }
  
  bb.ibutton_key=ibutton.key;
  bb.sk1=in_kontakt.sk1;
  bb.sk2=in_kontakt.sk2;
  bb.ain1=(uint16_t)(in_kontakt.ain1*1000);
  bb.ain2=(uint16_t)(in_kontakt.ain2*1000);
  bb.freq1=in_kontakt.freq1;
  bb.freq2=in_kontakt.freq2;
  bb.mode1=kp.mode_in1;
  bb.mode2=kp.mode_in2;
  bb.point_make_src=point_make.point_make_src;
  point_make.point_make_src=0;
  bb.crc=crc_str((char*)&bb.marker, sizeof(bb)-1);

}
uint8_t Unpack_Struc(void)
{
	
	if (bb.crc!=crc_str((char*)&bb.marker, sizeof(bb)-1)) return 0;
  if (bb.time<100000)
		sprintf((char*)blackbox.time, "0%d", bb.time);
	else
		sprintf((char*)blackbox.time, "%d", bb.time);
	if (bb.date<100000)
		sprintf((char*)blackbox.date, "0%d", bb.date);
	else
		sprintf((char*)blackbox.date, "%d", bb.date);
	if ((bb.lon!=0xFFFFFFFF)&&(bb.shirota!=0xFFFFFFFF))
	{
		blackbox.gnss_status='A';
		if ((float)(bb.lon & 0x7fffffff)>0)
		{
			sprintf((char*)blackbox.longitude, "%010.4f", (float)(bb.lon & 0x7fffffff)/10000);
			if(bb.lon & 0x80000000) 
        sprintf((char*)blackbox.lon,"E");
      else 
        sprintf((char*)blackbox.lon,"W");
		}
		if ((float)(bb.shirota & 0x7fffffff)>0)
		{
			sprintf((char*)blackbox.latitude, "%09.4f", (float)(bb.shirota & 0x7fffffff)/10000);
			if(bb.shirota & 0x80000000) 
        sprintf((char*)blackbox.lat,"N");
      else 
        sprintf((char*)blackbox.lat,"S");
		}	
		sprintf((char*)blackbox.fix_koord,"%s;%s;%s;%s",blackbox.latitude, blackbox.lat, blackbox.longitude, blackbox.lon);
		sprintf((char*)blackbox.speed, "%d", bb.Vel);
		sprintf((char*)blackbox.course, "%d", bb.Asimut*2);
		sprintf((char*)blackbox.altitude, "%d", bb.Hight);
		sprintf((char*)blackbox.sats, "%d", bb.Sputn & 0x7F);
		sprintf((char*)blackbox.hdop, "%.2f", (float)bb.Hdop/10);
	}
  else
  {
		blackbox.gnss_status='V';
		sprintf((char*)blackbox.fix_koord,"NA;NA;NA;NA");
		sprintf((char*)blackbox.speed, "NA");
		sprintf((char*)blackbox.course, "NA");
		sprintf((char*)blackbox.altitude, "NA");
		sprintf((char*)blackbox.sats, "NA");
		sprintf((char*)blackbox.hdop, "NA");
  }
  
  blackbox.SNR=bb.SNR;
	blackbox.Adc=bb.Adc;
	blackbox.pwr=bb.pwr;
	blackbox.temperature=bb.temperature;
	blackbox.qdrive=bb.qdrive;
  blackbox.ibutton_key=bb.ibutton_key;
  blackbox.sk1=bb.sk1;
  blackbox.sk2=bb.sk2;
  blackbox.ain1=bb.ain1;
  blackbox.ain2=bb.ain2;
  blackbox.freq1=bb.freq1;
  blackbox.freq2=bb.freq2;
  blackbox.mode1=bb.mode1;
  blackbox.mode2=bb.mode2;
  blackbox.point_make_src=bb.point_make_src;
  for (uint8_t cc=0; cc<DUT_COUNT; cc++)
  {
    blackbox.temp[cc]=bb.temp[cc];
    blackbox.level[cc]=bb.level[cc];
    blackbox.freq[cc]=bb.freq[cc];
  }
  return 1;
}
void WriteBlackBox(uint16_t str_num)
{
  //str_num eto nomer stroki vnutri yashika
	uint8_t* 	addr=0;
	uint32_t fl=0;
  uint16_t  len=0;
	uint32_t 	bb_address; 	//address structuri vnutri microshemi pamyati
	addr=&bb.marker;
	bb_address=str_num*(uint32_t)(LEN_PACK);
  len=sizeof(bb);
	if (LEN_PACK<len)
	{
		return;
	}
	fl=bb_address&0x0000FFFF;
	if (!fl)
	{
//		eat_trace("new sector 0x%X", bb_address);
		EraseSector(bb_address);
	}
  while (len>0)
  {
    if (len>8)
    {
      //zapis' structuri v pamyat' po adresu bb_address
      SPI_WriteBuff(bb_address, addr, 8);
      bb_address+=8;
      addr+=8;
      len-=8;
    } else
    {
      SPI_WriteBuff(bb_address, addr, len);
      len=0;
    }
  }

}
void ReadBlackBox(uint16_t str_num)
{
	uint8_t* 	addr=0;
	uint32_t 	bb_address; 	//address structuri vnutri microshemi pamyati
  uint16_t len=0;
	addr=&bb.marker;
	bb_address=str_num*(uint32_t)(LEN_PACK);	 
  len=sizeof(bb);
  while (len>0)
  {
    if (len>8)
    {
      //zapis' structuri v pamyat' po adresu bb_address
      SPI_ReadBuff(bb_address, addr, 8);
      bb_address+=8;
      addr+=8;
      len-=8;
    } else
    {
      SPI_ReadBuff(bb_address, addr, len);
      len=0;
    }
  }
	//chtenie structuri po adresu bb_address
//	SPI_ReadBuff(bb_address, addr, sizeof(bb));
}
void WriteMarker(uint16_t str_num)
{
	uint32_t 	bb_address; 	//address structuri vnutri microshemi pamyati
	bb_address=str_num*(uint32_t)(LEN_PACK);	
	SPI_WriteBuff(bb_address, &bb.marker, 2);
}
void ReadMarker(uint16_t str_num)
{
	uint32_t 	bb_address; 	//address structuri vnutri microshemi pamyati
	bb_address=str_num*(uint32_t)(LEN_PACK);	
	SPI_ReadBuff(bb_address, &bb.marker, 1);
}
void EraseSector(uint32_t addr)
{
  Write_Enable();
	//erase sector
	blackbox.buf_out[0]=0xD8;
	blackbox.buf_out[1]= (uint8_t)((addr >> 16) & 0x000000FF);
	blackbox.buf_out[2]= (uint8_t)((addr >> 8) & 0x000000FF);
	blackbox.buf_out[3]= (uint8_t)(addr & 0x000000FF);
	MEM_Spi_Run(4);
  Write_Disable();
}
void EraseMemory(void)
{
  Write_Enable();
	//full erase
	blackbox.buf_out[0]=0xC7;
	MEM_Spi_Run(1);
  blackbox.status=1;
	while ((blackbox.status)) 
  {
    CheckStatus();
    LL_IWDG_ReloadCounter(IWDG);
  }
	Write_Disable();
}
void CheckStatus(void)
{
	blackbox.buf_out[0]=0x05;
  MEM_Spi_Run(2);
  blackbox.status=blackbox.buf_in[1]&0x01;
}
void SPI_WriteBuff(uint32_t bb_addr, uint8_t* data, uint8_t len)
{
	uint8_t i=0;
  Write_Enable();
	//write data	
	blackbox.buf_out[0]= 0x02;
	blackbox.buf_out[1]= (uint8_t)((bb_addr >> 16) & 0x000000FF);
	blackbox.buf_out[2]= (uint8_t)((bb_addr >> 8) & 0x000000FF);
	blackbox.buf_out[3]= (uint8_t)(bb_addr & 0x000000FF);
	for (i=0; i<len; i++)
		blackbox.buf_out[i+4]= data[i];
	MEM_Spi_Run(len+4);
  Write_Disable();
}
void SPI_ReadBuff(uint32_t bb_addr, uint8_t* data, uint8_t len)
{
	uint8_t i=0;
	//read status register
  blackbox.status=1;
	while ((blackbox.status)) CheckStatus();
	//read data
	blackbox.buf_out[0]= 0x03;
	blackbox.buf_out[1]= (uint8_t)((bb_addr >> 16) & 0x000000FF);
	blackbox.buf_out[2]= (uint8_t)((bb_addr >> 8) & 0x000000FF);
	blackbox.buf_out[3]= (uint8_t)(bb_addr & 0x000000FF);
  MEM_Spi_Run(len+4);
	for (i=0; i<len; i++)
		data[i]=blackbox.buf_in[i+4];		
}	
void MEM_Spi_Run(uint8_t rxtx_len)
{
  MEM_CS_RESET();  
  for (uint8_t zad=0; zad<200; zad++) {};
  blackbox.tx_complete=0;
  blackbox.timeout=BLACKBOX_TIMEOUT;
  HAL_SPI_TransmitReceive_IT(&SPI_MEM, (uint8_t *)&blackbox.buf_out[0], (uint8_t *)&blackbox.buf_in[0], rxtx_len);
  while ((!blackbox.tx_complete)&&(blackbox.timeout)) {};
	if (!blackbox.timeout) 
		blackbox.timeout_counter++;
	else 
		blackbox.timeout_counter=0;
	if (blackbox.timeout_counter>BLACKBOX_TIMEOUT_NUM) blackbox.error|=BLACKBOX_NOT_AVALIABLE;
  MEM_CS_SET();
  for (uint8_t zad=0; zad<200; zad++) {};
}

