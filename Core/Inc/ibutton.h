#define iBUTTON_TIMER                 TIM3
#define iBUTTON_RX_LINE               LL_EXTI_LINE_6
#define ONE_WIRE_TX_PULL_DOWN()       LL_GPIO_SetOutputPin(ONE_WIRE_TX_GPIO_Port, ONE_WIRE_TX_Pin)
#define ONE_WIRE_TX_PULL_UP()         LL_GPIO_ResetOutputPin(ONE_WIRE_TX_GPIO_Port, ONE_WIRE_TX_Pin)

#define iBUTTON_PERIOD                1000
#define iBUTTON_DURATION              100

typedef struct
{
  uint8_t     cmd;
  uint8_t     rx;
  uint8_t     rx_changed;
  uint8_t     tx_byte;
  uint8_t     tx_cnt;
  uint8_t     rx_byte[8];
  uint8_t     rx_byte_cnt;
  uint8_t     rx_bite_cnt;
  uint16_t    ms_timer;
  uint16_t    led_timer;
  uint8_t     processing;
  uint64_t    key;
  uint8_t     changed;
} IBUTTON_STRUC;

enum
{
/*0*/   NOT_RUNNING=0,
/*1*/   PRESENCE_BEGIN,
/*2*/   PRESENCE_END,
/*3*/   READ_ROM,
/*4*/   TX_WAIT,
/*5*/   TX_RESET,
/*6*/   READ_INIT,
/*7*/   READ,
/*8*/   READING,
/*9*/   READ_END,


  END,
  NOPE
};



extern IBUTTON_STRUC ibutton;

void iButton_Serv(void);
void iButton_Timer_Handler(void);
void iButton_EXTI_Handler(void);
void Start_Reading(void);
void Stop_Reading(void);
uint8_t iButtonCRC(void);



