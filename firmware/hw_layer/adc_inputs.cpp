/**
 * @file	adc_inputs.cpp
 * @brief	Low level ADC code
 *
 * @date Jan 14, 2013
 * @author Andrey Belomutskiy, (c) 2012-2014
 */

#include "main.h"
#include "engine_configuration.h"
#include "adc_inputs.h"
#include "AdcConfiguration.h"

#include "pin_repository.h"
#include "engine_math.h"

#if EFI_SPEED_DENSITY
#include "map_averaging.h"
#endif /* EFI_SPEED_DENSITY */

AdcConfiguration::AdcConfiguration(ADCConversionGroup* hwConfig) {
	this->hwConfig = hwConfig;
	channelCount = 0;
	conversionCount = 0;

	hwConfig->sqr1 = 0;
	hwConfig->sqr2 = 0;
	hwConfig->sqr3 = 0;
	memset(internalAdcIndexByHardwareIndex, 0xFFFFFFFF, sizeof(internalAdcIndexByHardwareIndex));
}

#define ADC_GRP1_BUF_DEPTH_FAST      1

#define ADC_NUMBER_CHANNELS_FAST		1

// todo: migrate from hardware timer to software ADC conversion triggering
// todo: I guess we would have to use ChibiOS timer and not our own timer because
// todo: adcStartConversionI requires OS lock. currently slow ADC is 10Hz (?)
#define PWM_FREQ_SLOW 5000   /* PWM clock frequency. I wonder what does this setting mean?  */
#define PWM_PERIOD_SLOW 500  /* PWM period (in PWM ticks).    */

/**
 * 8000 RPM is 133Hz
 * If we want to sample MAP once per 5 degrees we need 133Hz * (360 / 5) = 9576Hz of fast ADC
 */
// todo: migrate to continues ADC mode? probably not - we cannot afford the callback in
// todo: continues mode. todo: look into our options
#define PWM_FREQ_FAST 100000   /* PWM clock frequency. I wonder what does this setting mean?  */
#define PWM_PERIOD_FAST 10  /* PWM period (in PWM ticks).    */

#define ADC_SLOW_DEVICE ADCD1

#define ADC_FAST_DEVICE ADCD2

#define ADC_DEBUG_KEY "adcDebug"

static char LOGGING_BUFFER[500];
static Logging logger;
static int adcCallbackCounter_slow = 0;

static int adcDebugReporting = FALSE;

static int fastAdcValue;
extern engine_configuration_s *engineConfiguration;
extern board_configuration_s *boardConfiguration;

static adc_hw_helper_s slowAdcState;

/*
 * ADC samples buffer.
 */
static adcsample_t samples_fast[ADC_NUMBER_CHANNELS_FAST * ADC_GRP1_BUF_DEPTH_FAST];

static adcsample_t getAvgAdcValue(int index, adcsample_t *samples, int bufDepth, int numChannels) {
	adcsample_t result = 0;
	int i;
	for (i = 0; i < bufDepth; i++) {
		result += samples[index];
		index += numChannels;
	}
	return result / bufDepth;
}

static void adc_callback_slow(ADCDriver *adcp, adcsample_t *buffer, size_t n);
static void adc_callback_fast(ADCDriver *adcp, adcsample_t *buffer, size_t n);

#define MY_SAMPLING_SLOW ADC_SAMPLE_480
#define MY_SAMPLING_FAST ADC_SAMPLE_28

/*
 * ADC conversion group.
 */
static ADCConversionGroup adcgrpcfgSlow = { FALSE, 0, adc_callback_slow, NULL,
/* HW dependent part.*/
ADC_TwoSamplingDelay_20Cycles,   // cr1
		ADC_CR2_SWSTART, // cr2

		ADC_SMPR1_SMP_AN10(MY_SAMPLING_SLOW) |
		ADC_SMPR1_SMP_AN11(MY_SAMPLING_SLOW) |
		ADC_SMPR1_SMP_AN12(MY_SAMPLING_SLOW) |
		ADC_SMPR1_SMP_AN13(MY_SAMPLING_SLOW), // sample times for channels 10...18
		ADC_SMPR2_SMP_AN0(MY_SAMPLING_SLOW) |
		ADC_SMPR2_SMP_AN1(MY_SAMPLING_SLOW) |
		ADC_SMPR2_SMP_AN3(MY_SAMPLING_SLOW) |
		ADC_SMPR2_SMP_AN4(MY_SAMPLING_SLOW) |
		ADC_SMPR2_SMP_AN5(MY_SAMPLING_SLOW) |
		ADC_SMPR2_SMP_AN6(MY_SAMPLING_SLOW) |
		ADC_SMPR2_SMP_AN7(MY_SAMPLING_SLOW) |
		ADC_SMPR2_SMP_AN8(MY_SAMPLING_SLOW) |
		ADC_SMPR2_SMP_AN9(MY_SAMPLING_SLOW)

		, // In this field must be specified the sample times for channels 0...9

		0, // Conversion group sequence 13...16 + sequence length

		0
//		| ADC_SQR2_SQ7_N(ADC_CHANNEL_IN12) /* PC2 - green */
//				| ADC_SQR2_SQ8_N(ADC_CHANNEL_IN13) /* PC3 - yellow maf? */

		,// Conversion group sequence 7...12
		0
//		| ADC_SQR3_SQ1_N(ADC_CHANNEL_IN6) /* PA6 - white */
//		| ADC_SQR3_SQ2_N(ADC_CHANNEL_IN7) /* PA7 - blue */
//				| ADC_SQR3_SQ3_N(ADC_CHANNEL_IN14) /* PC4 - green */
//				| ADC_SQR3_SQ4_N(ADC_CHANNEL_IN15) /* PC5 - yellow */
//				| ADC_SQR3_SQ5_N(ADC_CHANNEL_IN8) /* PB0 - blue */
//				| ADC_SQR3_SQ6_N(ADC_CHANNEL_IN9) /* PB1 - white */
// Conversion group sequence 1...6
		};

AdcConfiguration slowAdc(&adcgrpcfgSlow);

static ADCConversionGroup adcgrpcfg_fast = { FALSE, 0 /* num_channels */, adc_callback_fast, NULL,
/* HW dependent part.*/
ADC_TwoSamplingDelay_5Cycles,   // cr1
		ADC_CR2_SWSTART, // cr2

		0, // sample times for channels 10...18
		ADC_SMPR2_SMP_AN0(MY_SAMPLING_FAST), // In this field must be specified the sample times for channels 0...9

		ADC_SQR1_NUM_CH(ADC_NUMBER_CHANNELS_FAST), // Conversion group sequence 13...16 + sequence length

		0, // Conversion group sequence 7...12
		0

// Conversion group sequence 1...6
		};

AdcConfiguration fastAdc(&adcgrpcfg_fast);

static void pwmpcb_slow(PWMDriver *pwmp) {
#if EFI_INTERNAL_ADC
	(void) pwmp;

	/* Starts an asynchronous ADC conversion operation, the conversion
	 will be executed in parallel to the current PWM cycle and will
	 terminate before the next PWM cycle.*/
	chSysLockFromIsr()
	;
	if (ADC_SLOW_DEVICE.state != ADC_READY &&
	ADC_SLOW_DEVICE.state != ADC_COMPLETE &&
	ADC_SLOW_DEVICE.state != ADC_ERROR) {
		// todo: why and when does this happen? firmwareError("ADC slow not ready?");
		slowAdc.errorsCount++;
		chSysUnlockFromIsr()
		;
		return;
	}
	slowAdc.errorsCount++;
	adcStartConversionI(&ADC_SLOW_DEVICE, &adcgrpcfgSlow, slowAdcState.samples, ADC_GRP1_BUF_DEPTH_SLOW);
	chSysUnlockFromIsr()
	;
	slowAdc.conversionCount++;
#endif
}

static void pwmpcb_fast(PWMDriver *pwmp) {
#if EFI_INTERNAL_ADC
	(void) pwmp;

	/*
	 * Starts an asynchronous ADC conversion operation, the conversion
	 * will be executed in parallel to the current PWM cycle and will
	 * terminate before the next PWM cycle.
	 */
	chSysLockFromIsr()
	;
	if (ADC_FAST_DEVICE.state != ADC_READY &&
	ADC_FAST_DEVICE.state != ADC_COMPLETE &&
	ADC_FAST_DEVICE.state != ADC_ERROR) {
		fastAdc.errorsCount++;
		// todo: when? why? firmwareError("ADC fast not ready?");
		chSysUnlockFromIsr()
		;
		return;
	}
	adcStartConversionI(&ADC_FAST_DEVICE, &adcgrpcfg_fast, samples_fast, ADC_GRP1_BUF_DEPTH_FAST);
	chSysUnlockFromIsr()
	;
	fastAdc.conversionCount++;
#endif
}

int getInternalAdcValue(adc_channel_e hwChannel) {
	if (boardConfiguration->adcHwChannelEnabled[hwChannel] == ADC_FAST)
		return fastAdcValue;

	int internalIndex = slowAdc.internalAdcIndexByHardwareIndex[hwChannel];
	return slowAdc.getAdcValueByIndex(internalIndex);
}

static PWMConfig pwmcfg_slow = { PWM_FREQ_SLOW, PWM_PERIOD_SLOW, pwmpcb_slow, { {
PWM_OUTPUT_DISABLED, NULL }, { PWM_OUTPUT_DISABLED, NULL }, {
PWM_OUTPUT_DISABLED, NULL }, { PWM_OUTPUT_DISABLED, NULL } },
/* HW dependent part.*/
0, 0 };

static PWMConfig pwmcfg_fast = { PWM_FREQ_FAST, PWM_PERIOD_FAST, pwmpcb_fast, { {
PWM_OUTPUT_DISABLED, NULL }, { PWM_OUTPUT_DISABLED, NULL }, {
PWM_OUTPUT_DISABLED, NULL }, { PWM_OUTPUT_DISABLED, NULL } },
/* HW dependent part.*/
0, 0 };

static void initAdcPin(ioportid_t port, int pin, const char *msg) {
	print("adc %s\r\n", msg);
	mySetPadMode("adc input", port, pin, PAL_MODE_INPUT_ANALOG);
}

adc_channel_e getAdcChannel(brain_pin_e pin) {
	switch(pin) {
	case GPIOA_0:
		return EFI_ADC_0;
	case GPIOA_1:
		return EFI_ADC_1;
	case GPIOA_2:
		return EFI_ADC_2;
	case GPIOA_3:
		return EFI_ADC_3;
	case GPIOA_4:
		return EFI_ADC_4;
	case GPIOA_5:
		return EFI_ADC_5;
	case GPIOA_6:
		return EFI_ADC_6;
	case GPIOA_7:
		return EFI_ADC_7;
	case GPIOB_0:
		return EFI_ADC_8;
	case GPIOB_1:
		return EFI_ADC_9;
	case GPIOC_0:
		return EFI_ADC_10;
	case GPIOC_1:
		return EFI_ADC_11;
	case GPIOC_2:
		return EFI_ADC_12;
	case GPIOC_3:
		return EFI_ADC_13;
	case GPIOC_4:
		return EFI_ADC_14;
	case GPIOC_5:
		return EFI_ADC_15;
	default:
		return EFI_ADC_ERROR;
	}
}

GPIO_TypeDef* getAdcChannelPort(adc_channel_e hwChannel) {
	// todo: replace this with an array :)
	switch (hwChannel) {
	case ADC_CHANNEL_IN0:
		return GPIOA;
	case ADC_CHANNEL_IN1:
		return GPIOA;
	case ADC_CHANNEL_IN2:
		return GPIOA;
	case ADC_CHANNEL_IN3:
		return GPIOA;
	case ADC_CHANNEL_IN4:
		return GPIOA;
	case ADC_CHANNEL_IN5:
		return GPIOA;
	case ADC_CHANNEL_IN6:
		return GPIOA;
	case ADC_CHANNEL_IN7:
		return GPIOA;
	case ADC_CHANNEL_IN8:
		return GPIOB;
	case ADC_CHANNEL_IN9:
		return GPIOB;
	case ADC_CHANNEL_IN10:
		return GPIOC;
	case ADC_CHANNEL_IN11:
		return GPIOC;
	case ADC_CHANNEL_IN12:
		return GPIOC;
	case ADC_CHANNEL_IN13:
		return GPIOC;
	case ADC_CHANNEL_IN14:
		return GPIOC;
	case ADC_CHANNEL_IN15:
		return GPIOC;
	default:
		firmwareError("Unknown hw channel");
		return NULL;
	}
}

const char * getAdcMode(adc_channel_e hwChannel) {
	if (slowAdc.isHwUsed(hwChannel)) {
		return "slow";
	}
	if (fastAdc.isHwUsed(hwChannel)) {
		return "fast";
	}
	return "INACTIVE";
}

int getAdcChannelPin(adc_channel_e hwChannel) {
	// todo: replace this with an array :)
	switch (hwChannel) {
	case ADC_CHANNEL_IN0:
		return 0;
	case ADC_CHANNEL_IN1:
		return 1;
	case ADC_CHANNEL_IN2:
		return 2;
	case ADC_CHANNEL_IN3:
		return 3;
	case ADC_CHANNEL_IN4:
		return 4;
	case ADC_CHANNEL_IN5:
		return 5;
	case ADC_CHANNEL_IN6:
		return 6;
	case ADC_CHANNEL_IN7:
		return 7;
	case ADC_CHANNEL_IN8:
		return 0;
	case ADC_CHANNEL_IN9:
		return 1;
	case ADC_CHANNEL_IN10:
		return 0;
	case ADC_CHANNEL_IN11:
		return 1;
	case ADC_CHANNEL_IN12:
		return 2;
	case ADC_CHANNEL_IN13:
		return 3;
	case ADC_CHANNEL_IN14:
		return 4;
	case ADC_CHANNEL_IN15:
		return 5;
	default:
		firmwareError("Unknown hw channel");
		return -1;
	}
}

static void initAdcHwChannel(adc_channel_e hwChannel) {
	GPIO_TypeDef* port = getAdcChannelPort(hwChannel);
	int pin = getAdcChannelPin(hwChannel);

	initAdcPin(port, pin, "hw");
}

int AdcConfiguration::size() {
	return channelCount;
}

int AdcConfiguration::getAdcValueByIndex(int internalIndex) {
	return values.adc_data[internalIndex];
}

void AdcConfiguration::init(void) {
	hwConfig->num_channels = size();
	hwConfig->sqr1 += ADC_SQR1_NUM_CH(size());
}

bool AdcConfiguration::isHwUsed(adc_channel_e hwChannelIndex) {
	for (int i = 0; i < channelCount; i++) {
		if (hardwareIndexByIndernalAdcIndex[i] == hwChannelIndex) {
			return true;
		}
	}
	return false;
}

void AdcConfiguration::addChannel(adc_channel_e hwChannel) {
	int logicChannel = channelCount++;

	internalAdcIndexByHardwareIndex[hwChannel] = logicChannel;
	hardwareIndexByIndernalAdcIndex[logicChannel] = hwChannel;
	if (logicChannel < 6) {
		hwConfig->sqr3 += (hwChannel) << (5 * logicChannel);
	} else {
		hwConfig->sqr2 += (hwChannel) << (5 * (logicChannel - 6));
	}
	// todo: support for more then 12 channels? not sure how needed it would be

	initAdcHwChannel(hwChannel);
}

static void printAdcValue(adc_channel_e channel) {
	int value = getAdcValue(channel);
	float volts = adcToVoltsDivided(value);
	scheduleMsg(&logger, "adc voltage : %f", volts);
}

adc_channel_e AdcConfiguration::getAdcHardwareIndexByInternalIndex(int index) {
	return hardwareIndexByIndernalAdcIndex[index];
}

static void printFullAdcReport(void) {
	scheduleMsg(&logger, "fast %d slow %d", fastAdc.conversionCount, slowAdc.conversionCount);

	for (int index = 0; index < slowAdc.size(); index++) {
		appendMsgPrefix(&logger);

		adc_channel_e hwIndex = slowAdc.getAdcHardwareIndexByInternalIndex(index);
		GPIO_TypeDef* port = getAdcChannelPort(hwIndex);
		int pin = getAdcChannelPin(hwIndex);

		int adcValue = slowAdc.getAdcValueByIndex(index);
		appendPrintf(&logger, " ch%d %s%d", index, portname(port), pin);
		appendPrintf(&logger, " ADC%d 12bit=%d", hwIndex, adcValue);
		float volts = adcToVolts(adcValue);
		appendPrintf(&logger, " v=%f", volts);

		appendMsgPostfix(&logger);
		scheduleLogging(&logger);
	}
}

static void printStatus(void) {
	scheduleIntValue(&logger, ADC_DEBUG_KEY, adcDebugReporting);
}

static void setAdcDebugReporting(int value) {
	adcDebugReporting = value;
	printStatus();
}

static void adc_callback_slow(ADCDriver *adcp, adcsample_t *buffer, size_t n) {
	(void) buffer;
	(void) n;
	/* Note, only in the ADC_COMPLETE state because the ADC driver fires
	 * an intermediate callback when the buffer is half full. */
	if (adcp->state == ADC_COMPLETE) {
		/* Calculates the average values from the ADC samples.*/

		adcCallbackCounter_slow++;

//		newState.time = chimeNow();
		for (int i = 0; i < slowAdc.size(); i++) {
			int value = getAvgAdcValue(i, slowAdcState.samples, ADC_GRP1_BUF_DEPTH_SLOW, slowAdc.size());
			slowAdc.values.adc_data[i] = value;
		}
	}
}

static void adc_callback_fast(ADCDriver *adcp, adcsample_t *buffer, size_t n) {
	(void) buffer;
	(void) n;
//	/* Note, only in the ADC_COMPLETE state because the ADC driver fires an
//	 intermediate callback when the buffer is half full.*/
	if (adcp->state == ADC_COMPLETE) {
		fastAdcValue = getAvgAdcValue(0, samples_fast, ADC_GRP1_BUF_DEPTH_FAST, fastAdc.size());

		fastAdc.values.adc_data[0] = fastAdcValue;

#if EFI_MAP_AVERAGING
		mapAveragingCallback(fastAdcValue);
#endif /* EFI_MAP_AVERAGING */
	}
}

void initAdcInputs(void) {

	initLoggingExt(&logger, "ADC", LOGGING_BUFFER, sizeof(LOGGING_BUFFER));
	printMsg(&logger, "initAdcInputs()");

	printStatus();

	addConsoleActionI(ADC_DEBUG_KEY, &setAdcDebugReporting);

#if EFI_INTERNAL_ADC
	/*
	 * Initializes the ADC driver.
	 */
	adcStart(&ADC_SLOW_DEVICE, NULL);
	adcStart(&ADC_FAST_DEVICE, NULL);

	for (int adc = 0; adc < HW_MAX_ADC_INDEX; adc++) {
		adc_channel_mode_e mode = boardConfiguration->adcHwChannelEnabled[adc];

		if (mode == ADC_SLOW) {
			slowAdc.addChannel((adc_channel_e) (ADC_CHANNEL_IN0 + adc));
		} else if (mode == ADC_FAST) {
			fastAdc.addChannel((adc_channel_e) (ADC_CHANNEL_IN0 + adc));
		}
	}

	slowAdc.init();
	pwmStart(EFI_INTERNAL_SLOW_ADC_PWM, &pwmcfg_slow);
	fastAdc.init();
	/*
	 * Initializes the PWM driver.
	 */
	pwmStart(EFI_INTERNAL_FAST_ADC_PWM, &pwmcfg_fast);

	// ADC_CHANNEL_IN0 // PA0
	// ADC_CHANNEL_IN1 // PA1
	// ADC_CHANNEL_IN2 // PA2
	// ADC_CHANNEL_IN3 // PA3
	// ADC_CHANNEL_IN4 // PA4
	// ADC_CHANNEL_IN5 // PA5 - this is also TIM2_CH1
	// ADC_CHANNEL_IN6 // PA6
	// ADC_CHANNEL_IN7 // PA7
	// ADC_CHANNEL_IN8 // PB0
	// ADC_CHANNEL_IN9 // PB1
	// ADC_CHANNEL_IN10 // PC0
	// ADC_CHANNEL_IN11 // PC1
	// ADC_CHANNEL_IN12 // PC2
	// ADC_CHANNEL_IN13 // PC3
	// ADC_CHANNEL_IN14 // PC4
	// ADC_CHANNEL_IN15 // PC5

	//if(slowAdcChannelCount > ADC_MAX_SLOW_CHANNELS_COUNT) // todo: do we need this logic? do we need this check

	addConsoleActionI("adc", (VoidInt) printAdcValue);
	addConsoleAction("fadc", printFullAdcReport);
#else
	printMsg(&logger, "ADC disabled");
#endif
}

void pokeAdcInputs() {
	if (!adcDebugReporting)
		return;
	printFullAdcReport();
}

