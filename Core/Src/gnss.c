#include "main.h"

GNSS_USART_STRUC  gnss_usart;
GNSS_PARS_STRUC   gnss;
static uint8_t* tage1_str;


void GNSS_Serv(void)
{
  gnss.pars_timer+=t1_ms;
	gnss.saving_timer+=t1_ms;
	if (gnss.cold_start_timer<=COLD_START_TIMEOUT)
		gnss.cold_start_timer+=t1_ms;
  if (gnss.pars_timer<PARS_PERIOD) return;
  gnss.pars_timer=0;
	
	
	
  if (!gnss_usart.on) GNSS_On(); 
	if ((gnss_usart.on)&&(gnss.cold_start_timer>WARM_START_TIMEOUT)&&(gnss.start_type==HOT_START)) 	Warm_Start();
	if ((gnss_usart.on)&&(gnss.cold_start_timer>COLD_START_TIMEOUT)&&(gnss.start_type==WARM_START)) Cold_Start();
	
  GNSS_Parsing();
  if (good_koord())
    Get_Position_Data();
	if ((gnss_usart.on)&&(!gnss_usart.data_avaliable)) config.error|=GNSS_NOT_WORKING;
	
	if ((gnss.sats>=5)&&(gnss.saving_timer>FLASH_SAVE_PERIOD))	GNSS_Save_FL();
	if (gnss.sats<4) gnss.saving_timer=0;
}
void Warm_Start(void)
{
	sprintf((char*)gnss_usart.buf_out, "$GPSGG,CSTART*6B");
	Send_Cmd_GNSS(gnss_usart.buf_out, strlen((char*)gnss_usart.buf_out));
	gnss.start_type=WARM_START;
}
void Cold_Start(void)
{
	sprintf((char*)gnss_usart.buf_out, "$GPSGG,WSTART*7F");
	Send_Cmd_GNSS(gnss_usart.buf_out, strlen((char*)gnss_usart.buf_out));
	gnss.start_type=COLD_START;
}
void GNSS_Save_FL(void)
{
	gnss.saving_timer=0;
	sprintf((char*)gnss_usart.buf_out, "$GPSGG,SAVEFL*63");
	Send_Cmd_GNSS(gnss_usart.buf_out, strlen((char*)gnss_usart.buf_out));
}
void GNSS_Data_Clear(void)
{
  sprintf((char*)gnss.altitude, "NA");
  sprintf((char*)gnss.fix_koord, "NA;NA;NA;NA");
  sprintf((char*)gnss.speed, "NA");
  sprintf((char*)gnss.course, "NA");
  sprintf((char*)gnss.hdop, "NA");
  gnss.status='V';
	gnss.snr=0;
	gnss.sats=0;
}
void Get_Position_Data(void)
{
  static char mysrg[30];
  uint8_t num=0, snr=0, msg_num=0, pos=0, cc=0, min_snr=200, min_snr_num=0;
  long  snr_srednee;
  if(gnss.fl_gps_str & GGA_STR)  
  {  
    gnss.fl_gps_str &= ~GGA_STR;
    cpy_bet_num(mysrg,(char*)gnss.buf_gga, 7,8);	   		
    gnss.sats=atoi(mysrg); 
		cpy_bet_num((char*)gnss.hdop, (char*)gnss.buf_gga, 8, 9);
		cpy_bet_num((char*)gnss.altitude, (char*)gnss.buf_gga, 9, 10);
  } 
  if(gnss.fl_gps_str & GSV_STR)  
  { 
    gnss.fl_gps_str &= ~GSV_STR;
    cpy_bet_num(mysrg,(char*)gnss.buf_gsv1, 1,2);
    msg_num=atoi(mysrg);	
    while (msg_num)
    {
      cpy_bet_num(mysrg,(char*)gnss.buf_gsv1, pos+4,pos+5);
      num=atoi(mysrg);
      cpy_bet_num(mysrg,(char*)gnss.buf_gsv1, pos+7,pos+8);
      snr=atoi(mysrg);
      for (cc=0; cc<SNR_MASS_SIZE; cc++)
      {
          //поиск минимального SNR
          if (gnss.snr_mass[cc]<min_snr)
          {
            min_snr=gnss.snr_mass[cc];
            min_snr_num=cc;
          }
          if (gnss.sats_mass[cc]==num)
          {
            min_snr_num=cc;
            break;
          }
      }
      gnss.sats_mass[min_snr_num]=num;
      gnss.snr_mass[min_snr_num]=snr;
      msg_num--;
      pos+=4;
    } 
    snr_srednee=0;
    num=0;
    for (cc=0; cc<SNR_MASS_SIZE; cc++)
    {
      if ((gnss.sats_mass[cc])&&(gnss.snr_mass[cc]))
      {
        snr_srednee+=gnss.snr_mass[cc];
        num++;
      }
    }
    gnss.snr=(uint8_t)(snr_srednee/num);
  }
  
  if(gnss.fl_gps_str & RMC_STR)  
  {  
    gnss.fl_gps_str &= ~RMC_STR;
		
    cpy_bet_num((char*)gnss.speed, (char*)gnss.buf_rmc, 7, 8);
    cpy_bet_num((char*)gnss.course, (char*)gnss.buf_rmc, 8, 9);
    
    cpy_bet_num((char*)gnss.time, (char*)gnss.buf_rmc, 1, 2);
    cpy_bet_num((char*)gnss.date, (char*)gnss.buf_rmc, 9, 10);
		if (!calendar.actual) calendar.update_from_gnss=1;
  }
  gnss.fl_gps_str &= ~GPS_STR_READY;
}
void GNSS_Parsing(void)
{
  uint8_t ing2=0;
  //копирование во второй буфер
  gnss_usart.rx_index=(uint16_t)GNSS_RX_BUF_SIZE-(uint16_t)LL_DMA_GetDataLength(DMA_GNSS, DMA_GNSS_RX);
	if (gnss_usart.rx_index>30) gnss_usart.data_avaliable=1;
  while (gnss_usart.rx_index!=gnss_usart.rd_index) 
  {
    gnss_usart.buf_in2[gnss_usart.rd_index]=gnss_usart.buf_in[gnss_usart.rd_index];
    if (++gnss_usart.rd_index>=GNSS_RX_BUF_SIZE2) gnss_usart.rd_index=0;
  }
  //раскидываем строки RMC, GGA и GSV по отдельным буферам
  while (gnss_usart.pars_index!=gnss_usart.rd_index)
  {
    ing2=gnss_usart.buf_in2[gnss_usart.pars_index];
    if (ing2 == '$') 
    {   
      gnss.cntr_buf_nmea=0;	 	
      gnss.cnt_out=1; 	
    }
    if(gnss.cnt_out==1) 
    {
      if (gnss.cntr_buf_nmea==4)  
      {
        if(ing2=='G')
        { 
          tage1_str=gnss.buf_gga; 
          gnss.fl_gps_str |=GGA_STR; 
          gnss.cnt_out=2; 
          gnss.ogr_str_gps=0; 
        }
        if(ing2=='S')
        { 
          tage1_str=gnss.buf_gsv1; 
          gnss.fl_gps_str |=GSV_STR; 
          gnss.cnt_out=2; 
          gnss.ogr_str_gps=0; 
        }
      }
      if (gnss.cntr_buf_nmea==3)  
      {
        if(ing2=='R')
        { 
          tage1_str=gnss.buf_rmc; 
          gnss.fl_gps_str |=RMC_STR;  
          gnss.cnt_out=2; 
          gnss.ogr_str_gps=0;	
					gnss.rmc_debug=1;
        }
      }
    } 
    if(gnss.cnt_out==2) 
    { 
      *(tage1_str++)=ing2;
      if(ing2 == 0x0D)  gnss.cnt_out=3;                       
      if(++gnss.ogr_str_gps > 88) gnss.cnt_out=0; //перебор в строке - потеря конца и сброс
    }
    if (ing2 == 0x0A && gnss.cnt_out==3) 
    {	
      gnss.cnt_out=0; 
      *(tage1_str++)=ing2; 
      *tage1_str=0;  //вбиваем конец строки
      gnss.fl_gps_str |= GPS_STR_READY;	
			if ((gnss.rmc_debug)&&(config.debug_out))
			{
				gnss.rmc_debug=0;
				pusk_usb(gnss.buf_rmc,  strlen((char*)gnss.buf_rmc));
			}
    }
    if (gnss.cntr_buf_nmea <10) gnss.cntr_buf_nmea++;   
    if (++gnss_usart.pars_index>=GNSS_RX_BUF_SIZE2) gnss_usart.pars_index=0;
  }
}
void GNSS_On(void)
{  
	uint8_t* g_struc=(uint8_t*)&gnss.cntr_buf_nmea;
	uint16_t  g_len=(uint16_t)sizeof(gnss);
	switch (gnss_usart.state)
	{
		case PERIPH_PREPARE:
			for (uint16_t cc=0; cc<g_len; cc++)
				*g_struc++=0;
			GNSS_Data_Clear();
			gnss_usart.rd_index=0;
			gnss_usart.rx_index=0;
			gnss_usart.pars_index=0;
			gnss_usart.ready=1;
			LL_GPIO_SetOutputPin(GNSS_EN_GPIO_Port, GNSS_EN_Pin);
			LL_GPIO_SetOutputPin(GNSS_RST_GPIO_Port, GNSS_RST_Pin);
			LL_GPIO_SetOutputPin(GNSS_ON_OFF_GPIO_Port, GNSS_ON_OFF_Pin);
			LL_DMA_DisableChannel(DMA_GNSS, DMA_GNSS_RX);
			DMA_GNSS_RX_CLEAR_FLAG();
			DMA_GNSS_RX_ERROR_CLEAR(); 
			LL_USART_EnableDMAReq_RX(USART_GNSS);  
			DMA_GNSS_RX_CLEAR_FLAG();
			DMA_GNSS_RX_ERROR_CLEAR();
			DMA_GNSS_GI_FLAG_CLEAR();
			LL_DMA_ConfigAddresses(DMA_GNSS, DMA_GNSS_RX, LL_USART_DMA_GetRegAddr(USART_GNSS, LL_USART_DMA_REG_DATA_RECEIVE), (uint32_t)&gnss_usart.buf_in[0], LL_DMA_GetDataTransferDirection(DMA_GNSS, DMA_GNSS_RX));
			LL_DMA_DisableChannel(DMA_GNSS, DMA_GNSS_RX);
			LL_DMA_SetDataLength(DMA_GNSS, DMA_GNSS_RX, GNSS_RX_BUF_SIZE);
			LL_DMA_EnableChannel(DMA_GNSS, DMA_GNSS_RX);
			break;
		case SET_HOT_START:
			sprintf((char*)gnss_usart.buf_out, "$GPSGG,HSTART*60");
			Send_Cmd_GNSS(gnss_usart.buf_out, strlen((char*)gnss_usart.buf_out));
			gnss.start_type=HOT_START;
			break;
		case GLL_OFF:
			sprintf((char*)gnss_usart.buf_out, "$GPSGG,GLLOFF*60");
			Send_Cmd_GNSS(gnss_usart.buf_out, strlen((char*)gnss_usart.buf_out));
			break;
		case GSA_OFF:
			sprintf((char*)gnss_usart.buf_out, "$GPSGG,GSAOFF*72");
			Send_Cmd_GNSS(gnss_usart.buf_out, strlen((char*)gnss_usart.buf_out));
			break;
		case VTG_OFF:
			sprintf((char*)gnss_usart.buf_out, "$GPSGG,VTGOFF*62");
			Send_Cmd_GNSS(gnss_usart.buf_out, strlen((char*)gnss_usart.buf_out));
			break;
		case ZDA_OFF:
			sprintf((char*)gnss_usart.buf_out, "$GPSGG,ZDAOFF*78");
			Send_Cmd_GNSS(gnss_usart.buf_out, strlen((char*)gnss_usart.buf_out));
			break;
		case DTM_OFF:
			sprintf((char*)gnss_usart.buf_out, "$GPSGG,DTMOFF*7A");
			Send_Cmd_GNSS(gnss_usart.buf_out, strlen((char*)gnss_usart.buf_out));
			break;
		case RLM_OFF:
			sprintf((char*)gnss_usart.buf_out, "$GPSGG,RLMOFF*74");
			Send_Cmd_GNSS(gnss_usart.buf_out, strlen((char*)gnss_usart.buf_out));
			break;
		case GNSS_RATE:
			sprintf((char*)gnss_usart.buf_out, "$GPSGG,RATE01*6B");
			Send_Cmd_GNSS(gnss_usart.buf_out, strlen((char*)gnss_usart.buf_out));
			break;
		case GNSS_MIX:
			sprintf((char*)gnss_usart.buf_out, "$GPSGG,NVSMIX*7F ");
			Send_Cmd_GNSS(gnss_usart.buf_out, strlen((char*)gnss_usart.buf_out));
			break;
		case GNSS_OK:
			gnss_usart.on=1;
			break;
		default:
			break;
	}
	gnss_usart.state++;

}
void USART_GNSS_Handler(void)
{
  if ((LL_USART_IsActiveFlag_TXE(USART_GNSS))&&(LL_USART_IsEnabledIT_TXE(USART_GNSS)))
  {
    if (gnss_usart.ptr_out_tx2<gnss_usart.len_tx)
    {
      LL_USART_TransmitData8(USART_GNSS, *gnss_usart.tx++);
      if (++gnss_usart.ptr_out_tx2==gnss_usart.len_tx)
      {
        /* Disable TXE interrupt */
        LL_USART_DisableIT_TXE(USART_GNSS);

        /* Enable TC interrupt */
        LL_USART_EnableIT_TC(USART_GNSS);
      }
    }    
  }
  if ((LL_USART_IsActiveFlag_TC(USART_GNSS))&&(LL_USART_IsEnabledIT_TC(USART_GNSS)))
  {
	    /* Clear TC flag */
	    LL_USART_ClearFlag_TC(USART_GNSS);
	    LL_USART_DisableIT_TC(USART_GNSS);
	    /* Enable USART1 RX Interrupt */
	    LL_USART_EnableIT_RXNE(USART_GNSS);
	    gnss_usart.ready=1;    
  }
  USART_GNSS->ICR=0xFFFFFFFF;
}
void Send_Cmd_GNSS(uint8_t *str_gnss, uint16_t lenn)
{
	while (!gnss_usart.ready) {};
	gnss_usart.ready=0;
	gnss_usart.ptr_out_tx2=0; //счетчик отправленных байтов
	gnss_usart.tx=(uint8_t*)str_gnss; //отправляемая строка
	gnss_usart.len_tx=lenn; //ее длина
  LL_USART_EnableIT_TXE(USART_GNSS);  
}
uint8_t good_koord(void) 	
{
  uint8_t out=0;
  static int cnt_first_koord=5;
  unsigned int ii;
  static uint32_t cnt_upd_cal=61;
//  static char mysps[30];
  // 

  /* хорошо это А после второй запятой в строке rmc */

  ii= like_num((char*)gnss.buf_rmc,",",2);	
  cnt_upd_cal++;

  if (gnss.buf_rmc[ii]=='A') 
  {  
		gnss.cold_start_timer=0;
    if( !cnt_first_koord) 
    {
      if(calendar.update_counter>60) 
      { 
        calendar.update_counter=0;  
//        Time_date_on(); 
        calendar.update_from_gnss=1;
      }
      cpy_bet_num((char*)gnss.fix_koord,(char*)gnss.buf_rmc, 3,7);	//переписываем координаты 
      gnss.status='A';
//      for (uint8_t cc=0; cc<strlen((char*)gnss.fix_koord); cc++)
//        if (gnss.fix_koord[cc]==',') gnss.fix_koord[cc]=';';
      if (!gnss.first_koord)
        gnss.first_koord=1;      
//      cpy_bet_num(mysps,(char*)gnss.fix_koord, 0,1); 
//      RTC_WriteBackupRegister(TIME_SHIFT_REG,(u32)atof(mysps)/12);
      out=1;  
    }
    else 
    {
      cnt_first_koord--;  
    } 
  }
  else 
  {
    cnt_first_koord=10; 
    GNSS_Data_Clear();
  }
  return out; 
}
unsigned int like_num(char *instr, char *cmp_str, char num)
{
char lenin,lencmp;
int start_ind=0;
int out=0;

lenin=strlen(instr);
lencmp=strlen(cmp_str);

while (num) {
		instr +=start_ind;	lenin -=start_ind;
				
		start_ind = my_str_cmp(instr,lenin,cmp_str, lencmp)+1;

		num--; out +=start_ind;			
          };
return out;	
}
int cpy_bet_num(char *dst_str, char *src_str, int n1,int n2)
{

int j=0;


	*dst_str=0;	n2 -=n1;
	while (n1 && *src_str !=0 ) {  if(*src_str++ == ',') n1--;  };
	if(!n1)	{
		  while (n2 && *src_str !=0 && j<30) {	j++;
							                   *dst_str++=*src_str++;
											   *dst_str=0;
							                   if(*src_str == ',') n2--;
		              };
		  if(!n2) { return j;} else {return 0;};
	}
	else {
	   return 0;
	}
}
int my_str_cmp(char *str1, uint16_t length1, char *str2, uint16_t length2)
{
uint16_t ii,jj,pp;
 
ii=0; jj=0; pp=0;

 while ((pp<length1) && (jj<length2)) {
				 	 if (*(str1+ii)== *(str2+jj))  {   jj++; ii++; 
					                                   if (!pp) pp++;	} 				        
					  else   {    str1++; ii=0; jj=0; pp++; };	   
 };	
if(jj==length2)	  {	return pp; } else{ return 0;}; 
}
