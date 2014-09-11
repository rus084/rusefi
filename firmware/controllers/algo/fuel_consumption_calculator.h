/**
 * @file    fuel_consumption_calculator.cpp
 * @brief   fuel consumption calculator
 *
 * @date May 3, 2014
 * @author rus084
 */

#include "engine_configuration.h"


#ifndef FUEL_CONSUMPTION_CALCULATOR_H_
#define FUEL_CONSUMPTION_CALCULATOR_H_

class FuelConsumption {
public :
	FuelConsumption();
	void refreshConsumption(float fuel);
	float getConsumptionCurrLiterPerHour(void);
	float getFuelTrip(void);
private:
	float fuelConsumption;
	uint64_t nowUs;
	uint64_t diff;
	uint64_t lastFuelEventTimeUs;
};

void refreshConsumption(float fuel);
float getConsumptionCurrLiterPerHour(void);
float getFuelTrip(void);


#endif /* FUEL_CONSUMPTION_CALCULATOR_H_ */
