#define POINT_MAKE_TIMER								TIM1
#define POINT_MAKE_TIMER_RESOLUTION			5//ms

#define USRED_KOORD         4
#define PROCESSING_PERIOD   (uint16_t)500    //период адаптивного формирования точек трека
#define SEND_PERIOD         (uint32_t)30000
#define MIN_PERIOD          (uint32_t)5000
#define MIN_TIME_RESOLUTION (uint32_t)2000

//причины отметки точки на треке
#define TIMER_UPDATE        (uint8_t)0x01
#define FIRST_MSG           (uint8_t)0x02
#define IN_KONTAKT_CHANGED  (uint8_t)0x04
#define iBUTTON_CHANGED     (uint8_t)0x08
#define FIRST_KOORDINATES   (uint8_t)0x10
#define COURSE_CHANGED      (uint8_t)0x20

typedef struct
{
  uint8_t       rst;
  uint16_t      processing_timer;
  uint32_t      timer_period;
  uint32_t      timer;
  float         sr_vel;
  float         sr_ang;
  float         fix_ang;
  uint8_t       first_koord;
  uint8_t       first_msg;
  uint8_t       point_make_src;
	uint8_t				moving;
	char          fix_koord[50];
} POINT_MAKE_STRUC;

extern POINT_MAKE_STRUC point_make;


void Point_Make_Serv(void);
void Point_Make_ms_timer(void);
void Point_Make_Reset(void);
void Point_Make_Processing(void);
void Point_Make_Handler(void);





