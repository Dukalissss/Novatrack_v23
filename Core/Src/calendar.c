#include "main.h"

CALENDAR_STRUC calendar;

void Calendar_Serv(void)
{
  if (calendar.update_from_gnss) Calendar_Update_From_GNSS();
  if (calendar.update_from_ntp) Calendar_Update_From_NTP();
  calendar.timer+=t1_ms;
  if (calendar.timer<500) return;
  calendar.timer=0;
  Calendar_Refresh();
}
void Calendar_Update_From_GNSS(void)
{
  uint8_t buff[3];
	buff[0]=gnss.date[0];
	buff[1]=gnss.date[1];
	calendar.day=(uint8_t)atoi((char*)buff);
	buff[0]=gnss.date[2];
	buff[1]=gnss.date[3];
	calendar.month=(uint8_t)atoi((char*)buff);
	buff[0]=gnss.date[4];
	buff[1]=gnss.date[5];
	calendar.year=(uint8_t)atoi((char*)buff);
	buff[0]=gnss.time[0];
	buff[1]=gnss.time[1];
	calendar.hour=(uint8_t)atoi((char*)buff);
	buff[0]=gnss.time[2];
	buff[1]=gnss.time[3];
	calendar.min=(uint8_t)atoi((char*)buff);
	buff[0]=gnss.time[4];
	buff[1]=gnss.time[5];
	calendar.sec=(uint8_t)atoi((char*)buff);  
  
  Calendar_Update();

  calendar.update_from_gnss=0;
  calendar.actual=1;
}
void Calendar_Update_From_NTP(void)
{
  Calendar_Update();
  calendar.update_from_ntp=0;
  calendar.actual=1;
}
void Calendar_Update(void)
{
  LL_RTC_DisableWriteProtection(RTC);
  LL_RTC_EnableInitMode(RTC);
  while (!LL_RTC_IsActiveFlag_INIT(RTC)) {};
  LL_RTC_TIME_SetHour(RTC, __LL_RTC_CONVERT_BIN2BCD(calendar.hour));
  LL_RTC_DisableInitMode(RTC);
  LL_RTC_EnableWriteProtection(RTC);
  while (!LL_RTC_IsActiveFlag_RS(RTC)) {};
       
  LL_RTC_DisableWriteProtection(RTC);
  LL_RTC_EnableInitMode(RTC);
  while (!LL_RTC_IsActiveFlag_INIT(RTC)) {};    
  LL_RTC_TIME_SetMinute(RTC, __LL_RTC_CONVERT_BIN2BCD(calendar.min));
  LL_RTC_DisableInitMode(RTC);
  LL_RTC_EnableWriteProtection(RTC);
  while (!LL_RTC_IsActiveFlag_RS(RTC)) {};
  
  LL_RTC_DisableWriteProtection(RTC);
  LL_RTC_EnableInitMode(RTC);
  while (!LL_RTC_IsActiveFlag_INIT(RTC)) {};    
  LL_RTC_TIME_SetSecond(RTC, __LL_RTC_CONVERT_BIN2BCD(calendar.sec));
  LL_RTC_DisableInitMode(RTC);
  LL_RTC_EnableWriteProtection(RTC);
  while (!LL_RTC_IsActiveFlag_RS(RTC)) {};
  LL_RTC_DisableWriteProtection(RTC);
  LL_RTC_EnableInitMode(RTC);
  while (!LL_RTC_IsActiveFlag_INIT(RTC)) {};     
  LL_RTC_DATE_SetDay(RTC, __LL_RTC_CONVERT_BIN2BCD(calendar.day));
  LL_RTC_DisableInitMode(RTC);
  LL_RTC_EnableWriteProtection(RTC);
  while (!LL_RTC_IsActiveFlag_RS(RTC)) {};
  LL_RTC_DisableWriteProtection(RTC);
  LL_RTC_EnableInitMode(RTC);
  while (!LL_RTC_IsActiveFlag_INIT(RTC)) {};        
  LL_RTC_DATE_SetMonth(RTC, __LL_RTC_CONVERT_BIN2BCD(calendar.month));
  LL_RTC_DisableInitMode(RTC);
  LL_RTC_EnableWriteProtection(RTC);
  while (!LL_RTC_IsActiveFlag_RS(RTC)) {};
  LL_RTC_DisableWriteProtection(RTC);
  LL_RTC_EnableInitMode(RTC);
  while (!LL_RTC_IsActiveFlag_INIT(RTC)) {};        
  LL_RTC_DATE_SetYear(RTC, __LL_RTC_CONVERT_BIN2BCD(calendar.year));
  LL_RTC_DisableInitMode(RTC);
  LL_RTC_EnableWriteProtection(RTC);
  while (!LL_RTC_IsActiveFlag_RS(RTC)) {};
}
void Calendar_Refresh(void)
{
  calendar.update_counter++;
  if (LL_RTC_IsActiveFlag_RS(RTC))
  {
    calendar.sec=__LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetSecond(RTC));//
    calendar.min=__LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetMinute(RTC));
    calendar.hour=__LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetHour(RTC));
    calendar.day=__LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetDay(RTC));
    calendar.month=__LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetMonth(RTC));
    calendar.year=__LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetYear(RTC));
  }
}


