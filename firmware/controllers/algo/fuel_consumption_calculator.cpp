/**
 * @file    fuel_consumption_calculator.cpp
 * @brief   fuel consumption calculator
 *
 * @date May 3, 2014
 * @author rus084
 */

#include "fuel_consumption_calculator.h"
#include "rpm_calculator.h"
#include "main.h"
#include "ec2.h"

extern engine_configuration_s *engineConfiguration;
static FuelConsumption ConsCalc ;

FuelConsumption::FuelConsumption() {
	fuelConsumption = 0;
	nowUs = 0;
	diff = 0;
	lastFuelEventTimeUs = 0;
}

void FuelConsumption::refreshConsumption(float fuel) {

   fuelConsumption = fuelConsumption + fuel / 60000 * engineConfiguration->injectorFlow;
   // cm3

}

float FuelConsumption::getConsumptionCurrLiterPerHour(void) {

   nowUs = getTimeNowUs();

   diff = nowUs - lastFuelEventTimeUs;

   lastFuelEventTimeUs = nowUs;

   // 1000000 - Us per second
   // 1000 cm3 per liter
   // 3600 seconds per hour

   float consumption = fuelConsumption * 3600 / ( diff / 1000000 ) / 1000 ;
   fuelConsumption = 0 ;
   return consumption;
}

float FuelConsumption::getFuelTrip(void) {
   return fuelConsumption * 1000;
}

void refreshConsumption(float fuel) {
  ConsCalc.refreshConsumption(fuel);
}

float getConsumptionCurrLiterPerHour(void) {
   return ConsCalc.getConsumptionCurrLiterPerHour();
}

float getFuelTrip(void) {
   return ConsCalc.getFuelTrip();
}
