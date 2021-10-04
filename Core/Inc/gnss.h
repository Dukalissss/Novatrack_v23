#define USART_GNSS                  USART1
#define GNSS_RX_BUF_SIZE            1000
#define GNSS_RX_BUF_SIZE2           1000
#define GNSS_TX_BUF_SIZE            100
#define DMA_GNSS                    DMA1
#define DMA_GNSS_RX                 LL_DMA_CHANNEL_5

#define DMA_GNSS_RX_COMPLETE()      LL_DMA_IsActiveFlag_TC5(DMA_GNSS)
#define DMA_GNSS_RX_CLEAR_FLAG()    LL_DMA_ClearFlag_TC5(DMA_GNSS)
#define DMA_GNSS_RX_ERROR()         LL_DMA_IsActiveFlag_TE5(DMA_GNSS)
#define DMA_GNSS_RX_ERROR_CLEAR()   LL_DMA_ClearFlag_TE5(DMA_GNSS)
#define DMA_GNSS_GI_FLAG_CLEAR()    LL_DMA_ClearFlag_GI5(DMA_GNSS)

#define SNR_MASS_SIZE               5
#define FIX_COORD_SIZE              50
#define PARS_PERIOD                 200


#define size_buf_gga_rmc_str        90
#define RMC_STR                     0x02
#define GGA_STR                     0x04
#define GSV_STR                     0x08
#define GPS_STR_READY               0x01

#define FLASH_SAVE_PERIOD						120000


#define HOT_START										0x01
#define WARM_START									0x02
#define COLD_START									0x03

#define WARM_START_TIMEOUT					5000
#define COLD_START_TIMEOUT					25000

typedef struct
{
  uint8_t       ready;
	uint8_t 			buf_in[GNSS_RX_BUF_SIZE];
  uint8_t       buf_in2[GNSS_RX_BUF_SIZE2];
  uint8_t       buf_out[GNSS_TX_BUF_SIZE];
  uint16_t      ptr_out_tx2;
  uint16_t      len_tx;
  uint16_t      rx_index;
  uint16_t      rd_index;
  uint16_t      pars_index;
  uint8_t		 		*tx; 
  uint8_t       on;
	uint8_t				state;
	uint8_t				data_avaliable;
} GNSS_USART_STRUC;

typedef struct
{
  uint16_t      cntr_buf_nmea;
  uint8_t       cnt_out;
  uint8_t       ogr_str_gps;
  uint8_t       buf_gga[size_buf_gga_rmc_str];
  uint8_t       buf_rmc[size_buf_gga_rmc_str];
  uint8_t       buf_gsv1[size_buf_gga_rmc_str];
  uint8_t       fl_gps_str;
  uint8_t       first_koord;
  char          fix_koord[50];
  uint8_t       sats;
  uint8_t       snr;
  char          speed[10];
  char          course[10];
  char          hdop[6];
  char          altitude[10];
  char          time[10];
  char          date[10];
  char          status;
  uint8_t       sats_mass[SNR_MASS_SIZE];
  uint8_t       snr_mass[SNR_MASS_SIZE];
  uint16_t      pars_timer;
	uint8_t				rmc_debug;
	uint32_t			saving_timer;
	uint16_t			cold_start_timer;
	uint8_t				start_type;
} GNSS_PARS_STRUC;


enum{
	PERIPH_PREPARE=0,
	SET_HOT_START,
	GLL_OFF,
	GSA_OFF,
	VTG_OFF,
	ZDA_OFF,
	DTM_OFF,
	RLM_OFF,
	GNSS_RATE,
	GNSS_MIX,
	GNSS_OK
};
extern GNSS_USART_STRUC gnss_usart;
extern GNSS_PARS_STRUC   gnss;

void GNSS_On(void);
void USART_GNSS_Handler(void);
void Send_Cmd_GNSS(uint8_t *str_gnss, uint16_t lenn);
void GNSS_Serv(void);
void GNSS_Parsing(void);
void Get_Position_Data(void);
int my_str_cmp(char *str1, uint16_t length1, char *str2, uint16_t length2);
unsigned int like_num(char *instr, char *cmp_str, char num);
int cpy_bet_num(char *dst_str, char *src_str, int n1,int n2);
uint8_t good_koord(void);
void GNSS_Data_Clear(void);
void GNSS_Save_FL(void);
void Cold_Start(void);
void Warm_Start(void);



