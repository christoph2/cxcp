/*
 *
 * Example roughly based on Modeling Show Cases Report AUTOSAR CP Release 4.3.1.
 *
 * https://www.autosar.org/fileadmin/user_upload/standards/classic/4-3/AUTOSAR_TR_ModelingShowCases.pdf
 *
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "app_config.h"

#define ABS_ZERO (-273.15)

/*
 *
 * Calibration Parameters.
 *
 */
float CAL_PARAM prmLimitLow     = 65.0;
float CAL_PARAM prmLimitHigh    = 85.0;
float CAL_PARAM prmStepSize     = 1.5;
float CAL_PARAM prmEnvFactor    = 0.5;
float CAL_PARAM prmHeaterFactor = 0.5;
float CAL_PARAM prmDt           = 0.5;
float CAL_PARAM prmSetpoint     = 70.0;
float CAL_PARAM prmKi           = 2.5;
float CAL_PARAM prmMaxErrorSum  = 100.0;

/*
 *
 * Random selection of [-1, 0 , 1].
 *
 */
int8_t randomSlope(void) {
    return (rand() % 3) - 1;
}

/*
 *
 *  Random walking temperature within boundaries.
 *
 */
double environment(void) {
    static double t_prev = ABS_ZERO;

    t_prev = t_prev + (prmStepSize * randomSlope());
    if (t_prev < prmLimitLow) {
        t_prev = prmLimitLow;
    }
    if (t_prev > prmLimitHigh) {
        t_prev = prmLimitHigh;
    }
    return t_prev;
}

/*
 *
 * Electrically heated mass that is exposed to the air flow in the environment.
 *
 */
double plant(double temperature, double current) {
    static double q_plant  = 0.0;
    double        q_heater = 0.0;
    double        q_env    = 0.0;
    double        t_plant  = 0.0;

    t_plant  = q_plant;
    q_heater = current * prmHeaterFactor * prmDt;
    q_env    = (temperature - t_plant) * prmEnvFactor * prmDt;
    q_plant  = q_plant + q_heater + q_env;
    q_plant  = q_plant < 0.0 ? 0.0 : q_plant;
    t_plant  = q_plant;

    return t_plant;
}

/*
 *
 * I-Controller.
 *
 */
double controller(double temperature) {
    double        current       = 0.0;
    double        error         = 0.0;
    static double sum_of_errors = 0.0;

    error = prmSetpoint - temperature;
    sum_of_errors += error * prmDt;
    if (sum_of_errors > prmMaxErrorSum) {
        sum_of_errors = prmMaxErrorSum;
    }
    if (sum_of_errors < 0.0) {
        sum_of_errors = 0.0;
    }
    current = sum_of_errors * prmKi;
    return current;
}

void AppTask(void) {
    double        env_temp = 0;
    double        plant_temp;
    static double current = 0.0;

    env_temp   = environment();
    plant_temp = plant(env_temp, current);
    current    = controller(plant_temp);
    printf("T_E: %3.3f T_P: %3.3f I: %3.3f\n", env_temp, plant_temp, current);
}
