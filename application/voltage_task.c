#include "voltage_task.h"
#include "bsp_adc.h"
#include "cmsis_os.h"

#define MA_WINDOW_SIZE 100
static float ma_window[MA_WINDOW_SIZE];

static volatile float battery_voltage;

osThreadId battery_voltage_task_id;

static void battery_voltage_task(void const *argument)
{
    // use internal 1.2V to calibrate
    init_vrefint_reciprocal();

    float sampled_voltage = sample_battery_voltage();

    // initialize moving average filter
    size_t ma_window_idx = 0;
    float ma_window_sum = sampled_voltage * MA_WINDOW_SIZE;
    for (size_t i = 0; i < MA_WINDOW_SIZE; i++)
    {
        ma_window[i] = sampled_voltage;
    }

    for (;;)
    {
        // sample
        sampled_voltage = sample_battery_voltage();

        // moving average
        ma_window_sum = ma_window_sum - ma_window[ma_window_idx] + sampled_voltage;
        ma_window[ma_window_idx] = sampled_voltage;
        ma_window_idx = (ma_window_idx + 1) % MA_WINDOW_SIZE;
        battery_voltage = ma_window_sum / MA_WINDOW_SIZE;

        osDelay(10);
    }
}

void init_battery_voltage_task(void)
{
    osThreadDef(VOLTAGE_TASK, battery_voltage_task, osPriorityNormal, 0, 512);
    battery_voltage_task_id = osThreadCreate(osThread(VOLTAGE_TASK), NULL);
}

float get_battery_voltage(void)
{
    return battery_voltage;
}
