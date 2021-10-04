#define DUT_COUNT               10
#define ANSWER_WAITING_TIMEOUT  (uint16_t)200
#define DUT_TIMEOUT             10
#define LLS_PROCESSING_PERIOD   100
typedef struct
{
  int8_t          temp;
  uint16_t         level;
  uint16_t         freq;
  uint16_t        timeout;
} DUT_STRUC;


typedef struct
{
  uint8_t         cmd;
  uint8_t         state;
  uint8_t         dut_num;
  uint8_t         crc_in;
  uint8_t         crc_out;
  uint16_t        pars_index;
  uint16_t        cmd_timeout;
  uint16_t        processing_timer;
} LLS_STRUC;

enum
{
  NET_POLLING=0,
  ANSWER_WAITING
};


extern DUT_STRUC dut[DUT_COUNT];
extern LLS_STRUC lls;



void LLS_Serv(void);
void Net_Polling(void);
uint8_t LLS_CRC8(uint8_t data, uint8_t crc);
void LLS_Parsing(uint8_t b);


