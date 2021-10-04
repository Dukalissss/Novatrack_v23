#include "main.h"

POINT_MAKE_STRUC point_make;

char zz1=0;
int skorrr=0;
float mytemp, porog;
uint16_t pm_count=0;

void Point_Make_Serv(void)
{
  if (!point_make.rst)  Point_Make_Reset();
  if (!calendar.actual) return;
	if (!LL_TIM_IsEnabledCounter(POINT_MAKE_TIMER))
		LL_TIM_EnableCounter(POINT_MAKE_TIMER);
  point_make.processing_timer+=t1_ms;
  if (point_make.processing_timer<PROCESSING_PERIOD) return;
	point_make.timer+=point_make.processing_timer;
  point_make.processing_timer=0;
  Point_Make_Processing();
}
void Point_Make_Reset(void)
{
  point_make.rst=1;
  point_make.timer=0;
  point_make.timer_period=1000*kp.dop_period;
  if (point_make.timer_period<MIN_PERIOD) point_make.timer_period=MIN_PERIOD;
  point_make.first_koord=1;
  point_make.first_msg=1;
  blackbox.write=0;
	sprintf(point_make.fix_koord, "NA;NA;NA;NA");
	
	LL_TIM_SetAutoReload(POINT_MAKE_TIMER, point_make.timer_period/POINT_MAKE_TIMER_RESOLUTION);
	LL_TIM_EnableIT_UPDATE(POINT_MAKE_TIMER);
}
void Point_Make_Processing(void)
{ 		
  if (point_make.first_msg)
  {
    blackbox.write=1; 
    point_make.first_msg=0;
    point_make.point_make_src|=FIRST_MSG;
  }
  if (in_kontakt.changed)
  {
    blackbox.write=1;
    in_kontakt.changed=0;
    point_make.point_make_src|=IN_KONTAKT_CHANGED;
  }
  if (ibutton.changed)
  {
    blackbox.write=1;
    ibutton.changed=0;
    point_make.point_make_src|=iBUTTON_CHANGED;
  }
	if (gnss.status=='A')
	{
				
			mytemp=atof(gnss.speed);
			
			point_make.sr_vel=point_make.sr_vel+(mytemp-point_make.sr_vel)/USRED_KOORD;
			skorrr=(int)point_make.sr_vel;
			if (accel.work) {mytemp=20.0; } else {mytemp=2.0;}
			if((point_make.sr_vel>mytemp) || (accel.move) || (point_make.first_koord))
			{ 
				point_make.moving=1;
				if (point_make.first_koord)
				{
					point_make.first_koord=0;
					blackbox.write=1; 
					point_make.point_make_src|=FIRST_KOORDINATES;
					sprintf(point_make.fix_koord, "%s", gnss.fix_koord);
				}

				mytemp=atof(gnss.course); 
				
				if(fabs(point_make.sr_ang-mytemp)>180) 
				{ 
					if(point_make.sr_ang>180) {mytemp=360-mytemp;}
					else { mytemp=mytemp-360; };
				};
				point_make.sr_ang+=(mytemp-point_make.sr_ang)/USRED_KOORD;
				
				porog= 280/(point_make.sr_vel+12); 
				if(fabs(point_make.sr_ang-point_make.fix_ang)>porog) 
				{ 
					blackbox.write=1; 
					sprintf(point_make.fix_koord, "%s", gnss.fix_koord);
          point_make.point_make_src|=COURSE_CHANGED;
          point_make.fix_ang=point_make.sr_ang; 
				}
				point_make.timer_period=1000*kp.dop_period;
				LL_TIM_SetAutoReload(POINT_MAKE_TIMER, point_make.timer_period/POINT_MAKE_TIMER_RESOLUTION);				
			} else 
			{	
				point_make.moving=0;
				if (point_make.timer_period!=1000*kp.dop_period*8)
				{
					point_make.timer_period=1000*kp.dop_period*8;
					LL_TIM_SetAutoReload(POINT_MAKE_TIMER, point_make.timer_period/POINT_MAKE_TIMER_RESOLUTION);
				}
			}
	}
  if ((blackbox.write)&&(!(point_make.point_make_src&TIMER_UPDATE)))
  {
		pm_count=POINT_MAKE_TIMER->CNT;
    if (pm_count<500)
    {	
      blackbox.write=0;
      POINT_MAKE_TIMER->CNT=POINT_MAKE_TIMER->ARR-500+pm_count;
      
    } 
    else
      POINT_MAKE_TIMER->CNT=10;
  }
     
        
}
void Point_Make_Handler(void)
{
	if (LL_TIM_IsActiveFlag_UPDATE(POINT_MAKE_TIMER))
	{
    blackbox.write=1;
		if (point_make.moving)
		{
			pm_count=0;
			while ((gnss.fix_koord[pm_count])&&(pm_count<50))
			{
				point_make.fix_koord[pm_count]=gnss.fix_koord[pm_count];
				pm_count++;
			}
			point_make.fix_koord[pm_count]=0;
		}
    point_make.point_make_src|=TIMER_UPDATE;	
		point_make.timer=0;
	}
	POINT_MAKE_TIMER->SR=0;
}


