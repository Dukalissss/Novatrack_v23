#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "stdint.h"
#include <math.h>
#include "usb_serv.h"
#include "rs485_lib.h"
#include "gnss.h"
#include "gsm.h"
#include "accel.h"
#include "lls.h"
#include "calendar.h"
#include "wialon_ips_serv.h"
#include "point_make.h"
#include "in_kontakt.h"
#include "ibutton.h"
#include "blackbox.h"
#include "mem.h"
#include "config.h"
#include "fota.h"
#include "egts_serv.h"


extern volatile uint8_t t1_ms;
extern volatile uint16_t delay_ms;

#define NTP_NUM         4

#define VERSION         23

void my_main(void);



