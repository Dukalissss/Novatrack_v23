/*
Коллектор  141.105.70.183 - 7650
rnd.standbox.ru
Логин: teh_auto
Пароль: teh_auto999
*/
#include "main.h"

EGTS_PARSE_STRUCT 									egts_parse={0};
EGTS_SR_POS_DATA_STRUCT 						EGTS_SR_POS_DATA={0};
EGTS_SR_TERM_IDENTITY_STRUCT 				EGTS_SR_TERM_IDENTITY={0};
SUBRECORD_STRUCT 										SUBRECORD={0};
RECORD_STRUCT												RECORD={0};
PACKET_STRUCT												EGTS_PACKET={0};
EGTS_SR_LIQUID_LEVEL_SENSOR_STRUCT 	EGTS_SR_LIQUID_LEVEL_SENSOR={0};
EGTS_SR_EXT_POS_DATA_STRUCT 				EGTS_SR_EXT_POS_DATA={0};
EGTS_SR_ABS_AN_SENS_DATA_STRUCT 		EGTS_SR_ABS_AN_SENS_DATA={0};
EGTS_SR_STATE_DATA_STRUCT 					EGTS_SR_STATE_DATA={0};
EGTS_SR_AD_SENSORS_DATA_STRUCT 			EGTS_SR_AD_SENSORS_DATA={0};

const char day_in_mon[]={31,28,31,30,31,30,31,31,30,31,30,31,100};
uint8_t*	rd_uk=0;
uint8_t*	srd_uk=0;
uint32_t tmon,tyear,days,Tsek=0;
uint32_t an_voltage=0;
uint16_t		struc_len=0;


void EGTS_Serv(void)
{
	if (kp.server_protocol!=EGTS_PROTOCOL) return;
	
	if (gsm.pusk)
	{
		gsm.pusk=0;
		EGTS_Init_Data_Packet();
	}
	
	
	if ((egts_parse.parse_timeout>t1_ms)&&(egts_parse.rx_cnt))
	{
		egts_parse.parse_timeout-=t1_ms;
		if (egts_parse.parse_timeout<t1_ms)
		{
			egts_parse.rx_cnt=0;
			egts_parse.sfr_cnt=0;
			egts_parse.sdr_cnt=0;
			egts_parse.rd_cnt=0;
			egts_parse.parse_timeout=0;
		}
	}
	if (!egts_parse.parsing_ongoing)
	{
		egts_parse.read_index=gsm_usart.rd_index;
		if (egts_parse.read_index<egts_parse.parse_index) egts_parse.read_index+=GSM_RX_BUF_SIZE2;
		if (egts_parse.read_index-egts_parse.parse_index>10)
		{
			egts_parse.check_cnt=egts_parse.parse_index;
			for (uint8_t mrn=0; mrn<30; mrn++)
			{
				egts_parse.check_buf[mrn]=gsm_usart.buf_in2[egts_parse.check_cnt];
				if (++egts_parse.check_cnt>=GSM_RX_BUF_SIZE2) egts_parse.check_cnt=0;
			}
			if (Check_Receive_Packet(egts_parse.check_buf))
					egts_parse.parsing_ongoing=1;
			else
				if (++egts_parse.parse_index>=GSM_RX_BUF_SIZE2) egts_parse.parse_index=0;
		}
	}
	while ((egts_parse.parsing_ongoing)&&(gsm_usart.rd_index!=egts_parse.parse_index))
	{
		Parse_EGTS_Data(gsm_usart.buf_in2[egts_parse.parse_index]);
		if (++egts_parse.parse_index>=GSM_RX_BUF_SIZE2) egts_parse.parse_index=0;
	}	
}
void EGTS_Init_Data_Packet(void)
{
	uint16_t pack_len=0;
	
	Record_Prepare(EGTS_TELEDATA_SERVICE);
	Packet_Prepare();
	pack_len=sizeof(EGTS_PACKET)-SFRD_MAX_LENGTH+EGTS_PACKET.FDL;
	rd_uk=(uint8_t*)&EGTS_PACKET;
	for (uint16_t ccc=0; ccc<pack_len; ccc++)
		gsm.msg[ccc]=rd_uk[ccc];
	gsm.send_length=pack_len;
}
void EGTS_SR_AD_SENSORS_DATA_Subrecord_Prepare(void)
{
	EGTS_SR_AD_SENSORS_DATA.DIO_FL=0;
	EGTS_SR_AD_SENSORS_DATA.DIO_FL|=DIOE1_FLAG;
	EGTS_SR_AD_SENSORS_DATA.DOUT=0;
	EGTS_SR_AD_SENSORS_DATA.AS_FL=0;
	EGTS_SR_AD_SENSORS_DATA.AS_FL|=ASFE1_FLAG;
	EGTS_SR_AD_SENSORS_DATA.AS_FL|=ASFE2_FLAG;
	EGTS_SR_AD_SENSORS_DATA.ADIO1=0;
	if ((kp.mode_in1==DIGITAL_IN_MODE)&&(in_kontakt.sk1))	EGTS_SR_AD_SENSORS_DATA.ADIO1|=0x01;
	if ((kp.mode_in2==DIGITAL_IN_MODE)&&(in_kontakt.sk2))	EGTS_SR_AD_SENSORS_DATA.ADIO1|=0x02;
		
	if (kp.mode_in1==ANALOGUE_IN_MODE)
		an_voltage=(uint32_t)(in_kontakt.ain1*1000);
	else
		an_voltage=0;
	EGTS_SR_AD_SENSORS_DATA.ANS1[0]=(uint8_t)an_voltage;
	EGTS_SR_AD_SENSORS_DATA.ANS1[1]=(uint8_t)(an_voltage>>8);
	EGTS_SR_AD_SENSORS_DATA.ANS1[2]=(uint8_t)(an_voltage>>16);
	if (kp.mode_in2==ANALOGUE_IN_MODE)
		an_voltage=(uint32_t)(in_kontakt.ain2*1000);
	else
		an_voltage=0;
	EGTS_SR_AD_SENSORS_DATA.ANS2[0]=(uint8_t)an_voltage;
	EGTS_SR_AD_SENSORS_DATA.ANS2[1]=(uint8_t)(an_voltage>>8);
	EGTS_SR_AD_SENSORS_DATA.ANS2[2]=(uint8_t)(an_voltage>>16);

	SUBRECORD.SRT=EGTS_SR_AD_SENSORS_DATA_TYPE;
	SUBRECORD.SRL=sizeof(EGTS_SR_AD_SENSORS_DATA);
	if (SUBRECORD.SRL>SRD_MAX_LENGTH) return;
	srd_uk=(uint8_t*)&EGTS_SR_AD_SENSORS_DATA;
	for (uint16_t	mrn=0; mrn<SUBRECORD.SRL; mrn++)
		SUBRECORD.SRD[mrn]=srd_uk[mrn];		
}
void EGTS_SR_STATE_DATA_Subrecord_Prepare(void)
{
	EGTS_SR_STATE_DATA.ST=2;
	EGTS_SR_STATE_DATA.MPSV=(uint8_t)(in_kontakt.pwr*10);
	EGTS_SR_STATE_DATA.BBV=0;
	EGTS_SR_STATE_DATA.IBV=0;
	EGTS_SR_STATE_DATA.FL=0;
	EGTS_SR_STATE_DATA.FL|=NMS_FLAG;
	
	SUBRECORD.SRT=EGTS_SR_STATE_DATA_TYPE;
	SUBRECORD.SRL=sizeof(EGTS_SR_STATE_DATA);
	if (SUBRECORD.SRL>SRD_MAX_LENGTH) return;
	srd_uk=(uint8_t*)&EGTS_SR_STATE_DATA;
	for (uint16_t	mrn=0; mrn<SUBRECORD.SRL; mrn++)
		SUBRECORD.SRD[mrn]=srd_uk[mrn];	
}
void EGTS_SR_ABS_AN_SENS_DATA_Subrecord_Prepare(uint8_t inn)
{
	EGTS_SR_ABS_AN_SENS_DATA.ASN=inn;
	switch (inn)
	{
		case 0:
			an_voltage=(uint32_t)(in_kontakt.ain1*1000);
			break;
		case 1:
			an_voltage=(uint32_t)(in_kontakt.ain2*1000);
			break;
		default:
			an_voltage=(uint32_t)(in_kontakt.ain1*1000);
			break;
	}
	EGTS_SR_ABS_AN_SENS_DATA.ASV[0]=(uint8_t)an_voltage;
	EGTS_SR_ABS_AN_SENS_DATA.ASV[1]=(uint8_t)(an_voltage>>8);
	EGTS_SR_ABS_AN_SENS_DATA.ASV[2]=(uint8_t)(an_voltage>>16);
	
	SUBRECORD.SRT=EGTS_SR_ABS_AN_SENS_DATA_TYPE;
	SUBRECORD.SRL=sizeof(EGTS_SR_ABS_AN_SENS_DATA);
	if (SUBRECORD.SRL>SRD_MAX_LENGTH) return;
	srd_uk=(uint8_t*)&EGTS_SR_ABS_AN_SENS_DATA;
	for (uint16_t	mrn=0; mrn<SUBRECORD.SRL; mrn++)
		SUBRECORD.SRD[mrn]=srd_uk[mrn];	
}
void EGTS_SR_LIQUID_LEVEL_SENSOR_Subrecord_Prepare(uint8_t dut_number)
{
	EGTS_SR_LIQUID_LEVEL_SENSOR.FL=dut_number;
	EGTS_SR_LIQUID_LEVEL_SENSOR.LLSD=blackbox.level[dut_number];
	
	SUBRECORD.SRT=EGTS_SR_LIQUID_LEVEL_SENSOR_TYPE;
	SUBRECORD.SRL=sizeof(EGTS_SR_LIQUID_LEVEL_SENSOR);
	if (SUBRECORD.SRL>SRD_MAX_LENGTH) return;
	srd_uk=(uint8_t*)&EGTS_SR_LIQUID_LEVEL_SENSOR;
	for (uint16_t	mrn=0; mrn<SUBRECORD.SRL; mrn++)
		SUBRECORD.SRD[mrn]=srd_uk[mrn];	
}
void EGTS_SR_POS_DATA_Subrecord_Prepare(void)
{
	double xx=0, uui=0;
	
	EGTS_SR_POS_DATA.NTM=Calculate_Blackbox_TM();
	
	xx=atof((char*)blackbox.latitude);
	xx/=100;
	xx=modf(xx, &uui); 
	uui +=xx/0.6; 
	EGTS_SR_POS_DATA.LAT=(uint32_t)((uui/90)*0xffffffff);
	
	
	xx=atof((char*)blackbox.longitude);
	xx/=100;
	xx=modf(xx, &uui); 
	uui +=xx/0.6; 
	EGTS_SR_POS_DATA.LONG=(uint32_t)((uui/180)*0xffffffff);
	
	EGTS_SR_POS_DATA.FLG=0;
	if (blackbox.lon[0]=='W') EGTS_SR_POS_DATA.FLG|=LOHS_FLAG;
	if (blackbox.lat[0]=='S') EGTS_SR_POS_DATA.FLG|=LAHS_FLAG;
	
	xx=atof((char*)blackbox.speed);
	xx*=10;
	if (xx)	EGTS_SR_POS_DATA.FLG|=MV_FLAG;
	if (blackbox.gnss_status=='A')
		EGTS_SR_POS_DATA.FLG|=VLD_FLAG;
	EGTS_SR_POS_DATA.SPD=(uint16_t)xx;
	
	xx=atof((char*)blackbox.course);
	EGTS_SR_POS_DATA.DIR=(uint8_t)xx;

	if (0x0100 & (uint16_t)xx) EGTS_SR_POS_DATA.SPD|=DIRH_FLAG;
	
	if (blackbox.point_make_src&TIMER_UPDATE)
			EGTS_SR_POS_DATA.SRC=5;
	else 	if (blackbox.point_make_src&FIRST_MSG)
			EGTS_SR_POS_DATA.SRC=32;
	else 	if (blackbox.point_make_src&IN_KONTAKT_CHANGED)
			EGTS_SR_POS_DATA.SRC=2;
	else 	if (blackbox.point_make_src&FIRST_KOORDINATES)
			EGTS_SR_POS_DATA.SRC=30;
	else 	if (blackbox.point_make_src&COURSE_CHANGED)
			EGTS_SR_POS_DATA.SRC=2;
	
	EGTS_SR_POS_DATA.DIN=0;
	if ((kp.mode_in1==DIGITAL_IN_MODE)&&(in_kontakt.sk1))	EGTS_SR_POS_DATA.DIN|=0x01;
	if ((kp.mode_in2==DIGITAL_IN_MODE)&&(in_kontakt.sk2))	EGTS_SR_POS_DATA.DIN|=0x02;
	
	SUBRECORD.SRT=EGTS_SR_POS_DATA_TYPE;
	SUBRECORD.SRL=sizeof(EGTS_SR_POS_DATA);
	if (SUBRECORD.SRL>SRD_MAX_LENGTH) return;
	srd_uk=(uint8_t*)&EGTS_SR_POS_DATA;
	for (uint16_t	mrn=0; mrn<SUBRECORD.SRL; mrn++)
		SUBRECORD.SRD[mrn]=srd_uk[mrn];
}
void EGTS_SR_EXT_POS_DATA_Subrecord_Prepare(void)
{
	EGTS_SR_EXT_POS_DATA.FL=0;
	EGTS_SR_EXT_POS_DATA.FL|=SFE_FLAG;
	EGTS_SR_EXT_POS_DATA.FL|=HFE_FLAG;
	EGTS_SR_EXT_POS_DATA.HDOP=(uint16_t)(atof((char*)blackbox.hdop)*100);
	EGTS_SR_EXT_POS_DATA.SAT=(uint8_t)atof((char*)blackbox.sats);
	
	SUBRECORD.SRT=EGTS_SR_EXT_POS_DATA_TYPE;
	SUBRECORD.SRL=sizeof(EGTS_SR_EXT_POS_DATA);
	if (SUBRECORD.SRL>SRD_MAX_LENGTH) return;
	srd_uk=(uint8_t*)&EGTS_SR_EXT_POS_DATA;
	for (uint16_t	mrn=0; mrn<SUBRECORD.SRL; mrn++)
		SUBRECORD.SRD[mrn]=srd_uk[mrn];
}
uint16_t EGTS_Init_Login(void)
{
	uint16_t pack_len=0;
	
	EGTS_SR_TERM_IDENTITY_Subrecord_Prepare();
	Record_Prepare(EGTS_AUTH_SERVICE);
	Packet_Prepare();
	pack_len=sizeof(EGTS_PACKET)-SFRD_MAX_LENGTH+EGTS_PACKET.FDL;
	rd_uk=(uint8_t*)&EGTS_PACKET;
	for (uint16_t ccc=0; ccc<pack_len; ccc++)
		gsm.msg[ccc]=rd_uk[ccc];
	return pack_len;
}
uint16_t Format_Half_Word(uint16_t	hlfw)
{
	uint16_t m_cnt=0;
	m_cnt=hlfw>>8;
	m_cnt|=hlfw<<8;
	return (m_cnt);
}
void Packet_Prepare(void)
{
	uint16_t m_cnt=0;
	
	EGTS_PACKET.PRV=1;
	EGTS_PACKET.SKID=0;
	EGTS_PACKET.FLAGS=0x02;
	EGTS_PACKET.HE=0;
	EGTS_PACKET.HL=0x0B;
	EGTS_PACKET.PT=EGTS_PT_APPDATA;
	EGTS_PACKET.FDL=sizeof(RECORD)-RD_MAX_LENGTH+RECORD.RL;

	
	if (EGTS_PACKET.FDL>SFRD_MAX_LENGTH) return;
	rd_uk=(uint8_t*)&RECORD;
	EGTS_PACKET.SFRCS=0xFFFF;
	for (uint16_t mrn=0; mrn<EGTS_PACKET.FDL; mrn++)
	{
		EGTS_PACKET.SFRD[m_cnt++]=rd_uk[mrn];
		EGTS_PACKET.SFRCS=egts_Crc16_byte(rd_uk[mrn], EGTS_PACKET.SFRCS);
	}
	EGTS_PACKET.SFRD[m_cnt++]=(uint8_t)EGTS_PACKET.SFRCS;
	EGTS_PACKET.SFRD[m_cnt++]=(uint8_t)(EGTS_PACKET.SFRCS >> 8);
	
	EGTS_PACKET.HCS=crc8((unsigned char*)&EGTS_PACKET, EGTS_PACKET.HL-1);
	
}
uint32_t Calculate_Blackbox_TM(void)
{
	char pp[5]={0};
	
	pp[0]=blackbox.date[0];
	pp[1]=blackbox.date[1];
	pp[2]=0;
	days=atoi(pp);
	pp[0]=blackbox.date[2];
	pp[1]=blackbox.date[3];
	pp[2]=0;	
	
	tmon=atoi(pp)-1;
	pp[0]=blackbox.date[4];
	pp[1]=blackbox.date[5];
	pp[2]=0;
	 
	tyear=atoi(pp);

	days +=tyear*365+ /*+количество дней в високосных годах*/ tyear/4;

	if(!(tyear%4) && (tmon<2))	days--;   //если год високосный, идут 1-е 2 мес и не 29 число то убавляем день
	                                     
    while (tmon !=0) { 	days +=day_in_mon[--tmon];	};
	
	Tsek =	24*3600*days;
	
	pp[0]=blackbox.time[0];
	pp[1]=blackbox.time[1];
	pp[2]=0;
	Tsek += atoi(pp)*3600;
		
	pp[0]=blackbox.time[2];
	pp[1]=blackbox.time[3];
	pp[2]=0;
	Tsek+=atoi(pp)*60;
	pp[0]=blackbox.time[4];
	pp[1]=blackbox.time[5];
	pp[2]=0;
	Tsek+=atoi(pp);
	
	Tsek-=timshift;
		
	return Tsek;
}
uint32_t Calculate_Current_TM(void)
{	
	days=calendar.day;
	
	tmon=calendar.month-1;

	tyear=calendar.year;

	days +=tyear*365+ /*+количество дней в високосных годах*/ tyear/4;

	if(!(tyear%4) && (tmon<2))	days--;   //если год високосный, идут 1-е 2 мес и не 29 число то убавляем день
	                                     
    while (tmon !=0) { 	days +=day_in_mon[--tmon];	};
	
	Tsek =	24*3600*days;
	Tsek += calendar.hour*3600;	
	Tsek+=calendar.min*60;
	Tsek+=calendar.sec;	
	Tsek-=timshift;
		
	return Tsek;
}
void Record_Data_Copy(void)
{
	struc_len=sizeof(SUBRECORD)-SRD_MAX_LENGTH+SUBRECORD.SRL;
	if (RECORD.RL+struc_len>RD_MAX_LENGTH) return;
	rd_uk=(uint8_t*)&SUBRECORD;
	for (uint16_t mrn=0; mrn<struc_len; mrn++)
		RECORD.RD[RECORD.RL+mrn]=rd_uk[mrn];
	RECORD.RL+=struc_len;
}
void Record_Prepare(uint8_t	rec_service)
{
	switch (rec_service)
	{
		case EGTS_AUTH_SERVICE:
			RECORD.RFL=0;
			RECORD.RFL|=SSOD_FLAG;
			RECORD.RFL|=OBFE_FLAG;
		
			RECORD.RFL|=RPP_MEDIUM_FLAG;
			RECORD.SST=EGTS_AUTH_SERVICE;
			RECORD.RST=EGTS_AUTH_SERVICE;
			RECORD.OID=(uint32_t)atol(kp.id_dev);
			RECORD.RL=0;
			Record_Data_Copy();
			break;
		case EGTS_TELEDATA_SERVICE:
			RECORD.RFL=0;
			RECORD.RFL|=SSOD_FLAG;
			RECORD.RFL|=OBFE_FLAG;
			RECORD.RFL|=RPP_MEDIUM_FLAG;
			RECORD.SST=EGTS_TELEDATA_SERVICE;
			RECORD.RST=EGTS_TELEDATA_SERVICE;
			RECORD.OID=(uint32_t)atol(kp.id_dev);
			RECORD.RL=0;
		
			//EGTS_SR_POS_DATA
			EGTS_SR_POS_DATA_Subrecord_Prepare();
			Record_Data_Copy();
		
			//EGTS_SR_LIQUID_LEVEL_SENSOR
			for (uint8_t cc=0; cc<kp.dut_count; cc++)
			{	
				EGTS_SR_LIQUID_LEVEL_SENSOR_Subrecord_Prepare(cc);	
				Record_Data_Copy();
			}
			//EGTS_SR_EXT_POS_DATA
			if (blackbox.gnss_status=='A')
			{
				EGTS_SR_EXT_POS_DATA_Subrecord_Prepare();
				Record_Data_Copy();
			}
			//EGTS_SR_ABS_AN_SENS_DATA
//			if (kp.mode_in1==ANALOGUE_IN_MODE)	
//			{
//				EGTS_SR_ABS_AN_SENS_DATA_Subrecord_Prepare(0);
//				Record_Data_Copy();
//			}
//			if (kp.mode_in2==ANALOGUE_IN_MODE)	
//			{
//				EGTS_SR_ABS_AN_SENS_DATA_Subrecord_Prepare(1);
//				Record_Data_Copy();
//			}


			//EGTS_SR_AD_SENSORS_DATA
			EGTS_SR_AD_SENSORS_DATA_Subrecord_Prepare();
			Record_Data_Copy();
			
			//EGTS_SR_STATE_DATA
			EGTS_SR_STATE_DATA_Subrecord_Prepare();
			Record_Data_Copy();
			break;
		default:
			break;
	}
}
void EGTS_SR_TERM_IDENTITY_Subrecord_Prepare(void)
{
	EGTS_SR_TERM_IDENTITY.TID=(uint32_t)atol(kp.id_dev);
	EGTS_SR_TERM_IDENTITY.FLAGS=0;
	EGTS_SR_TERM_IDENTITY.FLAGS|=BSE_FLAG;
//	EGTS_SR_TERM_IDENTITY.FLAGS|=LNGCE_FLAG;
//	EGTS_SR_TERM_IDENTITY.FLAGS|=IMSIE_FLAG;
	EGTS_SR_TERM_IDENTITY.FLAGS|=IMEIE_FLAG;
	
	for (uint8_t	mrn=0; mrn<15; mrn++)
		EGTS_SR_TERM_IDENTITY.IMEI[mrn]=gsm.imei[mrn];
//	for (uint8_t	mrn=0; mrn<16; mrn++)
//		EGTS_SR_TERM_IDENTITY.IMSI[mrn]=gsm.imsi[mrn];	
//	EGTS_SR_TERM_IDENTITY.LNGC[0]='r';
//	EGTS_SR_TERM_IDENTITY.LNGC[1]='u';
//	EGTS_SR_TERM_IDENTITY.LNGC[2]='s';
	EGTS_SR_TERM_IDENTITY.BS=256;
	
	
	
	SUBRECORD.SRT=EGTS_SR_TERM_IDENTITY_TYPE;
	SUBRECORD.SRL=sizeof(EGTS_SR_TERM_IDENTITY);
	if (SUBRECORD.SRL>SRD_MAX_LENGTH) return;
	srd_uk=(uint8_t*)&EGTS_SR_TERM_IDENTITY;
	for (uint16_t	mrn=0; mrn<SUBRECORD.SRL; mrn++)
		SUBRECORD.SRD[mrn]=srd_uk[mrn];
		
}
uint8_t	Check_Receive_Packet(uint8_t *r_buf)
{
	uint8_t res=0, len=r_buf[3];
	
	if (len>10) len--;
	else return 0;
	if (r_buf[0]==0x01) res++;
	if (crc8(r_buf, len)==r_buf[len]) res++;
	if (res==2)
		return 1;
	else
		return 0;	
}
void Data_Response_OK(void)
{
	Wait_Ans_Clear();
	gsm.state++; 		
	gsm.rst=0;
	gsm.rst_timeout=0;
}
void Next_Step(void)
{
	if (egts_parse.rx_cnt<SFRD)
		egts_parse.rx_cnt++;
	else
		if (egts_parse.sfr_cnt<SDR1)
			egts_parse.sfr_cnt++;
		else
			if (egts_parse.sdr_cnt<RD)
				egts_parse.sdr_cnt++;
			else
				if (egts_parse.rd_cnt<SRD)
					egts_parse.rd_cnt++;
				else
					Parse_Error();
	egts_parse.parse_timeout=MAX_PARSE_TIMEOUT;
}
void Parse_EGTS_Data(uint8_t rx_b)
{
	switch (egts_parse.rx_cnt)
	{
		case PRV:
		{
			egts_parse.PRV=rx_b;
			egts_parse.HCS=0xFF;
			egts_parse.SFRCS=0xFFFF;
			egts_parse.HCS=crc8_byte(rx_b, egts_parse.HCS);
			egts_parse.sfr_cnt=0;
			egts_parse.rd_cnt=0;
			//поддерживаемые версии протокола
			switch (egts_parse.PRV)
			{
				case 1:
					Next_Step();
					break;
				default:
					break;
			}
		}break;
		case SKID:
		{
			egts_parse.SKID=rx_b;
			egts_parse.HCS=crc8_byte(rx_b, egts_parse.HCS);
			Next_Step();
		}break;
		case FL:
		{
			egts_parse.FL=rx_b;
//			fl_index=(uint8_t*)&egts_parse.FL;
//			*fl_index=rx_b;
			egts_parse.HCS=crc8_byte(rx_b, egts_parse.HCS);
//			if (egts_parse.FL.PRF) Parse_Error();	
//			if (egts_parse.FL.ENA) Parse_Error();	//шифрование не поддерживаем
//			if (egts_parse.FL.CMP) Parse_Error();	//сжатие не поддерживаем
			Next_Step();
		}break;
		case HL:
		{
			egts_parse.HL=rx_b;
			egts_parse.HCS=crc8_byte(rx_b, egts_parse.HCS);
			Next_Step();
		}break;
		case HE:
		{
			egts_parse.HE=rx_b;
			egts_parse.HCS=crc8_byte(rx_b, egts_parse.HCS);
			Next_Step();
		}break;
		case FDL:
		{
			if (!egts_parse.half_word_cnt)
			{
				egts_parse.FDL=(uint16_t)rx_b;
				egts_parse.HCS=crc8_byte(rx_b, egts_parse.HCS);
				egts_parse.half_word_cnt++;
			} 
			else
			{
				egts_parse.FDL|=(uint16_t)rx_b<<8;
				egts_parse.HCS=crc8_byte(rx_b, egts_parse.HCS);
				egts_parse.half_word_cnt=0;
				Next_Step();
			}
		}break;
		case PID:
		{
			if (!egts_parse.half_word_cnt)
			{
				egts_parse.PID=(uint16_t)rx_b;
				egts_parse.HCS=crc8_byte(rx_b, egts_parse.HCS);
				egts_parse.half_word_cnt++;
			} 
			else
			{
				egts_parse.PID|=(uint16_t)rx_b<<8;
				egts_parse.HCS=crc8_byte(rx_b, egts_parse.HCS);
				egts_parse.half_word_cnt=0;
				Next_Step();
			}
		}break;
		case PT:
		{
			egts_parse.PT=rx_b;
			egts_parse.HCS=crc8_byte(rx_b, egts_parse.HCS);
			if (~egts_parse.FL&RTE_FLAG) 
				egts_parse.rx_cnt=TTL;
			Next_Step();
		}break;
		case PRA:
		{
			egts_parse.PRA=rx_b;
			egts_parse.HCS=crc8_byte(rx_b, egts_parse.HCS);
			Next_Step();
		}break;
		case RCA:
		{
			egts_parse.RCA=rx_b;
			egts_parse.HCS=crc8_byte(rx_b, egts_parse.HCS);
			Next_Step();
		}break;
		case TTL:
		{
			egts_parse.TTL=rx_b;
			egts_parse.HCS=crc8_byte(rx_b, egts_parse.HCS);
			Next_Step();
		}break;
		case HCS:
		{
			if (egts_parse.HCS==rx_b)
				Next_Step();
			else
			{
				Parse_Error();
			}
		}break;
		case SFRD:
		{
			egts_parse.SFRCS=egts_Crc16_byte(rx_b, egts_parse.SFRCS);
			switch (egts_parse.PT)
			{
				case EGTS_PT_RESPONSE:
				{
					switch (egts_parse.sfr_cnt)
					{
						case RPID:
						{
							if (!egts_parse.half_word_cnt)
							{
								egts_parse.RPID=(uint16_t)rx_b;
								egts_parse.half_word_cnt++;
							} 
							else
							{
								egts_parse.RPID|=(uint16_t)rx_b<<8;
								egts_parse.half_word_cnt=0;
								Next_Step();
							}
						} break;
						case PR:
						{
							egts_parse.PR=rx_b;
							Next_Step();
						} break;
						case SDR1:
						{
							switch (egts_parse.sdr_cnt)
							{
								case RL:
								{
									if (!egts_parse.half_word_cnt)
									{
										egts_parse.RL=(uint16_t)rx_b;
										egts_parse.half_word_cnt++;
									} 
									else
									{
										egts_parse.RL|=(uint16_t)rx_b<<8;
										egts_parse.half_word_cnt=0;
										Next_Step();
									}						
								} break;
								case RN:
								{
									if (!egts_parse.half_word_cnt)
									{
										egts_parse.RN=(uint16_t)rx_b;
										egts_parse.half_word_cnt++;
									} 
									else
									{
										egts_parse.RN|=(uint16_t)rx_b<<8;
										egts_parse.half_word_cnt=0;
										Next_Step();
									}										
								} break;
								case RFL:
								{
									egts_parse.RFL=rx_b;
									Next_Step();
									if (~egts_parse.RFL&OBFE_FLAG) egts_parse.sdr_cnt=EVID;
									if (~egts_parse.RFL&EVFE_FLAG) egts_parse.sdr_cnt=TM;
									if (~egts_parse.RFL&TMFE_FLAG) egts_parse.sdr_cnt=SST;
								} break;
								case OID:
								{
									if (egts_parse.half_word_cnt<3)
										egts_parse.half_word_cnt++;
									else
									{
										egts_parse.half_word_cnt=0;
										Next_Step();
									}
								} break;
								case EVID:
								{
									if (egts_parse.half_word_cnt<3)
										egts_parse.half_word_cnt++;
									else
									{
										egts_parse.half_word_cnt=0;
										Next_Step();
									}
								} break;
								case TM:
								{
									if (egts_parse.half_word_cnt<3)
										egts_parse.half_word_cnt++;
									else
									{
										egts_parse.half_word_cnt=0;
										Next_Step();
									}
								} break;
								case SST:
								{
									egts_parse.SST=rx_b;
									Next_Step();
								} break;
								case RST:
								{
									egts_parse.RST=rx_b;
									Next_Step();
								} break;
								case RD:
								{
									if (egts_parse.RL) egts_parse.RL--;
									switch (egts_parse.rd_cnt)
									{
										case SRT:
										{
											egts_parse.SRT=rx_b;
											Next_Step();
										}	break;
										case SRL:
										{
											if (!egts_parse.half_word_cnt)
											{
												egts_parse.SRL=(uint16_t)rx_b;
												egts_parse.half_word_cnt++;
											} 
											else
											{
												egts_parse.SRL|=(uint16_t)rx_b<<8;
												egts_parse.half_word_cnt=0;
												Next_Step();
											}
										}	break;
										case SRD:
										{
											egts_parse.SRL--;
											if (!egts_parse.SRL)
											{
												if (!egts_parse.RL)
													egts_parse.rx_cnt=SFRCS;
												else
													egts_parse.sdr_cnt=RD;
											}
										}	break;
										default:
											Parse_Error();
											break;
										
									}
								} break;
								default:
									Parse_Error();
									break;
								
							}
						} break;
						default:
							Parse_Error();
							break;
					}
				}	break;
				case EGTS_PT_APPDATA:
				{
					egts_parse.sfr_cnt=SDR1;
					switch (egts_parse.sdr_cnt)
					{
						case RL:
						{
							if (!egts_parse.half_word_cnt)
							{
								egts_parse.RL=(uint16_t)rx_b;
								egts_parse.half_word_cnt++;
							} 
							else
							{
								egts_parse.RL|=(uint16_t)rx_b<<8;
								egts_parse.half_word_cnt=0;
								Next_Step();
							}						
						} break;
						case RN:
						{
							if (!egts_parse.half_word_cnt)
							{
								egts_parse.RN=(uint16_t)rx_b;
								egts_parse.half_word_cnt++;
							} 
							else
							{
								egts_parse.RN|=(uint16_t)rx_b<<8;
								egts_parse.half_word_cnt=0;
								Next_Step();
							}										
						} break;
						case RFL:
						{
							egts_parse.RFL=rx_b;
							Next_Step();
							if (~egts_parse.RFL&OBFE_FLAG) egts_parse.sdr_cnt=EVID;
							if (~egts_parse.RFL&EVFE_FLAG) egts_parse.sdr_cnt=TM;
							if (~egts_parse.RFL&TMFE_FLAG) egts_parse.sdr_cnt=SST;
						} break;
						case OID:
						{
							if (egts_parse.half_word_cnt<3)
								egts_parse.half_word_cnt++;
							else
							{
								egts_parse.half_word_cnt=0;
								Next_Step();
							}
						} break;
						case EVID:
						{
							if (egts_parse.half_word_cnt<3)
								egts_parse.half_word_cnt++;
							else
							{
								egts_parse.half_word_cnt=0;
								Next_Step();
							}
						} break;
						case TM:
						{
							if (egts_parse.half_word_cnt<3)
								egts_parse.half_word_cnt++;
							else
							{
								egts_parse.half_word_cnt=0;
								Next_Step();
							}
						} break;
						case SST:
						{
							egts_parse.SST=rx_b;
							Next_Step();
						} break;
						case RST:
						{
							egts_parse.RST=rx_b;
							Next_Step();
						} break;
						case RD:
						{
							if (egts_parse.RL) egts_parse.RL--;
							switch (egts_parse.rd_cnt)
							{
								case SRT:
								{
									egts_parse.SRT=rx_b;
									Next_Step();
								}	break;
								case SRL:
								{
									if (!egts_parse.half_word_cnt)
									{
										egts_parse.SRL=(uint16_t)rx_b;
										egts_parse.half_word_cnt++;
									} 
									else
									{
										egts_parse.SRL|=(uint16_t)rx_b<<8;
										egts_parse.half_word_cnt=0;
										Next_Step();
									}
								}	break;
								case SRD:
								{
									egts_parse.SRL--;
									if (!egts_parse.SRL)
									{			
										if (!egts_parse.RL)
											egts_parse.rx_cnt=SFRCS;
										else
											egts_parse.sdr_cnt=RD;
									}
								}	break;
								default:
									Parse_Error();
									break;
								
							}
						} break;
						default:
							Parse_Error();
							break;
						
					}					
				} break;
				case EGTS_PT_SIGNED_APPDATA:
				{
					Parse_Error();//не поддерживаем
				} break;
			}
		} break;
		case SFRCS:
		{
			if (!egts_parse.half_word_cnt)
			{
				egts_parse.SFRCS_received=(uint16_t)rx_b;
				egts_parse.half_word_cnt++;
			} 
			else
			{
				egts_parse.SFRCS_received|=(uint16_t)rx_b<<8;
				egts_parse.half_word_cnt=0;
				if (egts_parse.SFRCS_received==egts_parse.SFRCS)
				{
					//успех
					egts_parse.rx_cnt=0;
					egts_parse.sfr_cnt=0;
					egts_parse.sdr_cnt=0;
					egts_parse.rd_cnt=0;
					egts_parse.parse_timeout=0;
					egts_parse.parsing_ongoing=0;
					switch (egts_parse.PT)
					{
						case EGTS_PT_RESPONSE:
						{
							if ((egts_parse.PR==EGTS_PC_OK)&&(egts_parse.RST==EGTS_AUTH_SERVICE)&&(egts_parse.SST==EGTS_AUTH_SERVICE))//ответ на логин
							{
								egts_parse.login_successful=1;
								Data_Response_OK();
							}
							if ((egts_parse.PR==EGTS_PC_OK)&&(egts_parse.RST==EGTS_TELEDATA_SERVICE)&&(egts_parse.SST==EGTS_TELEDATA_SERVICE))//ответ на пакет данных
							{
								gsm.ASD=1;
								gsm.sending_timeout=NO_SENDING_DATA_TIMEOUT;
								RECORD.RN++;
								EGTS_PACKET.PID++;
							}
						} break;
						default:
							break;
					}
				}
				else
						Parse_Error();
					
			}			
		} break;			
			
		default:
			egts_parse.rx_cnt=0;
			break;
	}
	
}
uint8_t	crc8_byte(uint8_t b, uint8_t crc_old)
{
	crc_old ^= b; 
	for(uint8_t i = 0; i < 8; i++) 
		crc_old = crc_old & 0x80 ? (crc_old << 1) ^ 0x31 : crc_old << 1; 
	return crc_old;
}
uint8_t crc8(unsigned char *pcBlock, uint16_t len) 
{  
	uint8_t crc = 0xFF;
	unsigned char i; 
	while(len--)  
		crc=crc8_byte(*pcBlock++, crc);
	return crc;
}
const unsigned short egts_Crc16Table[256] = 
{
0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A, 
0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, 
0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49, 
0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067, 
0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 
0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634, 
0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB, 
0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3, 
0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A, 
0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92, 
0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9, 
0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8, 
0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};
unsigned short egts_Crc16_byte(unsigned char c_byte, unsigned short old_crc)
{
	return (old_crc<<8)^egts_Crc16Table[(old_crc>> 8) ^ c_byte];
	
}
unsigned short egts_Crc16(unsigned char* pcBlock, unsigned short len)
{
	unsigned short crc = 0xffff;
	while (len--)
		crc=egts_Crc16_byte(*pcBlock++, crc);
	return crc;
}



