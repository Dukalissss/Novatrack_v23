#include "main.h"

uint32_t flAddress = 0;
uint32_t data32 = 0;

void Wait(void)
{
  uint32_t tickstart = 0;                 
  while(FLASH->SR & (0x1UL << (0U))) 
  { 
    if (++tickstart>=0xEFFFFFFF) return;
  }
  if (FLASH->SR & (0x1UL << (5U)))
  {
    FLASH->SR=(0x1UL << (5U));
  }                          
  if ((FLASH->SR & (0x1UL << (4U))) ||                                
    (FLASH->SR & (0x1UL << (2U))))  
  {
    return;
  }
}
void write_variables (char *wrstr,int lens, uint32_t address)
{
  uint8_t index = 0U;
  uint8_t nbiterations = 0U;
	uint64_t 	DATA_64=0;
	uint16_t *t;
	uint16_t	data_count=0;
	t=(uint16_t *)wrstr;
  if(FLASH->CR & (0x1UL << (7U)))
  {         
    FLASH->KEYR=(0x45670123UL << (0U));
    FLASH->KEYR=(0xCDEF89ABUL << (0U));
    if(FLASH->CR & (0x1UL << (7U)))
    {
      return;
    }
  }
//  uint32_t addresss = 0U;
 Wait();
//        addresss = address;
        FLASH->CR|=(0x1UL << (1U));
        FLASH->AR=address;               
        FLASH->CR|=(0x1UL << (6U));
          Wait();
          FLASH->CR&= ~ (0x1UL << (1U));
	flAddress = address;
	while (flAddress < address+lens)
	{
		DATA_64=(uint64_t)(t[data_count++]);
		DATA_64|=(uint64_t)(t[data_count++]) << 16;
		DATA_64|=(uint64_t)(t[data_count++]) << 32;
		DATA_64|=(uint64_t)(t[data_count++]) << 48;
 Wait();
      nbiterations = 4U;
    for (index = 0U; index < nbiterations; index++)
    {
      FLASH->CR |=  (0x1UL << (0U));    
    *(__IO uint16_t*)(flAddress + (2U*index)) = (uint16_t)(DATA_64 >> (16U*index));
         Wait();                 
        FLASH->CR &= ~(0x1UL << (0U));
    }
    flAddress = flAddress + 8;
	}                
	FLASH->CR|=(0x1UL << (7U));
}
void load_variables (char *str2,long lens,uint32_t address)
{
	uint16_t *t;
	t=(uint16_t *)str2;
	flAddress=address;

	while (flAddress < address+lens)
	{
		data32 = *(__IO uint32_t *)flAddress;
		*t++=(uint16_t)(data32&0x0000FFFF);
		*t++=(uint16_t)((data32 >> 16)&0x0000FFFF);
		flAddress = flAddress + 4;
	}
}
uint8_t* mymemcpy (uint8_t *s1, const char *s2, uint16_t q) 
{
   uint16_t ii;
   for (ii=0; ii<q; ii++) {
      *(s1+ii)=*(s2+ii); 
   };
   return s1+ii;
}

