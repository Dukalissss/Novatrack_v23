#define KALIBR_ADDR             (uint32_t)0x0801F800
#define RESET_COUNTER						TIM7  
	
	
/*TD_ONLINE BEGIN********************************************************************************************/
#define GET_STATUS      (uint8_t)0xA0
#define ZAPROS_UPR      (uint8_t)0xA1
#define GET_CONFIG      (uint8_t)0xA2
#define PUT_CONFIG      (uint8_t)0xA3
#define SET_SERIAL      (uint8_t)0xA4	
#define PUT_WI          (uint8_t)0xA5
#define	SET_BORT_P      (uint8_t)0xA6
#define GET_IMEI_IMSI   (uint8_t)0xA7
#define GET_PARAM       (uint8_t)0xA8
#define OUT_DATA        (uint8_t)0x06    //Команда на выдачу данных
#define SET_PWD         (uint8_t)0xf7
#define IN_PWD          (uint8_t)0xf8
#define SET_ZERO_FI     (uint8_t)0xAA
#define SET_TEST_MODE   (uint8_t)0xAB


#define KEY_1 (uint8_t)0xA5
#define KEY_2 (uint8_t)0x36
#define KEY_3 (uint8_t)0x5B
//#define KEY_4 (uint8_t)0x5A
#define pref_out (uint8_t)0x3E

#define TX_BUFFER_SIZE0 180
#define RX_BUFFER_SIZE0 200

#define out_DATA (uint8_t)0x06    //Команда на выдачу данных
#define pref_in (uint8_t)0x31
#define pref_out (uint8_t)0x3E

#define out_full (uint8_t)0xF0     //Команда на выдачу всех данных
#define set_Table (uint8_t)0xF1      //Команда на устнаноку серийного номера
#define set_MODE (uint8_t)0xF2        //Команда на устнаноку частотного выхода
#define del_AVG (uint8_t)0xF3      //Команда на выключение усреднения
#define shift_HI (uint8_t)0xF4       //Команда на сдвиг верхнего значения уровня  
#define set_HI (uint8_t)0xF5       //Команда на уснановку верхнего значения уровня
#define set_SER (uint8_t)0xF6      //Команда на установку серийного номера
#define set_PWD (uint8_t)0xF7    //установить пароль
#define in_PWD  (uint8_t)0xF8    //ввести  пароль
#define shift_LO (uint8_t)0xF9       //Команда на сдвиг нижнего значения уровня
#define set_LO (uint8_t)0xFA       //Команда на установку нижнего значения уровня
#define set_NET (uint8_t)0xFB      //Команда на устнаноку сетевого номера
#define out_SER (uint8_t)0xFC      //Команда на устнаноку серийного номера

#define set_bp_on (uint8_t)0xe3    //Команда калибровки нуля.
#define set_bp_off (uint8_t)0xe5    //Команда калибровки нуля.

#define set_gps_on (uint8_t)0xe7    //Команда лога gps.
#define set_gsm_on (uint8_t)0xe8    //Команда лога gsm.
#define log_bt_on (uint8_t)0xe9    //Команда лога BT.


#define no_PWD (uint8_t)0xFD      //Неверный пароль
#define put_Table (uint8_t)0xEF      //Выдать таблицу
#define set_POROG (uint8_t)0xEE      //Команда установки порога лампочки
#define opros_net (uint8_t)0xFF    //Команда опроса сетевого номера

#define downloadPRG (uint8_t)0xDD
#define clr_bb (uint8_t)0xDF

#define net_addr_dev (uint8_t)202

void instring(unsigned char n);
char crc_str(char *str6, int lens);
char crcc(char dat_crc, char crc8);
char * zagolovok( char comand);
char *ddout(char *cr,unsigned char n,unsigned long z);
void out_d(void);
void form_cfg_to_out(void);
void form_pusk_zapros(char addr_zaprosa,char cmd);
/*TD_ONLINE END**********************************************************************************************/


#define ACCEL_NOT_WORKING				0x01
#define BLACKBOX_NOT_WORKING		0x02
#define GNSS_NOT_WORKING				0x04

#define ERROR_DEBUG_PERIOD			2000


enum{
		WIALON_IPS_PROTOCOL=0,
		SHORT_OUT,
		EGTS_PROTOCOL
};
typedef __packed struct 
{
  char        f;
  char        api[30];		   //точка доступа
  char        user_api[20];
  char        pwd_api[20];
  char        pincod[5];		//пин код симкарты		
  char        ip_serv[25];		//адрес сервера.
  char        port_serv[6];	//порт сервера	6 симв. !!! внимательнее строчка должна кончаться нулем!!!
  char        id_dev[10];		//идентификатор прибора
  char        pwd_serv[25];	//пароль
  uint16_t    dop_period;		//период отправки пакетов с доп. параметрами
  uint8_t     mode_in1;		    // режим 1 входа
  uint8_t     mode_in2;			// режим второго входа
  char        mode_485[8]; 		// режимы 485-х
  uint8_t     addr_485[8];		// адреса  485-х
  uint8_t     crc_edata;
  
	char        ftp_serv[20];
	char        ftp_user[20];
	char        ftp_pass[20];
  
	char        app_name[62];
  
	char        ntp_serv1[25];		
	char        ntp_serv2[25];	
	char        ntp_serv3[25];		
	char        ntp_serv4[25];
	uint8_t	    dut_nums[DUT_COUNT];
  uint8_t     dut_count;
	uint8_t	    updated;
	uint8_t	    restarted;
	/*
	 * 1 manual restart
	 * 2 gsm.restart
	 * 3 ping_timeout
	 * 4 network_timeout
	 * 5 update app
	 * */
  uint32_t  ser_num;
	uint32_t	porog_run_engine;
	uint32_t	porog_stop_engine;
	uint8_t		server_protocol;
  uint8_t   crc;
} eData; 

typedef  struct 
{
  uint8_t           reset_kp;
  uint16_t          pars_index;
	uint8_t						debug_out;
	uint8_t						error;
	char 	        		imei[16];
	char 	        		imsi[25];
	uint16_t					error_timer;
} CONFIG_STRUC; 


extern eData kp;
extern CONFIG_STRUC config;

void Reset_kp(void);
void Init_kp(void);
void Configurator_Serv(void);
void Write_kalibr_data(void);
void Error_Debug(void);
void Restart_After_Pause(void);
void Reset_Counter_Handler(void);


