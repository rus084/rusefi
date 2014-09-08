/**
 * @file	engine_math.h
 *
 * @date Jul 13, 2013
 * @author Andrey Belomutskiy, (c) 2012-2014
 */

#ifndef ENGINE_MATH_H_
#define ENGINE_MATH_H_

#include "engine_configuration.h"

#ifdef __cplusplus
#include "ec2.h"
#include "trigger_structure.h"
#include "table_helper.h"
#include "engine.h"

extern engine_configuration_s *engineConfiguration;
extern board_configuration_s *boardConfiguration;

void findTriggerPosition(engine_configuration_s const *engineConfiguration, trigger_shape_s * s,
		event_trigger_position_s *position, float angleOffset);

int isInjectionEnabled(engine_configuration_s *engineConfiguration);

float fixAngle(float angle);

#endif

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

float getDefaultFuel(int rpm, float map);

float getOneDegreeTimeMs(int rpm);
float getOneDegreeTimeUs(int rpm);
float getCrankshaftRevolutionTimeMs(int rpm);

int isCrankingRT(engine_configuration_s *engineConfiguration, int rpm);
#define isCrankingR(rpm) isCrankingRT(engineConfiguration, rpm)

float getEngineLoadT(Engine *engine);
#define getEngineLoad() getEngineLoadT(&engine)

float getSparkDwellMsT(engine_configuration_s *engineConfiguration, int rpm);
#define getSparkDwellMs(rpm) getSparkDwellMsT(engineConfiguration, rpm)

int getCylinderId(firing_order_e firingOrder, int index);

void setFuelRpmBin(engine_configuration_s *engineConfiguration, float l, float r);
void setFuelLoadBin(engine_configuration_s *engineConfiguration, float l, float r);
void setTimingRpmBin(engine_configuration_s *engineConfiguration, float l, float r);
void setTimingLoadBin(engine_configuration_s *engineConfiguration, float l, float r);

void setSingleCoilDwell(engine_configuration_s *engineConfiguration);

int engineNeedSkipStokeT(engine_configuration_s *engineConfiguration);
#define engineNeedSkipStoke() engineNeedSkipStokeT(engineConfiguration)


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ENGINE_MATH_H_ */
