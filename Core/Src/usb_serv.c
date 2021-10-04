/**********************************************************************
В файле usbd_desc.c в функции Get_SerialNum заменить:
  deviceserial0 = 0x12345678;
  deviceserial1 = 0x12324678;
  deviceserial2 = 0x65445678;
	
	В файле usbd_cdc_if.c в функцию CDC_Receive_FS добавить строку:
  USB_Serv_Receive(Buf, *Len);
**********************************************************************/

#include "main.h"
#include "usbd_cdc_if.h"

USB_SERV_STRUC usb_serv={0};


void USB_Serv(void)
{
}
void pusk_usb(uint8_t *buf, uint16_t len)
{
	uint8_t ret=USBD_BUSY;
	while (ret!=USBD_OK)
		ret=CDC_Transmit_FS(buf, len);
}
void USB_Serv_Receive(uint8_t *buf, uint16_t len)
{
  uint32_t k=0;
  while (k<len)
  {
    usb_serv.buf_in[usb_serv.rx_index]=buf[k];
    k++;
    if (++usb_serv.rx_index>=USB_BUF_SIZE) usb_serv.rx_index=0;
  }
}




