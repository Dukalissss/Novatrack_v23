#define USB_BUF_SIZE            700
																

typedef  struct 
{
  uint8_t           buf_in[USB_BUF_SIZE];
//  uint8_t           buf_out[USB_BUF_SIZE];
  uint16_t          rx_index;
  uint16_t          tx_index;
  uint16_t          send_index;
} USB_SERV_STRUC;

extern USB_SERV_STRUC usb_serv;


void pusk_usb(uint8_t *buf, uint16_t len);
void USB_Serv(void);
void USB_Serv_Receive(uint8_t *buf, uint16_t len);




