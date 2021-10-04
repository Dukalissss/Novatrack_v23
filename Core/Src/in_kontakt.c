#include "main.h"

IN_KONTAKT_STRUC in_kontakt;

/* ------------------------------------------------------------------------------ */
STATUS_t info;
/* ------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------ */
/* TODO: подлкючить GIT */

void Service_9(){
	/** Алгоритм работы сервиса 
	* (1) Проверка изменения состояния modex 
	* (2) Обработка изменения - смена назначения вывода 
	* (3) Обработка текущего состояния вывода 
	* (4) Пересчёт напряжения с АЦП для темературы и pwr - напряжения питания АКБ
	* (5) Выполнение калибровки по данным engine_on(off) 
	**/
	
	
	if (IsMode1_Changed(kp.mode_in1)){ /* (1) */
		ChangesProcessed_mode1(kp.mode_in1); 	/* (2) */
		info.mode1_prev = kp.mode_in1;  
	} 
	else mode1_Handler(kp.mode_in1); /* (3) */
	
	if (IsMode2_Changed(kp.mode_in2)){ /* (1) */
		ChangesProcessed_mode2(kp.mode_in2); /* (2) */
		info.mode2_prev = kp.mode_in2;	
	}
	else mode2_Handler(kp.mode_in2); /* (3) */
	
	if (info.isADCDataReady){ /* (4) */
		static int32_t temperature_mean = 0, count_temper = 0; 
		static float pwr_voltage_mean = 0;
		if (count_temper++ < Ns - 1) {
			temperature_mean += GetTemperature(info.adc_RAW[3+5]);
			pwr_voltage_mean +=  GetVoltage(info.adc_RAW[0+5], (float)(R26+R27)/R27);
		}
		else {
			in_kontakt.pwr 					= pwr_voltage_mean/Ns;	pwr_voltage_mean = 0;
			in_kontakt.temperature 	= temperature_mean/Ns;	temperature_mean = 0;
		}
		count_temper &= (Ns-1);
		info.isADCDataReady = 0; 
	}
	
	/* (5) */
	/* получение наряжения АКБ при включённом и выключенно двигателе */
	/* getting voltage of storage battary in case if engine is on and off */
	if (in_kontakt.engine_on) {	
		info.VBAT_ON = in_kontakt.pwr;	
		kp.porog_run_engine = (uint32_t)(info.VBAT_ON*1000);
		Write_kalibr_data();
		in_kontakt.engine_on = 0; 
	}
	if (in_kontakt.engine_off){ 
		info.VBAT_OFF = in_kontakt.pwr; 
		kp.porog_stop_engine = (uint32_t)(info.VBAT_OFF*1000);
		Write_kalibr_data();
		in_kontakt.engine_off = 0; 
	}
	
	if (in_kontakt.engine_state != GetEngineState()){
		in_kontakt.engine_state = GetEngineState();
		in_kontakt.changed = 1;
	}
}

void Service_9_Init(){
	/* запуск первичной настройки - калибровка АЦП */
	ADC1_StartCalibration();
	LL_ADC_Enable(ADC1);
	
	/* настройка таймеров и включение счётчиков */
	Init_TIM2();
	Init_TIM6();
	
	/* включение прерываний по входам in1, in2 */
	LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_1);
	LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_3);
	
	/* correct enabling of ADC and DMA */
	Init_GPIO_as_ADC(1);
	Init_GPIO_as_ADC(2);
	Init_DMA_and_ADC1();
	
	/* получение напряжений при включенном и выключенном двигателях из памяти EEPROM */
	info.VBAT_ON	=	kp.porog_run_engine*0.001f;
	info.VBAT_OFF	=	kp.porog_stop_engine*0.001f;
	
	/* setting flag - service9 is ready to work */
	info.is_Service9_configured = 1;
	
}


void mode1_Handler (uint8_t mode_in1){
	switch (mode_in1) {
		case 1: { /* сухой контакт. вход. отмечать, произошло ли изменение лог. уровня на выводе */ 
			static uint8_t cnt_sk1 = 5;
			if (LL_GPIO_IsInputPinSet(in1_GPIO_Port, in1_Pin))	{cnt_sk1++;}
			else { cnt_sk1--;}	
			if (cnt_sk1 > 10) {cnt_sk1 = 10; if (in_kontakt.sk1==0) {in_kontakt.sk1=1; in_kontakt.changed = 1;}}
			if (cnt_sk1 < 02) {cnt_sk1 = 02; if (in_kontakt.sk1==1) {in_kontakt.sk1=0; in_kontakt.changed = 1;}}
			
		} break;
		case 2: { /* аналоговый вход. */	
			static float ADCval =0;
			static uint16_t counter_adc1=0;
			if(counter_adc1<Ns-1){
				ADCval += GetVoltage(info.adc_RAW[1+5], (float)(R52+R49)/R52);
			}
			else {
				in_kontakt.ain1 = ADCval/Ns;
				ADCval=0;;
			}
			counter_adc1++; counter_adc1 &=(Ns-1);
		} break;
		case 3: { /* частотомер по СА*/ 
			static float freq1Mean =0;
			static uint16_t c1=0;
			if(c1<Ns-1) freq1Mean += GetFreq(info.t1_in1);
			else {			in_kontakt.freq1 = freq1Mean/Ns; freq1Mean=0; }
			++c1;
			c1 &= (Ns-1);
		} break;
		case 4: { /* сухой контакт. выход. */ 
			if (in_kontakt.sk1==1) LL_GPIO_SetOutputPin(pullsw1_GPIO_Port,pullsw1_Pin);
			else LL_GPIO_ResetOutputPin(pullsw1_GPIO_Port,pullsw1_Pin);
		} break;
		default: break; /* на всякий случай. */
	}
}

void mode2_Handler (uint8_t mode_in2){
	switch (mode_in2) {
		case 1: { /* сухой контакт. вход. отмечает, произошло ли изменение лог. уровня на выводе */ 
			static uint8_t cnt_sk2 = 5;
			if (LL_GPIO_IsInputPinSet(in2_GPIO_Port, in2_Pin))	{cnt_sk2++;}
			else { cnt_sk2--;}	
			if (cnt_sk2 > 10) {cnt_sk2 = 10; if (in_kontakt.sk2==0) {in_kontakt.sk2=1; in_kontakt.changed = 1;}}
			if (cnt_sk2 < 02) {cnt_sk2 = 02; if (in_kontakt.sk2==1) {in_kontakt.sk2=0; in_kontakt.changed = 1;}}
		} break;
		case 2: { /* аналоговый вход. */
			static float ADCval2 =0;
			static uint16_t counter_adc2=0;
			if(counter_adc2<Ns-1){
				ADCval2 += GetVoltage(info.adc_RAW[2+5], (float)(R52+R49)/R52);
			}
			else {
				in_kontakt.ain2 = ADCval2/Ns;
				ADCval2=0;;
			}
			counter_adc2++; counter_adc2&=(Ns-1);
		} break;
		case 3: { /* частотомер. */ 
			static float freq2Mean =0;
			static uint16_t counter_freq2=0;
			if(counter_freq2<Ns-1){
				freq2Mean += GetFreq(info.t1_in2);
			}
			else {
				in_kontakt.freq2 = freq2Mean/Ns;
				freq2Mean=0;;
			}
			counter_freq2++; counter_freq2&=(Ns-1);
		} break;
		case 4: { /* сухой контакт. выход. */ 
			if (in_kontakt.sk2==1) LL_GPIO_SetOutputPin(pullsw2_GPIO_Port,pullsw2_Pin);
			else LL_GPIO_ResetOutputPin(pullsw2_GPIO_Port,pullsw2_Pin);
		} break;
		default: break; /* на всякий случай. */
	}
}

/* ------------------------------------------------------------------------------ */
uint8_t IsMode1_Changed(uint8_t mode1){
	uint8_t retVal=0;
	
	/* если текущее значение режима вывода не совпало с предыдущим */
	if (mode1 == info.mode1_prev) return retVal;
	else retVal = 1; 
	return retVal;
}

uint8_t IsMode2_Changed(uint8_t mode2){
	uint8_t retVal=0;
	
	/* если текущее значение режима вывода не совпало с предыдущим */
	if (mode2 == info.mode2_prev) return retVal;
	else retVal = 1; 
	return retVal;
}
void Init_GPIO_as_ADC (uint8_t inx){
	/* основная настройка происходит в main.c */
	
	/** ADC GPIO Configuration
  in1   ------> PC1   ------> ADC_IN11
  in2   ------> PC3   ------> ADC_IN13
	*/
	
//	LL_ADC_InitTypeDef ADC_InitStruct = {0};
//  LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_ADC1);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  /**ADC GPIO Configuration
  PC1   ------> ADC_IN11
  PC3   ------> ADC_IN13
  PA7   ------> ADC_IN7
  */
	
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_ADC1);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
	
	switch (inx){
		case 1: {
			GPIO_InitStruct.Pin = in1_Pin;
			GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
			GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
			LL_GPIO_Init(in1_GPIO_Port, &GPIO_InitStruct);
			
		} break; 
		case 2: {
			GPIO_InitStruct.Pin = in2_Pin;
			GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
			GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
			LL_GPIO_Init(in2_GPIO_Port, &GPIO_InitStruct);
			
		} break;
		default: break; 
	} 
	
	LL_ADC_REG_SetSequencerChAdd(ADC1, LL_ADC_CHANNEL_11);
  LL_ADC_REG_SetSequencerChAdd(ADC1, LL_ADC_CHANNEL_13);
}

void Init_GPIO_as_EXTI(uint8_t inx){ 
	/* inx - x - номер входа "in" по принципиальной схеме */
	
	LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
	
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);	
	
	switch (inx){
		case 1: {
			LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTC, LL_SYSCFG_EXTI_LINE1);
			LL_GPIO_SetPinPull(in1_GPIO_Port, in1_Pin, LL_GPIO_PULL_NO);
			LL_GPIO_SetPinMode(in1_GPIO_Port, in1_Pin, LL_GPIO_MODE_INPUT);
			
			EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_1;
			EXTI_InitStruct.LineCommand = ENABLE;
			EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
			EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
			LL_EXTI_Init(&EXTI_InitStruct);
			
			NVIC_SetPriority(EXTI0_1_IRQn, 0);
			NVIC_EnableIRQ(EXTI0_1_IRQn);
			
			/* включение прерываний */
			LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_1);
		} break;
		case 2: {
			LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTC, LL_SYSCFG_EXTI_LINE3);
			LL_GPIO_SetPinPull(in2_GPIO_Port, in2_Pin, LL_GPIO_PULL_NO);
			LL_GPIO_SetPinMode(in2_GPIO_Port, in2_Pin, LL_GPIO_MODE_INPUT);
			
			EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_3;
			EXTI_InitStruct.LineCommand = ENABLE;
			EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
			EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
			LL_EXTI_Init(&EXTI_InitStruct);
			
			NVIC_SetPriority(EXTI2_3_IRQn, 0);
			NVIC_EnableIRQ(EXTI2_3_IRQn);
			
			/* включение прерываний */
			LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_3);
		} break; 
		default: break;
	}
} 

void Init_TIM2(){
	
	/* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  TIM_InitStruct.Prescaler = TIM_PSC;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 4294967295;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM2, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM2);
  TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_FROZEN;
  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.CompareValue = 0;
  TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
  LL_TIM_OC_Init(TIM2, LL_TIM_CHANNEL_CH1, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM2, LL_TIM_CHANNEL_CH1);
  LL_TIM_SetTriggerOutput(TIM2, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM2);
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
	
	/* включение прерывания по переполнению ARR */
	LL_TIM_EnableCounter(TIM2);
//	LL_TIM_EnableIT_UPDATE(TIM2);
} 

void Init_TIM6(){
	/* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM6);

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  TIM_InitStruct.Prescaler = TIM_PSC;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 65535;
  LL_TIM_Init(TIM6, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM6);
  LL_TIM_SetTriggerOutput(TIM6, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM6);
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */
	
	LL_TIM_EnableCounter(TIM6);
}

void Init_DMA_and_ADC1(){
	/* настройка DMA & включение ADC1 */
//	LL_DMA_SetMemoryAddress(DMA1,LL_DMA_CHANNEL_1, (uint32_t)(info.adc_RAW));
//	LL_DMA_SetPeriphAddress(DMA1,LL_DMA_CHANNEL_1, (uint32_t)&ADC1->DR);
	LL_DMA_ConfigTransfer(DMA1,
                        LL_DMA_CHANNEL_1,
                        LL_DMA_DIRECTION_PERIPH_TO_MEMORY |
                        LL_DMA_MODE_CIRCULAR              |
                        LL_DMA_PERIPH_NOINCREMENT         |
                        LL_DMA_MEMORY_INCREMENT           |
                        LL_DMA_PDATAALIGN_HALFWORD        |
                        LL_DMA_MDATAALIGN_HALFWORD        |
                        LL_DMA_PRIORITY_HIGH               );
	 LL_DMA_ConfigAddresses(DMA1,
                         LL_DMA_CHANNEL_1,
                         LL_ADC_DMA_GetRegAddr(ADC1, LL_ADC_DMA_REG_REGULAR_DATA),
                         (uint32_t)&info.adc_RAW[0],
                         LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
	LL_DMA_SetDataLength(DMA1,LL_DMA_CHANNEL_1,(uint32_t)5);
	LL_DMA_EnableIT_TC(DMA1,LL_DMA_CHANNEL_1);
	LL_DMA_EnableIT_TE(DMA1,LL_DMA_CHANNEL_1);
	LL_DMA_EnableChannel(DMA1,LL_DMA_CHANNEL_1);
	
//	while(LL_ADC_IsEnabled(ADC1));
//	LL_ADC_StartCalibration(ADC1);
//	while(LL_ADC_IsCalibrationOnGoing(ADC1));
//	info.VREF_CAL_ACTUAL = LL_ADC_REG_ReadConversionData12(ADC1);
	LL_ADC_Enable(ADC1);
	while (!LL_ADC_IsActiveFlag_ADRDY(ADC1));
	LL_ADC_REG_StartConversion(ADC1);
}
void Deinit_All(uint8_t inx){
	/* inx - номер входа по ПС */
	/* таймеры никогда не отключаются */
	/* АЦП никогда не выключается */
	
	/* отключение внешних прерываний */
	if (inx==1) LL_EXTI_DisableIT_0_31(LL_EXTI_LINE_1);
	if (inx==2) LL_EXTI_DisableIT_0_31(LL_EXTI_LINE_3);
	
	/* инициализация pullsw1,2 как выходы с лог. нулём без подтяжек 
	происходит один раз в main.c и больше не меняется, иначе будет сброс лог. уровней на выводах
	**/
	
	/* инициализация in1,2 как входы без подтяжек */
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	if (inx==1){
		GPIO_InitStruct.Pin = in1_Pin;						
		GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
		LL_GPIO_Init(in1_GPIO_Port, &GPIO_InitStruct);
	}
	if (inx==2){
		GPIO_InitStruct.Pin = in2_Pin;
		GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
		LL_GPIO_Init(in2_GPIO_Port, &GPIO_InitStruct);
	}
}
void ChangesProcessed_mode1(uint8_t mode1){
	
	/* вернуть все в исходное состояние для in1 */
	Deinit_All(1);
	
	/* настроить inx в соответствии с mode1 */
	switch (mode1){
		case 1:{ /* сухой контакт (вход). */
			/* инициализация in1 как внешнее прерывание. pullsw1 - как выход - притянут к земле */
			Init_GPIO_as_EXTI(1);
			LL_GPIO_ResetOutputPin(pullsw1_GPIO_Port,pullsw1_Pin);
		} 
			break;
		case 2:{ /* аналоговый вход */
			/* инициализация in1 как аналоговый вход. pullsw1 - как выход - притянут к земле */
			Init_GPIO_as_ADC(1);
			Init_DMA_and_ADC1();
			LL_GPIO_ResetOutputPin(pullsw1_GPIO_Port,pullsw1_Pin);
		} 
			break;
		case 3:{ /* частотный вход */ // считывать с in1
			/* инициализация in1 как вход внешнего прерывания. pullsw1 - как выход - притянут к земле */
			Init_GPIO_as_EXTI(1);
			Init_TIM6(); // для определения частоты
			LL_GPIO_ResetOutputPin(pullsw1_GPIO_Port,pullsw1_Pin);
		} 
			break;
		case 4:{ /* сухой контакт (выход) */
			/* инициализация in1 как входа. Инициализация pullsw1 как цифрового выхода  */
			/* управляющее воздействие: in_kontakt.sk1 - устанавливается на pullsw1 */
		} 
			break;
		default: break;
	} 
}
void ChangesProcessed_mode2(uint8_t mode2){
	
	/* вернуть все в исходное состояние для in2 */
	Deinit_All(2);
	
	/* настроить inx в соответствии с mode1 */
	switch (mode2){
		case 1:{ /* сухой контакт (вход). */
			/* инициализация in1 как внешнее прерывание. pullsw1 - как выход - притянут к земле */
			Init_GPIO_as_EXTI(2);
			LL_GPIO_ResetOutputPin(pullsw2_GPIO_Port,pullsw2_Pin);
		} 
			break;
		case 2:{ /* аналоговый вход */
			/* инициализация in1 как аналоговый вход. pullsw1 - как выход - притянут к земле */
			Init_GPIO_as_ADC(2);
			Init_DMA_and_ADC1();
			LL_GPIO_ResetOutputPin(pullsw2_GPIO_Port,pullsw2_Pin);
		} 
			break;
		case 3:{ /* частотный вход */ // считывать с in1
			/* инициализация in2 как вход внешнего прерывания. pullsw1 - как выход - притянут к земле */
			Init_GPIO_as_EXTI(2);
			Init_TIM2(); // для определения частоты
			LL_GPIO_ResetOutputPin(pullsw2_GPIO_Port,pullsw2_Pin);
		} 
			break;
		case 4:{ /* сухой контакт (выход) */
			/* инициализация in2 как входа. Инициализация pullsw1 как цифрового выхода  */
			/* управляющее воздействие: in_kontakt.sk1 - устанавливается на pullsw1 */
		} 
			break;
		default: break;
	} 
}
void ADC1_StartCalibration (){
	/* (1) Ensure that ADEN = 0 */
	/* (2) Clear ADEN by setting ADDIS*/
	/* (3) Clear DMAEN */
	/* (4) Launch the calibration by setting ADCAL */
	/* (5) Wait until ADCAL=0 */
	/* NB: нужно запускать калибровку каждый раз, при выключении/включении ADC */
	
	if (LL_ADC_IsEnabled(ADC1)){ /* (1) */
		LL_ADC_Disable(ADC1); /* (2) */
	}
	while (LL_ADC_IsEnabled(ADC1)) {
		/* For robust implementation, add here time-out management */
		
	};
	ADC1->CFGR1 &= ~ADC_CFGR1_DMAEN; /* (3) */
	LL_ADC_StartCalibration(ADC1); /* (4) */
//	isCalibrationStarted = 1;
	while (LL_ADC_IsCalibrationOnGoing(ADC1)){
		/* For robust implementation, add here time-out management */
		
	};
	/* в DR теперь содержится калибровочное значение ADC */
	/* флаг об окончании калибровки сборосится автоматом */
	info.VREF_CAL_ACTUAL = LL_ADC_REG_ReadConversionData12(ADC1);
	/* вторичная калибровка не нужна. сброс флагов. */
	info.ADC_isCalibrated = 1; 
	
	ADC1->CFGR1 |= ADC_CFGR1_DMAEN;
	
}

float GetVoltage (uint16_t adc_val, float K){
	float V;
	V =(float)((float)adc_val*VDD_APPLI/VDD_CALIB/4096.0f)*3.30f*K;
	return V;
} 
float GetTemperature (uint16_t adc_val){
	
int32_t temperature; /* will contain the temperature in degrees Celsius */
temperature = (((int32_t) adc_val * VDD_APPLI / VDD_CALIB) - (int32_t) *TEMP30_CAL_ADDR );
temperature = temperature * (int32_t)(110 - 30);
temperature = temperature / (int32_t)(*TEMP110_CAL_ADDR - *TEMP30_CAL_ADDR);
temperature = temperature + 30;

return temperature;
}
float GetFreq (uint32_t t1){
	static float freq=0;
	freq = 12/(TIM_PSC+1)*1000000.0f/((float)(t1+1));
	return freq;
}
uint8_t GetEngineState (){
	uint8_t retval; // engine state 0 - don't known; 1 - powered; 2 - turned off; 
	if (info.VBAT_OFF && info.VBAT_ON) {
		info.V_MEAN = 0.5f*(info.VBAT_OFF+info.VBAT_ON);
		if (in_kontakt.pwr > info.V_MEAN){	retval = 1;	} 	// двигатель запущен
		else 																retval = 2; 		// двигатель заглушен
	}
	else retval = 0; // состояние двигателя неизвестно 
	return retval;
}
/* ------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------ */




