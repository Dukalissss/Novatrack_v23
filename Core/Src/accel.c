#include "main.h"

volatile  ACCEL_SPI_STRUC accel;
static    uint8_t accel_rst=1;
short     med_mass[med_filter_order];
static    uint8_t cnt_aw=0;
static    uint8_t f_res=10;


void Accel_serv(void)
{
	if (accel.not_avaliable) 
	{
		config.error|=ACCEL_NOT_WORKING;
		return;
	}
  if (accel_rst)
  {
    accel_rst=0;
    Accel_config();
  } 
  accel.period+=t1_ms;
  if (accel.period>=ACCEL_PERIOD)
  {
    accel.period=0;
    Get_Accel_Data();
    Med_Filter();
    calc_move();
  }
}
void Accel_TimeOut(void)
{
  if (accel.timeout) accel.timeout--;
}
void Get_Accel_Data(void)
{
  short g=0;
  
  accel.buf_out[0]=0xA7;  
  accel.buf_out[1]=0x00;
  accel.buf_in[0]=0x00;
  accel.buf_in[1]=0x00;
  Accel_Spi_Run(2);
  
  accel.buf_out[0]=0xE8;  
  accel.buf_in[0]=0x00;
  for (uint8_t cc=1; cc<7; cc++)
  {
    accel.buf_out[cc]=0x00;  
    accel.buf_in[cc]=0x00;
  }
  Accel_Spi_Run(8);
  
  g=accel.buf_in[2];
	g=g<<8;
	g=g | accel.buf_in[1];
	accel.Gx=g;

	g=accel.buf_in[4];
	g=g<<8;
	g=g | accel.buf_in[3];
	accel.Gy=g;


	g=accel.buf_in[6];
	g=g<<8;
	g=g | accel.buf_in[5];
	accel.Gz=g;
}
void Accel_config(void)
{
  ACCEL_CS_SET();
  accel.buf_out[0]=0x24;
  accel.buf_out[1]=0x80;
  Accel_Spi_Run(2);
  
  accel.buf_out[0]=0x1F;
  accel.buf_out[1]=0x40;
  accel.buf_out[2]=0x00;
  accel.buf_out[3]=0x00;
  accel.buf_in[0]=0x00;
  accel.buf_in[1]=0x00;
  accel.buf_in[2]=0x00;
  accel.buf_in[3]=0x00;
  Accel_Spi_Run(2);
    

  accel.buf_out[0]=0x23;
  accel.buf_out[1]=0x88;
  Accel_Spi_Run(2);
  
  accel.buf_out[0]=0x20;
  accel.buf_out[1]=0x27;//27 - 10Hz, 47 - 50 Hz
  Accel_Spi_Run(2); 

//  accel.buf_out[0]=0x21;
//  accel.buf_out[1]=0x01;
//  Accel_Spi_Run(2);

//  accel.buf_out[0]=0x22;
//  accel.buf_out[1]=0x40;
//  Accel_Spi_Run(2);

  accel.buf_out[0]=0x24;
  accel.buf_out[1]=0x00;
  Accel_Spi_Run(2);
  
//  accel.buf_out[0]=0x32;
//  accel.buf_out[1]=0x08;
//  Accel_Spi_Run(2);
  
//  accel.buf_out[0]=0x33;
//  accel.buf_out[1]=0x08;
//  Accel_Spi_Run(2);
  
//  accel.buf_out[0]=0x30;
//  accel.buf_out[1]=0x84;
//  Accel_Spi_Run(2);
  
  accel.buf_out[0]=0x8F;
  accel.buf_out[1]=0x00;
  accel.buf_out[2]=0x00;
  accel.buf_out[3]=0x00;
  accel.buf_in[0]=0x00;
  accel.buf_in[1]=0x00;
  accel.buf_in[2]=0x00;
  accel.buf_in[3]=0x00;
  Accel_Spi_Run(2);
  if (accel.buf_in[1]!=0x33) accel.not_avaliable=2;
}
void Accel_Spi_Run(uint8_t rxtx_len)
{
  ACCEL_CS_RESET();  
  for (uint8_t zad=0; zad<200; zad++) {};
  accel.tx_complete=0;
  accel.timeout=ACCEL_TIMEOUT;
  HAL_SPI_TransmitReceive_IT(&SPI_ACCEL, (uint8_t *)&accel.buf_out[0], (uint8_t *)&accel.buf_in[0], rxtx_len);
  while ((!accel.tx_complete)&&(accel.timeout)) {};
	if (!accel.timeout) 
		accel.timeout_counter++;
	else 
		accel.timeout_counter=0;
	if (accel.timeout_counter>ACCEL_TIMEOUT_NUM) accel.not_avaliable=1;
  ACCEL_CS_SET();
  for (uint8_t zad=0; zad<200; zad++) {};
}
void calc_move(void)
{

	if(f_res)
	{
		accel.mxl=(float)accel.Gx; accel.mxd=(float)accel.Gx;
		accel.myl=(float)accel.Gy; accel.myd=(float)accel.Gy;
		accel.mzl=(float)accel.Gz; accel.mzd=(float)accel.Gz;
		f_res--;
	}
	accel.mxl +=((float)accel.Gx-accel.mxl)/20;
	accel.myl +=((float)accel.Gy-accel.myl)/20;
	accel.mzl +=((float)accel.Gz-accel.mzl)/20;

	accel.mxd +=(accel.mxl-accel.mxd)/1000;
	accel.myd +=(accel.myl-accel.myd)/1000;
	accel.mzd +=(accel.mzl-accel.mzd)/1000;

	accel.tx=(long)(accel.mxl-accel.mxd);
	accel.ty=(long)(accel.myl-accel.myd);
	accel.tz=(long)(accel.mzl-accel.mzd);

	accel.disp1 = accel.tx*accel.tx;
	accel.disp1 += accel.ty*accel.ty;
	accel.disp1 += accel.tz*accel.tz; 

	if(accel.disp1>30)
	{
		accel.work=1;
		if(++cnt_aw>20) cnt_aw=20;
	}
	else
	{
		if(--cnt_aw<1)
		{
			accel.work=0;
			cnt_aw=1;
		}
	}

	if(accel.disp1 > 100000)
	{ 
		if (accel.med_filt<4950) {accel.med_filt=4950;} else { accel.med_filt++;};
	}
	else { if(accel.med_filt>0) accel.med_filt-=2; } ;

	if(accel.med_filt>5000) accel.med_filt=5000;

	if(accel.med_filt>=4995)
	{
		accel.move=1;
	}
	if(accel.med_filt<0) accel.med_filt=0;
	if(accel.med_filt<=3)
	{
		accel.move=0;
	}
  accel.disp1 /=4000000;
	if (accel.disp1>15) accel.disp1=15;
	if(accel.disp1>accel.quality_drive) accel.quality_drive = accel.disp1;
  if (!accel.move) accel.quality_drive = accel.disp1;
}
void Puzir(short *X, short n)
{
	short pr=0;
	uint8_t count=0, count2=0;
	for(count = 0 ; count < n - 1; count++) {
				 for(count2 = 0 ; count2 < n - count - 1 ; count2++) {
						 if(X[count2] > X[count2+1]) {
								pr = X[count2];
								X[count2] = X[count2+1] ;
								X[count2+1] = pr;
						 }
					}
			}
}
void Circshift(short *X, short n)
{
    short temp = X[--n];
    while ( n > 0 ) X[n--] = X[n-1];
}
void Copy_mass(short *X, short *Y, short n)
{
	uint8_t cc=0;
	for (cc=0; cc<n; cc++)
	 Y[cc] = X[cc];
}
void Med_Filter(void)
{
	Copy_mass((short *)accel.sr_med_mass[0], med_mass, med_filter_order);
	Circshift(med_mass, med_filter_order);
	med_mass[0]=accel.Gx;
	Copy_mass(med_mass, (short *)accel.sr_med_mass[0], med_filter_order);
	Puzir(med_mass, med_filter_order);
	if (med_mass[(med_filter_order-1)/2])
		accel.Gx=med_mass[(med_filter_order-1)/2];

	Copy_mass((short *)accel.sr_med_mass[1], med_mass, med_filter_order);
	Circshift(med_mass, med_filter_order);
	med_mass[0]=accel.Gy;
	Copy_mass(med_mass, (short *)accel.sr_med_mass[1], med_filter_order);
	Puzir(med_mass, med_filter_order);
	if (med_mass[(med_filter_order-1)/2])
		accel.Gy=med_mass[(med_filter_order-1)/2];

	Copy_mass((short *)accel.sr_med_mass[2], med_mass, med_filter_order);
	Circshift(med_mass, med_filter_order);
	med_mass[0]=accel.Gz;
	Copy_mass(med_mass, (short *)accel.sr_med_mass[2], med_filter_order);
	Puzir(med_mass, med_filter_order);
	if (med_mass[(med_filter_order-1)/2])
		accel.Gz=med_mass[(med_filter_order-1)/2];
}
