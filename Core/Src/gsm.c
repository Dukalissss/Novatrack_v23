#include "main.h"

GSM_USART_STRUC gsm_usart;
GSM_STRUC       gsm;
static uint8_t  string_end=0;

void GSM_Serv(void)
{
  if (!gsm.on)                       GSM_On();
  GSM_ms_timer();
  Receive_Modem_Data();
  gsm.state_machine_period+=t1_ms;
  if (gsm.state_machine_period<GSM_STATE_MACHINE_PERIOD) return;
  gsm.state_machine_period=0; 
  
  if (!gsm.net_connected)						                            Net_Connect();
	if (!gsm.rtc_en) 							                                Ntp_connect();
  if (gsm.sms_unread) 						                              Read_SMS();	
	if (gsm.send_sms)							                                Send_SMS();
	if (!gsm.tcp_connected) 					                            Tcp_connect();		
	if ((gsm.tcp_connected)&&(!gsm.log_in)) 	                    Send_login();
  if (gsm.send_length>0)						                            Send_data_to_server();
  if ((fota.update_program)&&(!fota.download_ok))               Ftp_Connect();
	if (gsm.restart)																							while(1);
}
void Receive_Modem_Data(void)
{
  gsm_usart.rx_index=(uint16_t)GSM_RX_BUF_SIZE-(uint16_t)LL_DMA_GetDataLength(DMA_GSM, DMA_GSM_RX);
  while (gsm_usart.rx_index!=gsm_usart.rd_index) 
  {
    gsm_usart.buf_in2[gsm_usart.rd_index]=gsm_usart.buf_in[gsm_usart.rd_index];
    if (++gsm_usart.rd_index>=GSM_RX_BUF_SIZE2) 
    {
      gsm_usart.rd_index=0;
    }
  }
  while (gsm_usart.pars_index!=gsm_usart.rd_index)
  {
    gsm.answer[gsm.answer_index]=gsm_usart.buf_in2[gsm_usart.pars_index];
    if ((gsm.answer[gsm.answer_index]==0x0D)||(gsm.answer[gsm.answer_index]==0x0A)) 
      string_end++;
    else
      string_end=0;
    if (gsm.answer[gsm.answer_index]=='>')  string_end+=2;
    if ((string_end>=2)&&(gsm.answer_index<GSM_RX_BUF_SIZE2-1))//конец строки
    {
      gsm.answer_index++;
      Serv_Cmd_Check();
      parse_mdm_data();
      if (gsm.sms_rec)
				sms_proc();
      gsm.answer_index=0;
      string_end=0;
      
    } else
    if (++gsm.answer_index>=GSM_RX_BUF_SIZE2) gsm.answer_index=0;
    
    if (++gsm_usart.pars_index>=GSM_RX_BUF_SIZE2) gsm_usart.pars_index=0;
  }
}
void parse_mdm_data(void)
{
	char jj=0;
  if (config.debug_out)
		pusk_usb(gsm.answer,  gsm.answer_index);
	if (gsm.state==GetCCLK) Get_Time_NTP();	
	jj= my_str_cmp((char*)gsm.answer,  gsm.answer_index, (char*)gsm.wait_ans, strlen((char*)gsm.wait_ans));
	if(gsm.not_flg) { if(jj){jj=0;} else{jj=1;}; gsm.not_flg=0;};
	if  (jj) 
	{
    Wait_Ans_Clear();
		gsm.state++; 		
		gsm.rst=0;
    gsm.rst_timeout=0;
		if ((gsm.cmd==NET_CONNECT)&&(gsm.state==Pimei+1))
			GetIMEI();
		if ((gsm.cmd==NET_CONNECT)&&(gsm.state==Pimsi+1))
      Get_IMSI();
    if ((gsm.cmd==FTP_CONNECT)&&(gsm.state==fs_local_drive+1)) 
      Get_Drive();
//    if ((gsm.cmd==FTP_CONNECT)&&(gsm.state==fs_check_file+1)) 
//      File_Exists();
    if ((gsm.cmd==FTP_CONNECT)&&(gsm.state==fs_free_size+1)) 
      Get_Available_Size();
    if ((gsm.cmd==FTP_CONNECT)&&(gsm.state==ftpsize+1)) 
      Get_File_Size();
    if ((gsm.cmd==FTP_CONNECT)&&(gsm.state==ftpget+1)) 
      CheckReceivedData();
	}
	if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "ERROR", strlen("ERROR"))) 
	{
		gsm.on=0;
	}		
	if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "CLOSED", strlen("CLOSED"))) 
	{
		gsm.on=0;
	}		
	if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "not allowed", strlen("not allowed"))) 
	{
		gsm.on=0;
	}	
	if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "DOWN", strlen("DOWN"))) 
	{
		gsm.on=0;
	}  
	if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "+CNTP: 61", strlen("+CNTP: 61"))) 
	{
    if (++gsm.ntp_num>=NTP_NUM)
    {
      gsm.state=NTP_fail; 		
      gsm.rst=0;	
    }
    else
    {
      gsm.state=0; 		
      gsm.rst=0;
    }
	}	
	if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "#ASD#1", strlen("#ASD#1"))) 
	{
		gsm.ASD=1;
		gsm.sending_timeout=NO_SENDING_DATA_TIMEOUT;
	} 
	if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "#ASD#0", strlen("#ASD#0"))) 
	{
		gsm.ASD=0;
	} 
	if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "SM", strlen("SM"))) 
	{
		gsm.sms_unread=1;
	} 
	if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "ME", strlen("ME"))) 
	{
		gsm.sms_unread=1;
	} 
}
void Wait_Ans_Clear(void)
{
  for (uint8_t cc=0; cc<WAIT_ANS_LEN; cc++)
     gsm.wait_ans[cc]=0;
}
void Net_Connect(void)
{
  if ((gsm.cmd!=NOP)&&(gsm.cmd!=NET_CONNECT)) return;
	gsm.cmd=NET_CONNECT;
  gsm.state_num=gsm.state;
  switch (gsm.state)
  {
    case reset1:
      GSM_PWRKEY_OFF();
      gsm.delay=500;//задержка в мс
      gsm.state++;
      break;
    case reset2:
      State_Machine_Delay();
      break;
    case reset3:
      GSM_PWRKEY_ON();
      gsm.delay=1000;//задержка в мс
      gsm.state++;
      break;
    case reset4:
      State_Machine_Delay();
      break;
    case Patz:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=3;
        }
        gsm.TimeOutGsm=1000;
				sprintf((char*)gsm_usart.buf_out, "ATZ\r\n");
				sprintf((char*)gsm.wait_ans, "OK");
        Send_Cmd_GSM(gsm_usart.buf_out, strlen((char*)gsm_usart.buf_out));
			}
      break;
    case Pate:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=3;
        }
        gsm.TimeOutGsm=1000;
				sprintf((char*)gsm_usart.buf_out, "ATE0\r\n");
				sprintf((char*)gsm.wait_ans, "OK");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;
		case Preg:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=100;
        }
        gsm.TimeOutGsm=1000;
				sprintf((char*)gsm_usart.buf_out, "at+creg?\r");
				sprintf((char*)gsm.wait_ans, "+CREG: 0,1");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;
		case Pcmee:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=3;
        }
        gsm.TimeOutGsm=5000;
				sprintf((char*)gsm_usart.buf_out, "at+cmee=2\r\n");
				sprintf((char*)gsm.wait_ans, "OK");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;
		case Papn:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=3;
        }
        gsm.TimeOutGsm=1000;
				sprintf((char*)gsm_usart.buf_out,"at+cstt=\"%s\",\"%s\",\"%s\"\r\n",kp.api,kp.user_api,kp.pwd_api);
				sprintf((char*)gsm.wait_ans, "OK");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}

			break;
		case PgprsUp:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=2;
        }
				gsm.TimeOutGsm=40000;
				sprintf((char*)gsm_usart.buf_out, "AT+CIICR\r\n");
				sprintf((char*)gsm.wait_ans, "OK");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out)); 
			}
			break;
		case Pip:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=2;
        }
				gsm.TimeOutGsm=1000;
				sprintf((char*)gsm_usart.buf_out, "AT+CIFSR\r\n");
				sprintf((char*)gsm.wait_ans, "0.0");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
				gsm.not_flg=1;
			}
			break;
		case Pimei:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=2;
        }
        gsm.TimeOutGsm=1000;
				sprintf((char*)gsm_usart.buf_out, "AT+CGSN\r\n");
				sprintf((char*)gsm.wait_ans, "OK");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;
		case Pimsi:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=2;
        }
        gsm.TimeOutGsm=30000;
				sprintf((char*)gsm_usart.buf_out, "AT+CIMI\r\n");
				sprintf((char*)gsm.wait_ans, "OK");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;
		case PSAPBR:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=2;
        }
        gsm.TimeOutGsm=10000;
				sprintf((char*)gsm_usart.buf_out, "AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n");
				sprintf((char*)gsm.wait_ans, "OK");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;	
		case PSAPBR2:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=2;
        }
        gsm.TimeOutGsm=10000;
				sprintf((char*)gsm_usart.buf_out, "AT+SAPBR=3,1,\"APN\",\"%s\"\r\n", kp.api);
				sprintf((char*)gsm.wait_ans, "OK");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;
		case PSAPBR3:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=2;
        }
        gsm.TimeOutGsm=10000;
				sprintf((char*)gsm_usart.buf_out, "AT+SAPBR=1,1\r\n");
				sprintf((char*)gsm.wait_ans, "OK");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;
		case PSAPBR4:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=2;
        }
        gsm.TimeOutGsm=10000;
				sprintf((char*)gsm_usart.buf_out, "AT+SAPBR=2,1\r\n");
				sprintf((char*)gsm.wait_ans, "+SAPBR: 1,1,");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;
		case NET_CONNECTED_OK:
			gsm.net_connected=1;
			gsm.state=0;
			gsm.cmd=NOP;
			gsm.rst=0;
      gsm.rst_timeout=0;
      gsm.TimeOutGsm=0;
      gsm.TimeOutCounter=0;
			break;
    default:
      gsm.state=0;
      break;
  }
}
void Ntp_connect(void)
{
	if ((gsm.cmd!=NOP)&&(gsm.cmd!=NTP_CONNECT)) return;
	gsm.cmd=NTP_CONNECT;
  gsm.state_num=gsm.state+NET_CONNECTED_OK;
	switch (gsm.state)
	{
		case CNTPCID:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=2;
        }
        gsm.TimeOutGsm=1000;
				sprintf((char*)gsm_usart.buf_out, "AT+CNTPCID=1\r\n");
				sprintf((char*)gsm.wait_ans, "OK");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;	
		case CNTP:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=1;
        }
        gsm.TimeOutGsm=10000;
        switch (gsm.ntp_num)
        {
          case 0:
            sprintf((char*)gsm_usart.buf_out, "AT+CNTP=\"%s\",0\r\n", kp.ntp_serv1);
            break;
          case 1:
            sprintf((char*)gsm_usart.buf_out, "AT+CNTP=\"%s\",0\r\n", kp.ntp_serv2);
            break;
          case 2:
            sprintf((char*)gsm_usart.buf_out, "AT+CNTP=\"%s\",0\r\n", kp.ntp_serv3);
            break;
          case 3:
            sprintf((char*)gsm_usart.buf_out, "AT+CNTP=\"%s\",0\r\n", kp.ntp_serv4);
            break;
          default:
            gsm.ntp_num=0;
            sprintf((char*)gsm_usart.buf_out, "AT+CNTP=\"%s\",0\r\n", kp.ntp_serv1);
            break;
        }
				sprintf((char*)gsm.wait_ans, "OK");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;	
		case GetCNTP:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=5;
        }
        gsm.TimeOutGsm=10000;
				sprintf((char*)gsm_usart.buf_out, "AT+CNTP\r\n");
				sprintf((char*)gsm.wait_ans, "+CNTP: 1");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;			
		case GetCCLK:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=2;
        }
        gsm.TimeOutGsm=10000;
				sprintf((char*)gsm_usart.buf_out, "AT+CCLK?\r\n");
				sprintf((char*)gsm.wait_ans, "OK");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;
		case SetCCLK:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=1;
        }
        gsm.TimeOutGsm=10000;
				sprintf((char*)gsm_usart.buf_out, "AT+CLTS=0\r\n");
				sprintf((char*)gsm.wait_ans, "OK");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;
		case Save_changes:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=1;
        }
        gsm.TimeOutGsm=10000;
				sprintf((char*)gsm_usart.buf_out, "AT&W\r\n");
				sprintf((char*)gsm.wait_ans, "OK");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;
		case NTP_connected:
			gsm.rtc_en=1;
			gsm.state=0;
			gsm.cmd=NOP;
			gsm.rst=0;
      gsm.rst_timeout=0;
      gsm.TimeOutGsm=0;
      gsm.TimeOutCounter=0;
			break;
    case NTP_fail:
			gsm.rtc_en=2;
			gsm.state=0;
			gsm.cmd=NOP;
			gsm.rst=0;
      gsm.rst_timeout=0;
      gsm.TimeOutGsm=0;
      gsm.TimeOutCounter=0;
      break;
		default:
			break;
	}
}
void Tcp_connect(void)
{
	if ((gsm.cmd!=NOP)&&(gsm.cmd!=TCP_CONNECT)) return;
	gsm.cmd=TCP_CONNECT;
  gsm.state_num=gsm.state+NET_CONNECTED_OK+NTP_connected+Read_sms_ok+Send_sms_ok;
	switch (gsm.state)
	{
		case Ptcp:
			if (!gsm.rst)
			{
				//connect to server kp.ip_serv
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=2;
        }
        gsm.TimeOutGsm=60000;
				sprintf((char*)gsm_usart.buf_out,"at+cipstart=\"TCP\",\"%s\",\"%s\"\r\n",kp.ip_serv,kp.port_serv);
				sprintf((char*)gsm.wait_ans, "CONNECT OK");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;
		case ip_addr:
			if (!gsm.rst)
			{
				//connect to server kp.ip_serv
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=1;
        }
        gsm.TimeOutGsm=1000;
				sprintf((char*)gsm_usart.buf_out,"AT+CIPSRIP=0\r\n");
				sprintf((char*)gsm.wait_ans, "OK");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;
		case Tcp_ok:
			gsm.tcp_connected=1;
			gsm.state=0;
			gsm.cmd=NOP;
			gsm.rst=0;
      gsm.rst_timeout=0;
      gsm.TimeOutGsm=0;
      gsm.TimeOutCounter=0;
			break;
		default:
			break;
	}	
}
void Send_login(void)
{
	if ((gsm.cmd!=NOP)&&(gsm.cmd!=SEND_LOGIN)) return;
	gsm.cmd=SEND_LOGIN;
  gsm.state_num=gsm.state+NET_CONNECTED_OK+NTP_connected+Read_sms_ok+Send_sms_ok+Tcp_ok;
	switch (gsm.state)
	{
		case Psend_l:
			if (!gsm.rst)
			{
				//send AT-command of TCP sending buffer with length len_of_buf
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=1;
        }
        gsm.TimeOutGsm=1000;
				switch (kp.server_protocol)
				{
					case WIALON_IPS_PROTOCOL:
						gsm.send_length=Wialon_IPS_Init_Login();
						break;
					case EGTS_PROTOCOL:
						gsm.send_length=EGTS_Init_Login();
						break;
					default:
						gsm.send_length=Wialon_IPS_Init_Login();
						break;
				}
				sprintf((char*)gsm_usart.buf_out,"AT+CIPSEND=%d\r\n", gsm.send_length);
				sprintf((char*)gsm.wait_ans, ">");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;
		case Psend_data_l:
			if (!gsm.rst)
			{
				//send buffer msg
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=1;
        }
				switch (kp.server_protocol)
				{
					case WIALON_IPS_PROTOCOL:
						sprintf((char*)gsm.wait_ans, "#AL#1");
						break;
					case SHORT_OUT:
						sprintf((char*)gsm.wait_ans, "#AL#1");
						break;
					case EGTS_PROTOCOL:
						break;
					default:
						break;
				}
        gsm.TimeOutGsm=20000;
				Send_Cmd_GSM((uint8_t*)gsm.msg , gsm.send_length);
			}
			break;
		case Data_l_send_ok:
			gsm.log_in=1;
			gsm.send_length=0;
			gsm.state=0;
			gsm.cmd=NOP;
			gsm.rst=0;
      gsm.rst_timeout=0;
      gsm.TimeOutGsm=0;
      gsm.TimeOutCounter=0;
			break;
		default:
			break;	
	}
}
void Send_data_to_server(void)
{
	if ((gsm.cmd!=NOP)&&(gsm.cmd!=SEND_DATA)) return;
	gsm.cmd=SEND_DATA;
  gsm.state_num=gsm.state+NET_CONNECTED_OK+NTP_connected+Read_sms_ok+Send_sms_ok+Tcp_ok+Data_l_send_ok;
	if ((gsm.send_length>=MSG_LENGTH)||(gsm.send_length<3))
	{
		gsm.send_length=0;
		gsm.cmd=0;
		gsm.state=0;
    for (uint16_t cc=0; cc<MSG_LENGTH; cc++)
      gsm.msg[cc]=0;
		return;
	}
	switch (gsm.state)
	{
		case Psend:
			if (!gsm.rst)
			{
				//send AT-command of TCP sending buffer with length len_of_buf
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=1;
        }
        gsm.TimeOutGsm=1000;
				sprintf((char*)gsm_usart.buf_out,"AT+CIPSEND=%d\r\n", gsm.send_length);
				sprintf((char*)gsm.wait_ans, ">");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;
		case Psend_data:
			if (!gsm.rst)
			{
				//send buffer msg
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=1;
        }
        gsm.TimeOutGsm=20000;
				sprintf((char*)gsm.wait_ans, "SEND OK");	
				Send_Cmd_GSM((uint8_t*)gsm.msg , gsm.send_length);
			}
			break;
		case Data_send_ok:
      gsm.send_length=0;
			gsm.state=0;
			gsm.cmd=NOP;
			gsm.rst=0;
      gsm.rst_timeout=0;
      gsm.TimeOutGsm=0;
      gsm.TimeOutCounter=0;
			break;
		default:
			break;	
	}
}
void Ftp_Connect(void)
{
	if ((gsm.cmd!=NOP)&&(gsm.cmd!=FTP_CONNECT)) return;
	gsm.cmd=FTP_CONNECT;
  gsm.state_num=gsm.state+NET_CONNECTED_OK+NTP_connected+Read_sms_ok+Send_sms_ok+Tcp_ok+Data_l_send_ok+Data_send_ok;
	switch (gsm.state)
	{
		case ftpcid:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=1;
        }
        gsm.TimeOutGsm=1000;
				fota.download_ok=0;
				sprintf((char*)gsm_usart.buf_out,"AT+FTPCID=1\r\n");
				sprintf((char*)gsm.wait_ans, "OK");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;
		case ftpserv:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=1;
        }
        gsm.TimeOutGsm=1000;
				sprintf((char*)gsm_usart.buf_out,"AT+FTPSERV=\"%s\"\r\n", kp.ftp_serv);
				sprintf((char*)gsm.wait_ans, "OK"); 
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;
		case ftpun:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=1;
        }
        gsm.TimeOutGsm=1000;
				sprintf((char*)gsm_usart.buf_out,"AT+FTPUN=\"%s\"\r\n", kp.ftp_user);
				sprintf((char*)gsm.wait_ans, "OK");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;
		case ftppw:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=1;
        }
        gsm.TimeOutGsm=1000;
				sprintf((char*)gsm_usart.buf_out,"AT+FTPPW=\"%s\"\r\n", kp.ftp_pass);
				sprintf((char*)gsm.wait_ans, "OK");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;
		case ftpgetname:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=1;
        }
        gsm.TimeOutGsm=1000;
				sprintf((char*)gsm_usart.buf_out,"AT+FTPGETNAME=\"app_tracker\"\r\n");
				sprintf((char*)gsm.wait_ans, "OK");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;
		case ftpgetpath:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=1;
        }
        gsm.TimeOutGsm=1000;
				sprintf((char*)gsm_usart.buf_out,"AT+FTPGETPATH=\"/Novatrack/v1/\"\r\n");
				sprintf((char*)gsm.wait_ans, "OK");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;
    case fs_local_drive:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=1;
        }
        gsm.TimeOutGsm=1000;
				sprintf((char*)gsm_usart.buf_out,"AT+FSDRIVE=0\r\n");
				sprintf((char*)gsm.wait_ans, "+FSDRIVE");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;  
//		case fs_check_file:
//			if (!gsm.rst)
//			{
//				gsm.rst=1;
//        if (!gsm.rst_timeout)
//        {
//          gsm.rst_timeout=1;
//          gsm.TimeOutCounter=1;
//        }
//        gsm.TimeOutGsm=1000;
//				sprintf((char*)gsm_usart.buf_out,"AT+FSLS=%s:\\user\\ftp\\\r\n", fota.drive);
//				sprintf((char*)gsm.wait_ans, "OK");
//				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
//			}
//			break;
//		case fs_del_file:
//			if (!gsm.rst)
//			{
//				gsm.rst=1;
//        if (!gsm.rst_timeout)
//        {
//          gsm.rst_timeout=1;
//          gsm.TimeOutCounter=1;
//        }
//        gsm.TimeOutGsm=1000;
//				sprintf((char*)gsm_usart.buf_out,"AT+FSDEL=%s:\\user\\ftp\\app\r\n", fota.drive);
//				sprintf((char*)gsm.wait_ans, "OK");
//				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
//			}
//			break;
    case fs_free_size:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=1;
        }
        gsm.TimeOutGsm=1000;
				sprintf((char*)gsm_usart.buf_out,"AT+FSMEM\r\n");
				sprintf((char*)gsm.wait_ans, "+FSMEM");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;  
		case ftpsize:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=5;
        }
        gsm.TimeOutGsm=40000;
				fota.download_ok=0;
				sprintf((char*)gsm_usart.buf_out,"AT+FTPSIZE\r\n");
				sprintf((char*)gsm.wait_ans, "+FTPSIZE");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;
		case ftpget:
			if (!gsm.rst)
			{
				if (fota.program_size>fota.available_size)
				{
					gsm.state=ftpquit;
					fota.program_size=0;
					fota.not_enough_size=1;
					return;
				}
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=3;
        }
        gsm.TimeOutGsm=60000;
				sprintf((char*)gsm_usart.buf_out,"AT+FTPGETTOFS=0,app\r\n");//AT+FTPGET=1
				sprintf((char*)gsm.wait_ans, "+FTPGETTOFS: 0,");//+FTPGET: 1,1
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;
		case ftpquit:
			if (fota.program_size>0)
			{
				gsm.state=ftpsize;
				return;
			} 
			else
			{
				fota.download_ok=1;
			}
			break;	
		default:
			break;
	}
}
void Read_SMS(void)
{
	if ((gsm.cmd!=NOP)&&(gsm.cmd!=READ_SMS)) return;
	gsm.cmd=READ_SMS;
  gsm.state_num=gsm.state+NET_CONNECTED_OK+NTP_connected;
	switch(gsm.state)
	{
		case Pcmgf:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=1;
        }
        gsm.TimeOutGsm=1000;
				sprintf((char*)gsm_usart.buf_out,"AT+CMGF=1\r\n");
				sprintf((char*)gsm.wait_ans, "OK");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;			
		case Pcmgl:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=1;
        }
        gsm.TimeOutGsm=1000;
				gsm.sms_rec=1;
				sprintf((char*)gsm_usart.buf_out,"AT+CMGL=\"REC UNREAD\"\r\n");
				sprintf((char*)gsm.wait_ans, "OK");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;		
		case Pdelsms:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=1;
        }
        gsm.TimeOutGsm=1000;
        gsm.sms_rec=0;
				sprintf((char*)gsm_usart.buf_out,"AT+CMGD=1,2\r\n");
				sprintf((char*)gsm.wait_ans, "OK");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;	
		case Read_sms_ok:
			gsm.sms_unread=0;
			gsm.state=0;
			gsm.cmd=NOP;
			break;
		default:
			break;
	}
}
void Send_SMS(void)
{
	if ((gsm.cmd!=NOP)&&(gsm.cmd!=SEND_SMS)) return;
	gsm.cmd=SEND_SMS;
  gsm.state_num=gsm.state+NET_CONNECTED_OK+NTP_connected+Read_sms_ok;
	switch(gsm.state)
	{
		case Pcmgs:
			if (!gsm.rst)
			{
				if (strlen(gsm.phone_number)<4) 
				{
					gsm.state=0;
					gsm.send_sms=0;
					return;
				}
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=1;
        }
        gsm.TimeOutGsm=1000;
				sprintf((char*)gsm_usart.buf_out,"AT+CMGS=%s\r\n", gsm.phone_number);
				sprintf((char*)gsm.wait_ans, ">");
				Send_Cmd_GSM(gsm_usart.buf_out , strlen((char*)gsm_usart.buf_out));
			}
			break;	
		case Psms:
			if (!gsm.rst)
			{
				gsm.rst=1;
        if (!gsm.rst_timeout)
        {
          gsm.rst_timeout=1;
          gsm.TimeOutCounter=1;
        }
        gsm.TimeOutGsm=40000;
				sprintf(gsm.phone_number, "");
				sprintf((char*)gsm.wait_ans, "OK");
				Send_Cmd_GSM((uint8_t*)gsm.sms_ans , strlen((char*)gsm.sms_ans));
			}
			break;	
		case Send_sms_ok:
			gsm.state=0;
			gsm.send_sms=0;	
			gsm.cmd=NOP;	
			break;		
		default:
			break;
	}	
}
void Serv_Cmd_Check(void)
{
	char jj=0, k=0;
	jj= my_str_cmp((char*)gsm.answer,  gsm.answer_index, "SRVCMD", strlen("SRVCMD"));
  if (!jj) return;  
  jj=0;
  while ((gsm.answer[jj]!=';')&&(jj<gsm.answer_index)) jj++;
	jj= my_str_cmp((char*)gsm.answer,  jj, kp.pwd_serv, strlen(kp.pwd_serv));
  if (!jj) return; 
  jj= my_str_cmp((char*)gsm.answer,  gsm.answer_index, "greet", strlen("greet"));
  if (jj)
  {
    jj=0;
    while ((gsm.answer[jj]!=':')&&(jj<gsm.answer_index)) jj++;
    gsm.phone_number[k++]='"';
    jj++;
    while ((gsm.answer[jj]!=0x0A)&&(gsm.answer[jj]!=0x0D)&&(jj<gsm.answer_index))
      gsm.phone_number[k++]=gsm.answer[jj++];
    gsm.phone_number[k++]='"';
    gsm.phone_number[k++]=0;
    sprintf(gsm.sms_ans, "call me, baby, I'm %s", kp.id_dev);
		uint8_t i=0;
		while (gsm.sms_ans[i]) i++;
		gsm.sms_ans[i]=0x1A;
		gsm.send_sms=1;
  }
  jj= my_str_cmp((char*)gsm.answer,  gsm.answer_index, "UPDATE_APP", strlen("UPDATE_APP"));
  if (jj) 
    fota.update_program=1;
  jj= my_str_cmp((char*)gsm.answer,  gsm.answer_index, "BB_ERASE", strlen("BB_ERASE"));
  if (jj) 
  {
		EraseMemory();		
		blackbox.read_lines=0;
		blackbox.writed_lines=0;	
	}
  jj= my_str_cmp((char*)gsm.answer,  gsm.answer_index, "SET_ENG_ON", strlen("SET_ENG_ON"));
  if (jj) 
    in_kontakt.engine_on=1;
  jj= my_str_cmp((char*)gsm.answer,  gsm.answer_index, "SET_ENG_OFF", strlen("SET_ENG_OFF"));
  if (jj) 
    in_kontakt.engine_off=1;
	
	
  jj= my_str_cmp((char*)gsm.answer,  gsm.answer_index, "SET_EGTS", strlen("SET_EGTS"));
  if (jj) 
	{
    kp.server_protocol=EGTS_PROTOCOL;
		Write_kalibr_data();
	}
	jj= my_str_cmp((char*)gsm.answer,  gsm.answer_index, "SET_WIALON_IPS", strlen("SET_WIALON_IPS"));
  if (jj) 
	{
    kp.server_protocol=WIALON_IPS_PROTOCOL;
		Write_kalibr_data();
	}
	
  jj= my_str_cmp((char*)gsm.answer,  gsm.answer_index, "RESTART", strlen("RESTART"));
  if (jj) 
		while(1) {};
}
void CheckReceivedData(void)
{
	uint8_t ibah=0, dnk=0;
	char size[5];
	int32_t ss=0;
	ibah=0;
	while ((gsm.answer[ibah]!=',')&&(ibah<gsm.answer_index)) ibah++;
	if (++ibah>=gsm.answer_index) return;
	while (ibah<gsm.answer_index-1)
		size[dnk++]=gsm.answer[ibah++];
	size[dnk]=0x00;
	ss=(int32_t)atol(size);
	fota.program_size-=ss;
//	eat_trace("Size of received data: %d, ostalos: %d\r\n", ss, ftp.program_size);
}
void Get_File_Size(void)
{
  uint16_t  k=0, i=0;
  char s[20]="";
  while ((gsm.answer[k]!=':')&&(k<gsm.answer_index)) k++;
  k+=6;
  while ((gsm.answer[k]!='\r')&&(gsm.answer[k]!='\n')&&(k<gsm.answer_index)) 
  {
    s[i]=gsm.answer[k];
    i++;
    k++;
  }
  s[i]=0;
  fota.program_size=(int32_t)atol(s);
}
void File_Exists(void)
{
  char jj=0;
  jj= my_str_cmp((char*)gsm_usart.buf_in2,  GSM_RX_BUF_SIZE2, "app", strlen("app"));
  if (!jj) 
    gsm.state=fs_free_size;
//  else
//    gsm.state=fs_del_file;
}
void Get_Drive(void)
{
  uint16_t  k=0;
  while ((gsm.answer[k]!=':')&&(k<gsm.answer_index)) k++;
  k+=2;
  fota.drive[0]=gsm.answer[k];
  fota.drive[1]=0;
}
void Get_Available_Size(void)
{
  uint16_t  k=0, i=0;
  char s[20]="";
  while ((gsm.answer[k]!=fota.drive[0])&&(k<gsm.answer_index)) k++;
  k+=2;
  while ((gsm.answer[k]!='b')&&(k<gsm.answer_index)) 
  {
    s[i]=gsm.answer[k];
    i++;
    k++;
  }
  s[i]=0;
  fota.available_size=(uint32_t)atol(s);
}
void Get_IMSI(void)
{
	uint16_t ibah=0, dnk=0;
  if (gsm_usart.pars_index>=4)
    ibah=gsm_usart.pars_index-4;
  else
    ibah=GSM_RX_BUF_SIZE2+gsm_usart.pars_index-4;
    
  dnk=gsm_usart.buf_in2[ibah];
	while ((gsm_usart.buf_in2[ibah]==' ')||(gsm_usart.buf_in2[ibah]=='\r')||(gsm_usart.buf_in2[ibah]=='\n')) 
  {
    if ((ibah>0)&&(ibah<GSM_RX_BUF_SIZE2))
    {
      ibah--;
      
    } else 
      ibah=GSM_RX_BUF_SIZE2-1;
  }
  if (ibah>=14)
    ibah-=14;
  else
    ibah=GSM_RX_BUF_SIZE2+ibah-14;
  
	
	for (dnk=0; dnk<15; dnk++) 
  {
    gsm.imsi[dnk]=gsm_usart.buf_in2[ibah];
    if (++ibah>=GSM_RX_BUF_SIZE2) ibah=0;
  }
	gsm.imsi[15]=0;
	for (dnk=0; dnk<=15; dnk++) 
    config.imsi[dnk]=gsm.imsi[dnk];
}
void GetIMEI(void)
{
	uint16_t ibah=0, dnk=0;
  if (gsm_usart.pars_index>=4)
    ibah=gsm_usart.pars_index-4;
  else
    ibah=GSM_RX_BUF_SIZE2+gsm_usart.pars_index-4;
    
  dnk=gsm_usart.buf_in2[ibah];
	while ((gsm_usart.buf_in2[ibah]==' ')||(gsm_usart.buf_in2[ibah]=='\r')||(gsm_usart.buf_in2[ibah]=='\n')) 
  {
    if ((ibah>0)&&(ibah<GSM_RX_BUF_SIZE2))
    {
      ibah--;
      
    } else 
      ibah=GSM_RX_BUF_SIZE2-1;
  }
  if (ibah>=14)
    ibah-=14;
  else
    ibah=GSM_RX_BUF_SIZE2+ibah-14;
  
	
	for (dnk=0; dnk<15; dnk++) 
  {
    gsm.imei[dnk]=gsm_usart.buf_in2[ibah];
    if (++ibah>=GSM_RX_BUF_SIZE2) ibah=0;
  }
	gsm.imei[15]=0;
	for (dnk=0; dnk<=15; dnk++) 
    config.imei[dnk]=gsm.imei[dnk];
 
}
void Get_Time_NTP(void)
{
	uint8_t i=0;
	char buf[3];
	char jj=0;
	
	jj= my_str_cmp((char*)gsm.answer,  gsm.answer_index, "+CCLK", strlen("+CCLK"));
	if (!jj) return;
	
	while ((i<20)&&(gsm.answer[i]!='"')) i++;
	buf[0]=gsm.answer[++i];
	buf[1]=gsm.answer[++i];
	buf[2]=0;
	calendar.year=atoi(buf);
	i++;
	buf[0]=gsm.answer[++i];
	buf[1]=gsm.answer[++i];
	calendar.month=atoi(buf);
	i++;
	buf[0]=gsm.answer[++i];
	buf[1]=gsm.answer[++i];
	calendar.day=atoi(buf);
	i++;
	buf[0]=gsm.answer[++i];
	buf[1]=gsm.answer[++i];
	calendar.hour=atoi(buf);
	i++;
	buf[0]=gsm.answer[++i];
	buf[1]=gsm.answer[++i];
	calendar.min=atoi(buf);
	i++;
	buf[0]=gsm.answer[++i];
	buf[1]=gsm.answer[++i];
	calendar.sec=atoi(buf);
	if (calendar.year<19) return;
  calendar.update_from_ntp=1;
}
void GSM_ms_timer(void)
{
	if (gsm.TimeOutGsm>t1_ms)
	{
		gsm.TimeOutGsm-=t1_ms;
		if (gsm.TimeOutGsm<t1_ms)
		{
			gsm.rst=0;
			if (gsm.TimeOutCounter>0) 
			{
				if (--gsm.TimeOutCounter==0)
        {
          gsm.on=0;
        }
			}
		}
	} else gsm.rst=0;
  if ((gsm.asd_timeout)&&(gsm.ASD==2))
  {
    if (--gsm.asd_timeout==0)
      gsm.ASD=0;
  }
	if (gsm.sending_timeout>t1_ms)
	{
		gsm.sending_timeout-=t1_ms;
		if (gsm.sending_timeout<t1_ms)
			while (1) {};//restart
	}
}
void State_Machine_Delay(void)
{
  if (gsm.delay>GSM_STATE_MACHINE_PERIOD) 
    gsm.delay-=GSM_STATE_MACHINE_PERIOD;
  else
    gsm.state++;
}
void GSM_On(void)
{
  uint8_t* g_struc=(uint8_t*)&gsm.on;
  uint16_t  g_len=(uint16_t)sizeof(gsm);
  for (uint16_t cc=0; cc<g_len-1; cc++)
    *g_struc++=0;
  if (!gsm.reboot)
  {
    gsm.cmd=NET_CONNECT;
    gsm.state=Preg;
    gsm.reboot=1;
    gsm.sms_unread=1;
    GSM_PWRKEY_ON();
  } else
  {
    GSM_PWRKEY_OFF();  
  }
	gsm.sending_timeout=NO_SENDING_DATA_TIMEOUT;
  gsm_usart.ready=1;
  LL_DMA_DisableChannel(DMA_GSM, DMA_GSM_RX);
  DMA_GSM_RX_CLEAR_FLAG();
  DMA_GSM_RX_ERROR_CLEAR(); 
  LL_USART_EnableDMAReq_RX(USART_GSM);  
  DMA_GSM_RX_CLEAR_FLAG();
  DMA_GSM_RX_ERROR_CLEAR();
  DMA_GSM_GI_FLAG_CLEAR();
  LL_DMA_ConfigAddresses(DMA_GSM, DMA_GSM_RX, LL_USART_DMA_GetRegAddr(USART_GSM, LL_USART_DMA_REG_DATA_RECEIVE), (uint32_t)&gsm_usart.buf_in[0], LL_DMA_GetDataTransferDirection(DMA_GSM, DMA_GSM_RX));
  LL_DMA_DisableChannel(DMA_GSM, DMA_GSM_RX);
  LL_DMA_SetDataLength(DMA_GSM, DMA_GSM_RX, GSM_RX_BUF_SIZE);
  LL_DMA_EnableChannel(DMA_GSM, DMA_GSM_RX);
  gsm.on=1;
}
void sms_proc(void)
{
	char jj=0;
	char bbb[10];
	uint8_t	i=0;
//	ist=(char*)gsm.answer;
	jj= my_str_cmp((char*)gsm.answer,  gsm.answer_index, "+CMGL", strlen("+CMGL"));
	if (jj) 
	{
		cpy_bet_num(gsm.phone_number, (char*)gsm.answer, 2, 3);
		return;
	}
	if (gsm.answer_index<3) return;
	while (((char)gsm.answer[i]!=';')&&(i<50)) i++;
	if (i>=50) return;
	if ((my_str_cmp((char*)gsm.answer, i+1,kp.pwd_serv,strlen(kp.pwd_serv))) //parol' sovpal
		||(my_str_cmp((char*)gsm.answer, i+1,"perfeo",strlen("perfeo"))))
	{
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "PLACE", strlen("PLACE")))
		{
			sprintf(gsm.sms_ans, "PLACE=%s;%s;%s;%s;%d;", gnss.fix_koord, gnss.speed, gnss.course, gnss.altitude, gnss.sats);
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "STATUS", strlen("STATUS")))
		{
			sprintf(gsm.sms_ans, "STATUS=%d,%d,%s,%.0f,%.1f",accel.move,gnss.sats,gnss.hdop, in_kontakt.temperature, in_kontakt.pwr);
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "IMEI", strlen("IMEI")))
		{
			sprintf(gsm.sms_ans, "IMEI=%s", gsm.imei);
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "BB_ERASE", strlen("BB_ERASE")))
		{
			EraseMemory();		
			blackbox.read_lines=0;
			blackbox.writed_lines=0;	
			sprintf(gsm.sms_ans, "Memory erased");			
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "SET_ENG_ON", strlen("SET_ENG_ON")))
		{
			in_kontakt.engine_on=1;
			sprintf(gsm.sms_ans, "OK ENGINE ON");
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "PERIOD", strlen("PERIOD")))
		{
			set_kal_param((char*)gsm.answer,bbb,gsm.answer_index);
			kp.dop_period=(uint32_t)strtol(bbb, 0, 10);
			if (1000*kp.dop_period>=MIN_PERIOD)
			{
				point_make.timer_period=1000*kp.dop_period; 	
				LL_TIM_SetAutoReload(POINT_MAKE_TIMER, point_make.timer_period/POINT_MAKE_TIMER_RESOLUTION);
				Write_kalibr_data();
			}			
			sprintf(gsm.sms_ans, "PERIOD=%d", kp.dop_period);
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "SET_ENG_OFF", strlen("SET_ENG_OFF")))
		{
			in_kontakt.engine_off=1;
			sprintf(gsm.sms_ans, "OK ENGINE OFF");
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "APN=", strlen("APN=")))
		{
			set_kal_param((char*)gsm.answer,kp.api,gsm.answer_index);
			sprintf(gsm.sms_ans, "NEW APN %s", kp.api);
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "APNUSER=", strlen("APNUSER=")))
		{
			set_kal_param((char*)gsm.answer,kp.user_api,gsm.answer_index);
			sprintf(gsm.sms_ans, "NEW APNUSER %s", kp.user_api);
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "APNPWD=", strlen("APNPWD=")))
		{
			set_kal_param((char*)gsm.answer,kp.pwd_api,gsm.answer_index);
			sprintf(gsm.sms_ans, "NEW APNPWD %s", kp.pwd_api);
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "IP=", strlen("IP=")))
		{
			set_kal_param((char*)gsm.answer,kp.ip_serv,gsm.answer_index);
			sprintf(gsm.sms_ans, "NEW IP %s", kp.ip_serv);
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "SERVPWD=", strlen("SERVPWD=")))
		{
			set_kal_param((char*)gsm.answer,kp.pwd_serv,gsm.answer_index);
			sprintf(gsm.sms_ans, "NEW SERVPWD %s", kp.pwd_serv);
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "SERIAL_NUMBER=", strlen("SERIAL_NUMBER=")))
		{
			set_kal_param((char*)gsm.answer,kp.id_dev,gsm.answer_index);
			sprintf(gsm.sms_ans, "NEW SERVID %s", kp.id_dev);
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "IPPORT=", strlen("IPPORT=")))
		{
			set_kal_param((char*)gsm.answer,kp.port_serv,gsm.answer_index);
			sprintf(gsm.sms_ans, "NEW IPPORT %s", kp.port_serv);
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "RESTART", strlen("RESTART")))
		{
			gsm.restart=1;
			sprintf(gsm.sms_ans, "I'M RESTARTING RIGHT NOW");
		}else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "VERSION", strlen("VERSION")))
		{
			sprintf(gsm.sms_ans, "VERSION=%d", (uint16_t)VERSION);
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "UPDATE_APP", strlen("UPDATE_APP")))
		{
			fota.update_program=1;
			sprintf(gsm.sms_ans, "UPDATING");
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "APP_NAME", strlen("APP_NAME")))
		{
			sprintf(gsm.sms_ans, "%s",kp.app_name);
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "SHORT_OUT_ON", strlen("SHORT_OUT_ON")))
		{
			sprintf(gsm.sms_ans, "OK SHORT OUT");
			kp.server_protocol=SHORT_OUT;
			Write_kalibr_data();
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "SHORT_OUT_OFF", strlen("SHORT_OUT_OFF")))
		{
			sprintf(gsm.sms_ans, "OK NO SHORT OUT");
			kp.server_protocol=WIALON_IPS_PROTOCOL;
			Write_kalibr_data();
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "TEMP", strlen("TEMP")))
		{
			sprintf(gsm.sms_ans, "temp=%.1f, pwr=%.1f", in_kontakt.temperature, in_kontakt.pwr);
			Write_kalibr_data();
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "EGTS", strlen("EGTS")))
		{
			sprintf(gsm.sms_ans, "OK EGTS");
			kp.server_protocol=EGTS_PROTOCOL;
			Write_kalibr_data();
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "WIALON_IPS", strlen("WIALON_IPS")))
		{
			sprintf(gsm.sms_ans, "OK WIALON_IPS");
			kp.server_protocol=WIALON_IPS_PROTOCOL;
			Write_kalibr_data();
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "ANALOG1", strlen("ANALOG1")))
		{
			sprintf(gsm.sms_ans, "OK ANALOG1");
			kp.mode_in1=ANALOGUE_IN_MODE;
			Write_kalibr_data();
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "ANALOG2", strlen("ANALOG2")))
		{
			sprintf(gsm.sms_ans, "OK ANALOG2");
			kp.mode_in2=ANALOGUE_IN_MODE;
			Write_kalibr_data();
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "DIGITAL_IN1", strlen("DIGITAL_IN1")))
		{
			sprintf(gsm.sms_ans, "OK DIGITAL_IN1");
			kp.mode_in1=DIGITAL_IN_MODE;
			Write_kalibr_data();
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "DIGITAL_IN2", strlen("DIGITAL_IN2")))
		{
			sprintf(gsm.sms_ans, "OK DIGITAL_IN2");
			kp.mode_in2=DIGITAL_IN_MODE;
			Write_kalibr_data();
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "FREQ_IN1", strlen("FREQ_IN1")))
		{
			sprintf(gsm.sms_ans, "OK FREQ_IN1");
			kp.mode_in1=FREQ_IN_MODE;
			Write_kalibr_data();
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "FREQ_IN2", strlen("FREQ_IN2")))
		{
			sprintf(gsm.sms_ans, "OK FREQ_IN2");
			kp.mode_in2=FREQ_IN_MODE;
			Write_kalibr_data();
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "IN1_OFF", strlen("IN1_OFF")))
		{
			sprintf(gsm.sms_ans, "OK IN1_OFF");
			kp.mode_in1=INACTIVE_MODE;
			Write_kalibr_data();
		} else
		if (my_str_cmp((char*)gsm.answer,  gsm.answer_index, "IN2_OFF", strlen("IN2_OFF")))
		{
			sprintf(gsm.sms_ans, "OK IN2_OFF");
			kp.mode_in2=INACTIVE_MODE;
			Write_kalibr_data();
		}
		else sprintf(gsm.sms_ans, "UNKNOWN COMMAND");
	}
	else
		sprintf(gsm.sms_ans, "BAD PASSWORD");
	i=0;
	while (gsm.sms_ans[i]) i++;
	gsm.sms_ans[i]=0x1A;
	gsm.send_sms=1;
  gsm.sms_rec=0;
}
void set_kal_param(char *src, char *dst, uint16_t len)
{
	uint8_t i=0, j=0;
	char s[20];
	mymemcpy((uint8_t*)s, dst, strlen(dst));
	while ((src[i]!='"')&&(i<len)) i++;
	if (i>=len) return;
	i++;
	while ((src[i]!='"')&&(i<len)) 
	{
		dst[j]=src[i];
		i++;
		j++;
	}
	dst[j]=0;
	if (i>=len)
		mymemcpy((uint8_t*)dst, s, strlen(s));
		
	Write_kalibr_data();
	
}
void GSM_UART_Handler(void)
{
  if ((LL_USART_IsActiveFlag_TXE(USART_GSM))&&(LL_USART_IsEnabledIT_TXE(USART_GSM)))
  {
    if (gsm_usart.ptr_out_tx2<gsm_usart.len_tx)
    {
      LL_USART_TransmitData8(USART_GSM, *gsm_usart.tx++);
      if (++gsm_usart.ptr_out_tx2==gsm_usart.len_tx)
      {
        /* Disable TXE interrupt */
        LL_USART_DisableIT_TXE(USART_GSM);

        /* Enable TC interrupt */
        LL_USART_EnableIT_TC(USART_GSM);
      }
    }    
  }
  if ((LL_USART_IsActiveFlag_TC(USART_GSM))&&(LL_USART_IsEnabledIT_TC(USART_GSM)))
  {
	    /* Clear TC flag */
	    LL_USART_ClearFlag_TC(USART_GSM);
	    LL_USART_DisableIT_TC(USART_GSM);
	    gsm_usart.ready=1;    
  }
  USART_GSM->ICR=0xFFFFFFFF;
}
void Send_Cmd_GSM(uint8_t *str_gsm, uint16_t lenn)
{
	while (!gsm_usart.ready) {};
	if (config.debug_out)
		pusk_usb(str_gsm,lenn);
	gsm_usart.ready=0;
	gsm_usart.ptr_out_tx2=0; //счетчик отправленных байтов
	gsm_usart.tx=(uint8_t*)str_gsm; //отправляемая строка
	gsm_usart.len_tx=lenn; //ее длина
  LL_USART_EnableIT_TXE(USART_GSM);  
}


