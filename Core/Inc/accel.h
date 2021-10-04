extern  SPI_HandleTypeDef     hspi2;
#define SPI_ACCEL             hspi2
#define ACCEL_BUF_SIZE        20
#define ACCEL_CS_RESET()      LL_GPIO_ResetOutputPin(CS_ACCEL_GPIO_Port, CS_ACCEL_Pin)
#define ACCEL_CS_SET()        LL_GPIO_SetOutputPin(CS_ACCEL_GPIO_Port, CS_ACCEL_Pin)
#define ACCEL_PERIOD          20
#define med_filter_order		  7
#define ACCEL_TIMEOUT         100
#define ACCEL_TIMEOUT_NUM			10


typedef struct
{
  uint8_t       work;
  uint8_t       move;
	uint8_t 			buf_in[ACCEL_BUF_SIZE];
  uint8_t       buf_out[ACCEL_BUF_SIZE];
  uint8_t       tx_complete;
  uint16_t      period;
  short         Gx;
  short         Gy;
  short         Gz;
  short         sr_med_mass[3][med_filter_order];
	float 	      mxd,myd,mzd;
	float         mxl,myl,mzl;
	long          tx,ty,tz;
	short 		    med_filt;
	long 		      disp1;
	long          quality_drive;
  uint16_t      timeout;
	uint8_t				timeout_counter;
	uint8_t				not_avaliable;
} ACCEL_SPI_STRUC;


extern volatile ACCEL_SPI_STRUC accel;
void Accel_config(void);
void Accel_Spi_Run(uint8_t rxtx_len);
void Accel_serv(void);
void Get_Accel_Data(void);
void Med_Filter(void);
void Copy_mass(short *X, short *Y, short n);
void Circshift(short *X, short n);
void Puzir(short *X, short n);
void calc_move(void);
void Accel_TimeOut(void);
