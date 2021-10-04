typedef struct
{
  uint8_t   sec;
  uint8_t   min;
  uint8_t   hour;
  uint8_t   day;
  uint8_t   month;
  uint8_t   year;
  uint16_t  timer;
  uint8_t   update_from_gnss;
  uint8_t   update_from_ntp;
  uint32_t  update_counter;
  uint8_t   actual;
} CALENDAR_STRUC;


void Calendar_Serv(void);
void Calendar_Update_From_GNSS(void);
void Calendar_Update_From_NTP(void);
void Calendar_Refresh(void);
void Calendar_Update(void);

extern CALENDAR_STRUC calendar;



