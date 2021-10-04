#define USART_GSM                   USART2
#define DMA_GSM                     DMA1
#define DMA_GSM_RX                  LL_DMA_CHANNEL_6
#define DMA_GSM_RX_CLEAR_FLAG()     LL_DMA_ClearFlag_TC6(DMA_GSM)
#define DMA_GSM_RX_ERROR_CLEAR()    LL_DMA_ClearFlag_TE6(DMA_GSM)
#define DMA_GSM_GI_FLAG_CLEAR()     LL_DMA_ClearFlag_GI6(DMA_GSM)
#define GSM_RX_BUF_SIZE             100
#define GSM_RX_BUF_SIZE2            100
#define GSM_TX_BUF_SIZE             100
#define ANSWER_BUF_SIZE             GSM_RX_BUF_SIZE2
#define WAIT_ANS_LEN                50
#define MSG_LENGTH                  500


#define GSM_STATE_MACHINE_PERIOD    200
#define GSM_PWR_ON()                LL_GPIO_ResetOutputPin(GSM_PWR_ON_GPIO_Port, GSM_PWR_ON_Pin)
#define GSM_PWR_OFF()               LL_GPIO_SetOutputPin(GSM_PWR_ON_GPIO_Port, GSM_PWR_ON_Pin)
#define GSM_PWRKEY_OFF()            LL_GPIO_SetOutputPin(GSM_PWRKEY_GPIO_Port, GSM_PWRKEY_Pin)
#define GSM_PWRKEY_ON()             LL_GPIO_ResetOutputPin(GSM_PWRKEY_GPIO_Port, GSM_PWRKEY_Pin)

#define PING_TIMEOUT			          10000
#define NETWORK_TIMEOUT			        70000
#define SHORT_CMD_TIMEOUT		        60000
#define LONG_CMD_TIMEOUT		        120000
#define NO_SENDING_DATA_TIMEOUT			600000

typedef struct
{
  uint8_t       ready;
	uint8_t 			buf_in[GSM_RX_BUF_SIZE];
	uint8_t 			buf_in2[GSM_RX_BUF_SIZE2];
  uint8_t       buf_out[GSM_TX_BUF_SIZE];
  uint16_t      ptr_out_tx2;
  uint16_t      len_tx;
  uint16_t      rx_index;
  uint16_t      rd_index;
  uint16_t      pars_index;
  uint8_t		 		*tx; 
} GSM_USART_STRUC;
typedef struct
{
  uint8_t       reboot;
  uint8_t       on;
  uint16_t      state;
  uint8_t       cmd;
  uint8_t       net_connected;
  uint8_t       rtc_en;
	uint8_t 		  tcp_connected;
  uint8_t       log_in;
  uint8_t       state_machine_period;
  uint16_t      delay;
  uint16_t      timer;
  uint8_t       ntp_num;
  uint8_t       wait_ans[WAIT_ANS_LEN];
  uint8_t       answer[GSM_RX_BUF_SIZE2];
  uint8_t       answer_index;
  uint8_t       rst;
  uint8_t       rst_timeout;
  uint16_t      TimeOutGsm;
  uint8_t       TimeOutCounter;
  uint8_t       not_flg;
	char 	        imei[16];
	char 	        imsi[25];
  char          msg[MSG_LENGTH];
  uint16_t      send_length;
  uint8_t       pusk;
  uint8_t       ASD;
  uint32_t      asd_timeout;
  uint8_t       ftp_connected;
  char          phone_number[15];
  uint8_t       sms_rec;
  uint8_t       sms_unread;
	char	        sms_ans[100];
  uint8_t       send_sms;
  uint16_t      state_num;
	uint8_t				restart;
	uint32_t			sending_timeout;
} GSM_STRUC;
enum
{
	NOP=0,		
	NET_CONNECT, 		
	NTP_CONNECT,
	TCP_CONNECT,
	SEND_LOGIN,		
	READ_SMS,	
	SEND_DATA,	
	SEND_SMS,	
	CHECK_TCP,	
	FTP_CONNECT,//8
	AGPS,		
	GET_LOC,		
	CHCK_EPO,	
	DWNLD_EPO,//13	
	NO_CMD
};
enum
{
  reset1=0,   //0
  reset2,//1
  reset3,//2
  reset4,//3
	Patz,//4
	Pate, //5
	Preg, //6
	Pcmee,//7
	Papn, //8
	PgprsUp,//9
	Pip,//10
	Pimei,//11
  Pimsi,//12
	PSAPBR,//13
	PSAPBR2,//14
	PSAPBR3,//15
	PSAPBR4,//16
	NET_CONNECTED_OK
};
enum
{
	CNTPCID=0,//17
	CNTP,//18
	GetCNTP,//19
	GetCCLK,//20
	SetCCLK,//21
	Save_changes,//22
	NTP_connected,//
  NTP_fail
};
enum
{	
	Pcmgf=0,//23
	Pcmgl,//24
	Pdelsms,//25
	Read_sms_ok//
};
enum{	
	Pcmgs=0,//26
	Psms,//27
	Send_sms_ok//
};
enum modem_step 
{ 
	Ptcp=0,//28
  ip_addr,//29
	Tcp_ok //
};
enum
{	
	Psend_l=0,//30
	Psend_data_l,//31
	Data_l_send_ok//
};
enum
{	
	Psend=0,//32
	Psend_data,//33
	Data_send_ok//34
};
enum
{
	ftpcid=0,//34
	ftpserv,//35
	ftpun,//36
	ftppw,//37
	ftpgetname,//38
	ftpgetpath,//39
  fs_local_drive,//40
//  fs_check_file,//41
//  fs_del_file,//42
  fs_free_size,//43
	ftpsize,//44
	ftpget,//45
	ftpquit//46
};
extern GSM_USART_STRUC gsm_usart;
extern GSM_STRUC       gsm;

void GSM_UART_Handler(void);
void Send_Cmd_GSM(uint8_t *str_gsm, uint16_t lenn);
void GSM_On(void);
void GSM_Serv(void);
void Net_Connect(void);
void State_Machine_Delay(void);
void GSM_ms_timer(void);
void Receive_Modem_Data(void);
void parse_mdm_data(void);
void Wait_Ans_Clear(void);
void Ntp_connect(void);
void Get_Time_NTP(void);
void GetIMEI(void);
void Tcp_connect(void);
void Send_login(void);
void Send_data_to_server(void);
void Ftp_Connect(void);
void Get_Drive(void);
void Get_Available_Size(void);
void File_Exists(void);
void Get_File_Size(void);
void CheckReceivedData(void);
void Serv_Cmd_Check(void);
void Read_SMS(void);
void set_kal_param(char *src, char *dst, uint16_t len);
void sms_proc(void);
void Send_SMS(void);
void Get_IMSI(void);


