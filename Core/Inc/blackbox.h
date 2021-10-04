extern  SPI_HandleTypeDef     hspi1;
#define SPI_MEM               hspi1
#define MEM_BUF_SIZE          20
#define MEM_CS_RESET()        LL_GPIO_ResetOutputPin(CS_MEM_GPIO_Port, CS_MEM_Pin)
#define MEM_CS_SET()          LL_GPIO_SetOutputPin(CS_MEM_GPIO_Port, CS_MEM_Pin)

#define SENDED                (uint8_t)0x5A
#define NOT_SENDED            (uint8_t)0xD2
#define LEN_PACK              128
#define BB_SIZE_BYTES         (uint32_t)0x00800000
#define BB_SIZE               (uint32_t)65536
#define BLACKBOX_PERIOD       175
#define BLACKBOX_TIMEOUT      100

#define BLACKBOX_TIMEOUT_NUM	10

#define BLACKBOX_NOT_AVALIABLE	0x01
#define SEND_MSG_ERROR					0x02
#define BB_CRC_ERROR						0x04
#define MARKER_ERROR						0x08


typedef struct
{
	uint8_t 			  buf_in[MEM_BUF_SIZE];
  uint8_t         buf_out[MEM_BUF_SIZE];
  uint8_t         tx_complete;
  uint8_t         status;
  char            time[10];
  char            date[10];
  char            longitude[13];
  char            lon[3];
  char            latitude[13];
  char            lat[3];
  char            fix_koord[FIX_COORD_SIZE];
  char            speed[10];
  char            course[10];
  char            altitude[10];
  char            sats[5];
  char            hdop[6];
  char						gnss_status;
  uint8_t         SNR;
	uint16_t 			  Adc;
	uint16_t 			  pwr;
	int16_t			 	  temperature;
	uint16_t				qdrive;
  
  
  uint64_t        ibutton_key;
  uint8_t         sk1;
  uint8_t         sk2;
  uint16_t        ain1;
  uint16_t        ain2;
  uint16_t        freq1;
  uint16_t        freq2;
  uint8_t         mode1;
  uint8_t         mode2;

  int8_t          temp[DUT_COUNT];
  uint16_t        level[DUT_COUNT];  
  uint16_t        freq[DUT_COUNT];
  
  
  
  uint8_t         write;
  uint32_t        writed_lines;
  uint32_t        read_lines;
  uint16_t        timer;
  uint8_t         fail_count;
  uint8_t         point_make_src;
  uint16_t        timeout;
	uint8_t					timeout_counter;
	uint8_t					error;
	uint8_t					find_pos_state;
} MEM_SPI_STRUC;

enum{
	begin_find_position=0,
	finding1,
	finding2,
	finding3,
	finding_end
};

typedef __packed struct {
	uint8_t 			  marker;
	uint32_t 			  time;  
	uint32_t 			  date;
	uint32_t 			  lon;
	uint32_t 			  shirota;
	uint16_t 			  Hight;
	uint8_t 				Vel;
	uint8_t 				Asimut;
	uint8_t 				Hdop;
	uint8_t 				Sputn;
  uint8_t         SNR;
	uint16_t 			  Adc;
	uint16_t 			  pwr;
	int16_t			 	  temperature;
	uint16_t				qdrive;
  
  
  uint64_t        ibutton_key;
  uint8_t         sk1;
  uint8_t         sk2;
  uint16_t        ain1;
  uint16_t        ain2;
  uint16_t        freq1;
  uint16_t        freq2;
  uint8_t         mode1;
  uint8_t         mode2;

  int8_t          temp[DUT_COUNT];
  uint16_t        level[DUT_COUNT];
  uint16_t        freq[DUT_COUNT];
  uint8_t         point_make_src;
	uint8_t 				crc;
} BB_STRUC;


extern volatile MEM_SPI_STRUC  blackbox;


void BlackBox_Serv(void);
void MEM_Spi_Run(uint8_t rxtx_len);
void CheckStatus(void);
void EraseMemory(void);
void EraseSector(uint32_t addr);
void Pack_Struct(void);
uint8_t Unpack_Struc(void);
void Write_Enable(void);
void Write_Disable(void);
void SPI_ReadBuff(uint32_t bb_addr, uint8_t* data, uint8_t len);
void SPI_WriteBuff(uint32_t bb_addr, uint8_t* data, uint8_t len);
void WriteMarker(uint16_t str_num);
void ReadMarker(uint16_t str_num);
void WriteBlackBox(uint16_t str_num);
void ReadBlackBox(uint16_t str_num);
void FindPosition(void);
void BlackBox_TimeOut(void);

