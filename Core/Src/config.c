#include "main.h"


CONFIG_STRUC config;

/*TD_ONLINE BEGIN********************************************************************************************/
uint8_t nn=0;  
char in_str[RX_BUFFER_SIZE0];
char str_put[60];
uint32_t sstp=0;
uint8_t fl_egts=0;
char put_s_dat[400];
char *ptr_out_dat;
char out_buf_485[200];
static char *s;
static uint16_t len_data=0;
static uint8_t fl_opros_net=0;
/*TD_ONLINE END**********************************************************************************************/


void Configurator_Serv(void)
{
  if (config.reset_kp)    Reset_kp();
  if (config.pars_index!=usb_serv.rx_index)
  {
    instring((unsigned char)usb_serv.buf_in[config.pars_index]);
    if (++config.pars_index>=USB_BUF_SIZE)  config.pars_index=0;
  }
	config.error_timer+=t1_ms;
	if (config.error_timer>ERROR_DEBUG_PERIOD)	Error_Debug();
}
void Restart_After_Pause(void)
{
	LL_TIM_EnableIT_UPDATE(RESET_COUNTER);
	LL_TIM_SetCounter(RESET_COUNTER, 1);
  LL_TIM_EnableCounter(RESET_COUNTER);
}
void Reset_Counter_Handler(void)
{
	RESET_COUNTER->SR=0;
	if (LL_TIM_IsEnabledCounter(RESET_COUNTER))
	{
		LL_TIM_DisableCounter(RESET_COUNTER);
		gsm.restart=1;
	}
}
void Error_Debug(void)
{
	config.error_timer=0;
	if (blackbox.error)
	{
		if (blackbox.error&BLACKBOX_NOT_AVALIABLE)
			Restart_After_Pause();
		if (config.debug_out)
		{
			if (blackbox.error&BLACKBOX_NOT_AVALIABLE){ sprintf(out_buf_485, "ERROR CODE BLACKBOX_NOT_AVALIABLE\r\n");pusk_usb((uint8_t *)out_buf_485,strlen(out_buf_485));}
			if (blackbox.error&SEND_MSG_ERROR){ sprintf(out_buf_485, "ERROR CODE SEND_MSG_ERROR\r\n");pusk_usb((uint8_t *)out_buf_485,strlen(out_buf_485));}
			if (blackbox.error&BB_CRC_ERROR){ sprintf(out_buf_485, "ERROR CODE BB_CRC_ERROR\r\n");pusk_usb((uint8_t *)out_buf_485,strlen(out_buf_485));}
			if (blackbox.error&MARKER_ERROR){ sprintf(out_buf_485, "ERROR CODE MARKER_ERROR\r\n");pusk_usb((uint8_t *)out_buf_485,strlen(out_buf_485));}
		}
		blackbox.error=0;
	}
}
void Write_kalibr_data(void)
{
  kp.crc_edata= crc_str(&kp.f, 162);
  kp.crc=crc_str(&kp.f, sizeof(kp)-1);
	write_variables((char*)&kp,sizeof(kp),KALIBR_ADDR);
}	
void out_d(void)
{    

char *ast;
                  ast = zagolovok(out_DATA);
 //                 crc1 = zagolovok(out_DATA);
									*ast++=0;//temperature;
								//	crc1= ddout(1,crc1,z);           // Выдача температуры
									ast=ddout(ast,2, 1000);//fu[0]);  //z=fu;
								//	crc1= ddout(2,crc1,z);            // выдача текущего  уровня 
									ast=ddout(ast,2, 660);//prom);  //;                         // 2 тек. периода 
									*ast++= crc_str(put_s_dat, ast-put_s_dat);
//								    pusk_usb((uint8_t *)put_s_dat,ast-put_s_dat);     
          pusk_usb((uint8_t *)put_s_dat,ast-put_s_dat);  
 } 
void out_td(void)
{ 
  uint32_t pgf; 

  char *ast;
  ast = zagolovok(out_full);
  *ast++=(int8_t)0;//temperature;
  pgf=0;//fu[0];
  ast=ddout(ast,2,pgf );	  // fu[0]	100
  ast=ddout(ast,4, 0);//prom);
  ast=ddout(ast,4, 0);//kal_str.mem_LOW);
  ast=ddout(ast,4, 0);//kal_str.mem_HI);
  ast=ddout(ast,2, 0);//kal_str.mem_KO);
  *ast++=0;//kal_str.mode;
  *ast++=10;//z=mem_porog;
  *ast++=crc_str(put_s_dat, ast-put_s_dat);
//  pusk_usb((uint8_t *)put_s_dat,ast-put_s_dat);
  pusk_usb((uint8_t *)put_s_dat,ast-put_s_dat);
}
void save_congig(char *outs) 
{
  char *kpp;
  static uint16_t mylen,ii;
  mylen =163-1; 
  kpp=&kp.f;
  outs +=3;
  for(ii=0;ii<mylen;ii++) 
    *(kpp++) = *(outs++);	   //в оперативку
  kp.dut_count=0;
  for (ii=0; ii<8; ii++)
  {
    if (kp.mode_485[ii])
    kp.dut_nums[kp.dut_count++]=kp.addr_485[ii];
  }
  Write_kalibr_data(); 
  form_pusk_zapros(net_addr_dev,PUT_CONFIG);
  point_make.timer_period=1000*kp.dop_period;
  if (point_make.timer_period<MIN_PERIOD) 
    point_make.timer_period=MIN_PERIOD;
	LL_TIM_SetAutoReload(POINT_MAKE_TIMER, point_make.timer_period/POINT_MAKE_TIMER_RESOLUTION);
}
void form_cfg_to_out(void) 
{
char *kpp;
uint16_t mylen,ii, len_out_buf;
  
  out_buf_485[0]=pref_out;
  out_buf_485[1]=net_addr_dev;
  out_buf_485[2]=GET_CONFIG;
	mylen =3; 
	kpp=&kp.f;
	for(ii=0;ii<163-1;ii++)
    out_buf_485[mylen++]=*(kpp++);	
//  *(outs++)=*(kpp++);	   //грузим строку
	ii=(uint16_t)VERSION;//ver; 	
	out_buf_485[mylen++] = ii >>8;	         // вставить версию ПО
	out_buf_485[mylen++] = ii;
	out_buf_485[mylen++] = kp.ser_num>>24; // serial_num>>24;	 // вставить серийный номер
	out_buf_485[mylen++] = kp.ser_num>>16; //serial_num>>16;
	out_buf_485[mylen++] = kp.ser_num>>8; //serial_num>>8;
	out_buf_485[mylen] =   kp.ser_num; //serial_num;	
	 	                     
	len_out_buf=mylen+1;
	out_buf_485[len_out_buf] = crc_str(out_buf_485,mylen); //скс 
  len_out_buf++;
  pusk_usb((uint8_t *)out_buf_485, len_out_buf);
}
void Init_kp(void)
{
  load_variables((char*)&kp,sizeof(kp),KALIBR_ADDR);
  if (kp.crc!=crc_str(&kp.f, sizeof(kp)-1))
    Reset_kp();
}
void Reset_kp(void)
{
  uint8_t* d=0;
  
	char api[]="m2m.beeline.ru";
	char pwdapi[]="beeline";
	char iddev[]="100000";
	char aserv[]="193.193.165.165";//"tcp.sokolmeteo.com"; ///"185.27.193.111";//
	char pwds[]="5051";
	char ports[]="20332";//"20268";//"8001";//	
	char ftp_serv[]="ftp.fmeter.ru";
	char ftp_user[]="td500beo_fuser"; 
	char ftp_pass[]="fpass11";
  char ntp1[]="2.ru.pool.ntp.org";
  char ntp2[]="ru.pool.ntp.org";
  char ntp3[]="nl.pool.ntp.org";
  char ntp4[]="uk.pool.ntp.org";
  
  config.reset_kp=0;
  d=(uint8_t*)&kp.f;
  for (uint16_t cc=0; cc<sizeof(kp); cc++)
    *d++=0;

  
  mymemcpy ((uint8_t*)kp.api, api, strlen(api)+1); 
  mymemcpy ((uint8_t*)kp.pwd_api, pwdapi, strlen(pwdapi)+1);
  mymemcpy ((uint8_t*)kp.user_api, pwdapi, strlen(pwdapi)+1);
  mymemcpy ((uint8_t*)kp.id_dev, iddev, strlen(iddev)+1);
  mymemcpy ((uint8_t*)kp.ip_serv, aserv, strlen(aserv)+1);
  mymemcpy ((uint8_t*)kp.pwd_serv, pwds, strlen(pwds)+1);
  mymemcpy ((uint8_t*)kp.port_serv, ports, strlen(ports)+1);
  mymemcpy ((uint8_t*)kp.ftp_serv, ftp_serv, strlen(ftp_serv)+1);	
  mymemcpy ((uint8_t*)kp.ftp_user, ftp_user, strlen(ftp_user)+1);	
  mymemcpy ((uint8_t*)kp.ftp_pass, ftp_pass, strlen(ftp_pass)+1);		
  
  
  mymemcpy ((uint8_t*)kp.ntp_serv1, ntp1, strlen(ntp1)+1);
  mymemcpy ((uint8_t*)kp.ntp_serv2, ntp2, strlen(ntp2)+1);
  mymemcpy ((uint8_t*)kp.ntp_serv3, ntp3, strlen(ntp3)+1);
  mymemcpy ((uint8_t*)kp.ntp_serv4, ntp4, strlen(ntp4)+1);
  
  kp.dop_period=SEND_PERIOD/1000;
  kp.dut_count=0;
  kp.mode_in1=0;
  kp.mode_in2=0;
	
  if (strlen(kp.pwd_serv)<4) 
  {
    mymemcpy ((uint8_t*)kp.pwd_serv, pwds, strlen(pwds)+1);
  } 
	kp.server_protocol=WIALON_IPS_PROTOCOL;
  Write_kalibr_data();
}
void instring(unsigned char n)
{ 
  char ii=0,j=0;
  int ttz;
  uint16_t level=0;
  uint8_t cc=0;
  switch (nn)
  { 
    case 0: 
      if ((n==pref_out) ||(n==pref_in))  
      {  
        s=in_str; 
        *s=n; 
        nn=1;  
      }
      break;
    case 1:  
      s++;  
      *s=n;   
      nn++;   
      if(*(s-1)==pref_in) 
      {
        if(!( n == 1 || n == opros_net)) 
        { 
          nn=0; 
          s=in_str;	 
        }
      }
      if(n==opros_net) 
      {
        len_data=0; 
        fl_opros_net=1; 
        nn++;
      }
      break;
    case 2: 
      s++; 
      *s=n; 
      nn++;    
      switch (n)
      {						  
        case OUT_DATA :
          if(in_str[0]==pref_out/*3e*/) 
          {
            len_data=5;
          }
          if(in_str[0]==pref_in/*3e*/) 
          {
            len_data=0;
          } 
          break;


        case out_full : len_data=0;				     break;
        case set_LO :   len_data=0;			       break;
        case set_HI :   len_data=0;			       break;
        case set_bp_on: len_data=0;			       break;
        case set_bp_off: len_data=0;			     break;
        case log_bt_on:  len_data=0;			     break;
        case set_gsm_on: len_data=0;			     break; 
        case set_gps_on: len_data=0;			     break;
        case downloadPRG:	len_data=0;			     break;
        case clr_bb:		len_data=0;			       break;
        case GET_PARAM: len_data=0;            break;
        case set_MODE:  len_data=1;			       break;
        case ZAPROS_UPR: len_data=5;				   break;
        case GET_CONFIG: len_data=5;				   break;
        case PUT_CONFIG :  len_data= 163+2;   break;
        case GET_STATUS:	len_data=0;				   break;
        case SET_SERIAL:	len_data=4;				   break;
        case SET_PWD:     len_data=4;				   break; 
        case IN_PWD:      len_data=4;				   break; 
        case GET_IMEI_IMSI:  len_data=0;			 break;
        case SET_ZERO_FI:  len_data=0;			 break; 
        case SET_TEST_MODE:  len_data=0;			 break; 
        default:  nn=0; s=in_str;			 break; 
      } 
      break;                            
    case 3:    
      if (len_data) 
      {	
        s++; 
        *s=n; 
        len_data--;	
      }
      else 
      { 
        len_data = s-in_str; 
        len_data++;	 
        s=in_str;

        if (n==crc_str(in_str, len_data)) 
        {			//3
          if(fl_opros_net) 
          { 
            *(s+2)=opros_net; 
            fl_opros_net=0;
          }
          switch (*(s+2))
          {
            case opros_net:  
              ii=1;  
              pusk_usb((uint8_t *)&ii, 1);										
              //	   str_put[0]=1; pusk_485(str_put,1); 
              break;

            case out_full: 
              out_td(); 
              break;//	for(rdf=0; rdf<10000;rdf++) {};
            case set_LO:    
							config.debug_out=1;
              break;	      
            case set_HI :   
//              set_hi(set_HI, 0 );  
              break;
            case set_MODE:  
//              set_mode(*(s+3)); 	
              break;
            case SET_ZERO_FI:  
							fota.update_program=1;
//              set_horizont();		
              break;
            case set_bp_on: 
							in_kontakt.engine_on=1;
//              if  (open_eeprom(1) ) 
//              { 
//              kal_str.porog_run_engine=adc_sr[2];  
//              write_variables (&kal_str.f,sizeof(kal_str),SL_START_ADDR);
//              }  else 
//              {
//              no_pass();
//              };
//              close_eeprom();
              break;
            case set_bp_off: 
							in_kontakt.engine_off=1;
//              if  (open_eeprom(1) ) 
//              {
//              kal_str.porog_stop_engine=adc_sr[2]; 
//              write_variables (&kal_str.f,sizeof(kal_str),SL_START_ADDR); 
//              }  
//              else 
//              {
//              no_pass();
//              };
//              close_eeprom();
              break; 	   
            case OUT_DATA : 
//              if(in_str[0]==pref_out/*3e*/) 
//                read_bufer_485_in(in_str);
              if(in_str[0]==pref_in/*3e*/) 
                out_d(); 
              break;// resive_data();

            case PUT_CONFIG: 
//              if  (open_eeprom(1) ) 
//              {
              if((uint8_t)in_str[1]==net_addr_dev) 
                save_congig(in_str);
//              // pusk_usb((uint8_t *)str_put,++ii); 
//              }  
//              else 
//              {
//              no_pass();
//              };
//              close_eeprom();
              break;
            case GET_CONFIG: 
              if((uint8_t)in_str[1]==net_addr_dev) 
                form_cfg_to_out();	
              break;
            case GET_PARAM:   
//              out_param(); 
              break;
            case ZAPROS_UPR: 
              if(in_str[1]==net_addr_dev) 
              {	
//                fl_extern_control=1; 
                form_pusk_zapros(net_addr_dev,ZAPROS_UPR);
              }  
              break;
            case GET_STATUS: ii=0; 
              str_put[ii++]=pref_out; 
              str_put[ii++]=net_addr_dev; 
              str_put[ii++]=GET_STATUS;
              str_put[ii++]=kp.mode_in1;  
              str_put[ii++]=kp.mode_in2;
              str_put[ii++]=0;//adc_sr[0]>>4; 
              str_put[ii++]=0;//adc_sr[0]>>12; //ADCCnvVal[0];
              ttz=(int8_t)in_kontakt.temperature;//temperature*360; 
              str_put[ii++]=ttz/*adc_sr[1]>>4*/;  
              str_put[ii++]=ttz>>8 /*adc_sr[1]>>12*/;//ADCCnvVal[1];
              sstp=(uint32_t)(in_kontakt.pwr*100);//((float)adc_sr[2]*6.24)/kal_str.k_bortp; 
              str_put[ii++]=sstp;
              str_put[ii++]=sstp>>8; 
              str_put[ii++]=0;//cnt[0];	
              str_put[ii++]=0;//cnt[0]>>8;
              str_put[ii++]=0;//cnt[1]; 
              str_put[ii++]=7;//cnt[1]>>8; 
              str_put[ii++]=0;//inputs;
              for (j=0;j<8;j++) 
              {	
                level=0;
                if (kp.mode_485[j])
                {
                  cc=0;
                  while ((cc<8)&&(kp.dut_nums[cc]!=kp.addr_485[j])) cc++;
                  level=dut[cc].level;
                }
                str_put[ii++]=level>>8;//fu[j]>>8;  
                str_put[ii++]=level;//fu[j];
              }
              str_put[ii++]=(uint8_t)gsm.state_num;//ch_ggssmm;
              str_put[ii++]=gnss.sats;//kol_sputn;
              if(fl_egts) 
              {  
//                r_put[ii++]= 0;//nuuumz;  
              } 
              else 
              {    
                str_put[ii++]= config.error;//send_traf;
              }
              str_put[ii++]= gnss.snr;//ns3;	 
              str_put[ii++]= 2;//possl;
              str_put[ii++]= 3;//amax;
              str_put[ii] = crc_str(str_put,ii);
              pusk_usb((uint8_t *)str_put,++ii);
              break;
            case IN_PWD:  
//              pwd_in((s+3));
              break;
            case SET_PWD: 
//              set_pwd((s+3)); 
              break;
            case SET_SERIAL: 	
//              kal_str.k_bortp = adc_sr[2]/192;	//калибруем делитель пит	            
              kp.ser_num =*(s+3);
              kp.ser_num =(kp.ser_num<<8) | *(s+4);
              kp.ser_num =(kp.ser_num<<8) | *(s+5);
              kp.ser_num =(kp.ser_num<<8) | *(s+6); 
              Write_kalibr_data();
              			
              break;				
            case set_gsm_on:	
//              fl_log_gsm=1; 
//              fl_log_gps=0;	
//              log_bt=0;
              break;
            case log_bt_on:	
//              fl_log_gsm=0;
//              fl_log_gps=0; 
//              log_bt=1;
              break;
            case set_gps_on:	
//              fl_log_gsm=0; 
//              fl_log_gps=1;
//              log_bt=0;
              break;
            case GET_IMEI_IMSI:
              ii=0; 
              str_put[ii++]=pref_out; 
              str_put[ii++]=net_addr_dev; 
              str_put[ii++]=GET_IMEI_IMSI;
              for(j=0;j<15;j++) 
              { 
                str_put[ii++]= config.imei[j];//imei_str[j]; 
              }
              // ii +=j;
              str_put[ii++]= ';';	
              j=0;
              for(j=0;j<20;j++) 
              {  
                str_put[ii++]= config.imsi[j];//imsi_str[j]; 
              }
              //	  ii +=j;

              str_put[ii] = crc_str(str_put,ii);
              pusk_usb((uint8_t *)str_put,++ii);		  	 
              break;
            case downloadPRG: /* load_prg=2;	*/  
              break;
            case clr_bb: 
//              erase_bb();
//              pusk_usb((uint8_t *)"OK\r\n",4);       
              break;
            case SET_TEST_MODE:  
//              test_mode_flag=1;			 
              break; 	

            default:   	   
              break; 
          } 
        }
        s=in_str; 
        nn=0;                   
      }
      break; 
    default:   
      s=in_str;
      nn=0;
      break;                        
  }
}
void form_pusk_zapros(char addr_zaprosa,char cmd) 
{
  out_buf_485[0]=pref_in;
  out_buf_485[1]=addr_zaprosa;	// ch_ggssmm
  out_buf_485[2]=cmd;
  out_buf_485[3] = crc_str(out_buf_485,3);	   
  pusk_usb((uint8_t*)out_buf_485, 4);//запускаем выхлоп
}
char crcc(char dat_crc, char crc8)
{ 
char j;
  j = dat_crc ^ crc8;
  crc8 = 0;
  if(j & 0x01) crc8 ^= 0x5e;
  if(j & 0x02) crc8 ^= 0xbc;
  if(j & 0x04) crc8 ^= 0x61;
  if(j & 0x08) crc8 ^= 0xc2;
  if(j & 0x10) crc8 ^= 0x9d;
  if(j & 0x20) crc8 ^= 0x23;
  if(j & 0x40) crc8 ^= 0x46;
  if(j & 0x80) crc8 ^= 0x8c;
  return crc8;
}
char crc_str(char *str6, int lens) 
{

	uint8_t crct=0;
	uint16_t ltt=0; 
	
	for(ltt=0;ltt<lens;ltt++)	{  crct= crcc(*(str6+ltt),crct); };
	
	return crct;	
}
/************передаем значение******/ 
char *ddout(char *cr,unsigned char n,unsigned long z)
{
unsigned char i,r;	
for (i=0;i<n;i++)
{ r=z;
 *cr++=r;
 z>>=8;
};
return cr;
}
/**************Передаем заголовок*************************/
char * zagolovok( char comand)
{
//unsigned char crc1=0; 
			 ptr_out_dat=put_s_dat;
			 *ptr_out_dat++=pref_out;
			 *ptr_out_dat++=1;  //mem_NetNUM
			 *ptr_out_dat++=comand;
 //           delay_ms(2);    
//            putchar(pref_out);          // Префикс передачи
//            crc1 = crcc(pref_out,crc1);  
//            putchar(mem_NetNUM);        // сетевой адрес устройства
//            crc1 = crcc(mem_NetNUM,crc1); 
//            putchar(comand);            // подтверждение команды 
//            crc1 = crcc(comand,crc1);  
	//					flag_start_recive=0; UART_time=0;
          return ptr_out_dat;  
					
}



