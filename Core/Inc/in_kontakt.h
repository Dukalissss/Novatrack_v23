#ifndef SERVICE_9_H__
#define SERVICE_9_H__

#include "main.h"

/* public settings -------------------------------------------------------------- */
#define R49 				220 			/* сопротивления резисторов на inx (кОм.) */
#define R52 				91 		
#define R27					10				/* сопротивления резисторов на pwr (кОм.) */
#define R26					220				
#define TIM_PSC 		2 				/* предделитель таймеров */
#define Ns					128				/* количество значений усреднения частоты - степень двойки*/

/* private settings ------------------------------------------------------------- */
/* данные значения взяты из примера: RM. p. 943/1004 - A.7.16 -  Temperature computation code example */
#define TEMP110_CAL_ADDR 	((uint16_t*) ((uint32_t) 0x1FFFF7C2))
#define TEMP30_CAL_ADDR 	((uint16_t*) ((uint32_t) 0x1FFFF7B8))
#define VDD_CALIB 				((uint16_t) (330))
#define VDD_APPLI 				((uint16_t) (300))

#define INACTIVE_MODE				0x00
#define DIGITAL_IN_MODE			0x01
#define ANALOGUE_IN_MODE		0x02
#define FREQ_IN_MODE				0x03
#define DIGITAL_OUT_MODE		0x04

typedef struct { /*  */
  uint8_t       changed;
  uint8_t       sk1 ;
  uint8_t       sk2;
  float         ain1;
  float         ain2;
  float         pwr;
  float      		freq1, freq2;
  float         temperature;
	uint8_t 			engine_on, engine_off, engine_state;
} IN_KONTAKT_STRUC;

extern IN_KONTAKT_STRUC in_kontakt;

typedef struct { // информация по состоянию сервиса
	// --------------------------------------------------------
	
	// Status section ----------------------------------------- 
	uint8_t is_Service9_configured;/* флаг заверешения настройки сервиса9 к работе */
	
	// ADC section -------------------------------------------- 
	uint16_t 	VREF_CAL_ACTUAL;			/* калибровочное значение АЦП. */
	uint8_t 	ADC_isCalibrated;			/* флаг, показывающий необходимость калибровки ADC */
	uint16_t 	adc_RAW[5*2];					/* массив сырых значений, полученных при помощи DMA */
	uint8_t 	isADCDataReady;				/* флаг показывающий что данные из массиыва выше готовы для пересчёта в другие величины */
	
	// modex section ------------------------------------------
	uint8_t mode1_prev;							/* прошлые значения для сравнения режимов modex*/
	uint8_t mode2_prev;
	
	// freq section -------------------------------------------
	/* переменные для вычисления частоты */
	uint32_t t1_in1; 								// для первого входа. Используется в обработчике прерывания  входа 1
	uint32_t t1_in2; 								// для второго входа. Используется в обработчике прерывания  входа 2
	
	/* напряжения при выключенном двигателе и при включённом */
	float VBAT_ON, VBAT_OFF, V_MEAN;
	
} STATUS_t;

/* ------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------ */
void 			Deinit_All (uint8_t inx); 											/* функция сбрасывает выводы (in1,in2) в состояние по умолчанию (входы без подтяжек) */
void 			Init_GPIO_as_EXTI(uint8_t which_input);					/* функция инциализирует внешнее прерывание по фронту на выбранном канале*/
void 			Init_GPIO_as_ADC (uint8_t inx);									/* функция инициализирует АЦП на выбранном канале */
void 			Init_TIM2(void);																/* функция инициализирует таймер TIM2 */
void 			Init_TIM6(void); 																/* функция инициализирует таймер TIM6 */
void 			Init_DMA_and_ADC1(void); 												/* функция настраивает каналы 7,11,13,16 АЦП для работы с DMA */
void 			Enable_ADC1 (void);															/* функция включает АЦП1 */
void 			ADC1_StartCalibration (void);										/* функция запускает калибровку АЦП */
uint8_t 	IsMode1_Changed(uint8_t mode1);									/* функция "IsModex_Changed" проверяет, был ли изменён режим modex, сравнивая с предыдущим значением */
uint8_t 	IsMode2_Changed(uint8_t mode2);								
void 			ChangesProcessed_mode1(uint8_t val);						/* функция запускается после проверки о несовпадении прошлого и текущего состояния modex для его перенастройки */
void 			ChangesProcessed_mode2(uint8_t val);							
uint16_t 	AnalogRead(uint8_t inx);												/* функция запускает преобразование АЦП и ждет его окончания, после чего возвращает полученное значение */
float 		GetVoltage (uint16_t adcv, float k);  					/* функция пересчёт сырого значения с АЦП в напряжение, поступающее на делитель, с его последующим возвратом */
float 		GetTemperature (uint16_t adc_val);							/* функция пересчёт сырого значения с АЦП в температуру с её последующим возвратом */
float 		GetFreq (uint32_t t1);													/* функция вычисляет частоту сигнала по задержкам t1, t2 */
uint8_t 	GetEngineState (void);													/* функция определяет включён ли двигатель или нет */
/* ------------------------------------------------------------------------------ */
void 			Service_9_Init(void);														/* функция запускается перед супер-циклом и настраивает сервис к работе */
void 			Service_9(void);													 			/* функция вызывается в супер-цикле программы. Основная функция сервиса9 */
void 			mode1_Handler (uint8_t mode_in1);								/* modex_Handler обрабатывает текущий режим modex */
void 			mode2_Handler (uint8_t mode_in1);														
/* ------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------ */
extern volatile uint8_t 	t1_ms;													/* значение времени в мсек. суперсчётчик в основном цикле программы */
extern IN_KONTAKT_STRUC 	in_kontakt;											/* внешняя структура "Сухой Контакт" */
extern STATUS_t 					info; 													/* структура с информацией по текущему состоянию сервиса9 */
/* ------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------ */
#endif  




