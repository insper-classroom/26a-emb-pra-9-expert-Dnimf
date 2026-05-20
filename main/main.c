#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/uart.h"

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "mpu6050.h"
#include "hardware/pwm.h"

#include "Fusion.h"
#define SAMPLE_PERIOD (0.01f) // replace this with actual sample period

#define UART_ID uart0
#define BAUD_RATE 115200

QueueHandle_t xQueueMPU;
QueueHandle_t xQueuePos;
QueueHandle_t xQueueColor;
SemaphoreHandle_t xSemaphoreBtn;
const int MPU_ADDRESS = 0x68;
const int I2C_SDA_GPIO = 4;
const int I2C_SCL_GPIO = 5;

const int LED_R_PIN = 13;
const int LED_G_PIN = 12;
const int LED_B_PIN = 11;

const int MPU_PIN = 16;
const int FUSION_PIN = 17;
const int UART_PIN = 18;
const int PWM_PIN = 19;


typedef struct {
    /* data */
    int accel[3];
    int gyro[3];
    double temporario;
} dados;
typedef struct {
    int roll;
    int yaw;
} angulos;
typedef struct {
    int color;
    int proporcao;
} cor;
static void mpu6050_init() {
    i2c_init(i2c_default, 400 * 1000);
    gpio_set_function(I2C_SDA_GPIO, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_GPIO, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_GPIO);
    gpio_pull_up(I2C_SCL_GPIO);

    // Two byte reset. First byte register, second byte data
    // There are a load more options to set up the device in different ways that could be added here
    uint8_t buf[] = {0x6B, 0x00};
    i2c_write_blocking(i2c_default, MPU_ADDRESS, buf, 2, false);
}

static void mpu6050_read_raw(int16_t accel[3], int16_t gyro[3], int16_t *temp) {
    uint8_t buffer[14];

    // Read all data sequentially starting from acceleration registers (0x3B)
    // 0x3B-0x40: acceleration (6 bytes)
    // 0x41-0x42: temperature (2 bytes)
    // 0x43-0x48: gyro (6 bytes)
    uint8_t val = 0x3B;
    i2c_write_blocking(i2c_default, MPU_ADDRESS, &val, 1, true);
    i2c_read_blocking(i2c_default, MPU_ADDRESS, buffer, 14, false);

    // Parse acceleration
    for (int i = 0; i < 3; i++) {
        accel[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);
    }

    // Parse temperature
    *temp = buffer[6] << 8 | buffer[7];

    // Parse gyro
    for (int i = 0; i < 3; i++) {
        gyro[i] = (buffer[8 + i * 2] << 8 | buffer[8 + (i * 2) + 1]);
    }
    vTaskDelay(pdMS_TO_TICKS(20));
}

void mpu6050_task(void *p) {
    mpu6050_init();

    while (1) {
        int16_t acceleration[3], gyro[3], temp;
        gpio_put(MPU_PIN, 1);
        mpu6050_read_raw(acceleration, gyro, &temp);

        // 100 Hz
        dados dads;
        dads.accel[0] = acceleration[0];
        dads.accel[1] = acceleration[1];
        dads.accel[2] = acceleration[2];
        dads.gyro[0] = gyro[0];
        dads.gyro[1] = gyro[1];
        dads.gyro[2] = gyro[2];
        dads.temporario = (temp / 340.0) + 36.53;
        xQueueSend(xQueueMPU, &dads, 0);
        gpio_put(MPU_PIN, 0);

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
void fusion_task(void *p) {
    // int16_t acceleration[3], gyro[3], temp;
    dados dados1;
    FusionAhrs ahrs;
    FusionAhrsInitialise(&ahrs);
    while (true) {
        gpio_put(FUSION_PIN,1);
        if (xQueueReceive(xQueueMPU, &dados1, 0)) {
            FusionVector gyroscope = {
                .axis.x = dados1.gyro[0] / 131.0f, // Conversão para graus/s
                .axis.y = dados1.gyro[1] / 131.0f,
                .axis.z = dados1.gyro[2] / 131.0f,
            };

            FusionVector accelerometer = {
                .axis.x = dados1.accel[0] / 16384.0f, // Conversão para g
                .axis.y = dados1.accel[1] / 16384.0f,
                .axis.z = dados1.accel[2] / 16384.0f,
            };

            FusionAhrsUpdateNoMagnetometer(&ahrs, gyroscope, accelerometer, SAMPLE_PERIOD);
            const FusionEuler euler = FusionQuaternionToEuler(FusionAhrsGetQuaternion(&ahrs));
            if ((dados1.accel[1] / 16384.0f) < -1.5) {
                xSemaphoreGive(xSemaphoreBtn);
            }
            float roll_0 = euler.angle.roll;
            if (roll_0 > 180.0f) {
                roll_0 = 180.0f;
            }
            if (roll_0 < -180.0f) {
                roll_0 = -180.0f;
            }
            if (roll_0 < 3.0f && roll_0 > -3.0f) {
                roll_0 = 0.0f;
            }
            float yaw_0 = euler.angle.yaw;
            if (yaw_0 > 180.0f) {
                yaw_0 = 180.0f;
            }
            if (yaw_0 < -180.0f) {
                yaw_0 = -180.0f;
            }
            if (yaw_0 < 3.0f && yaw_0 > -3.0f) {
                yaw_0 = 0.0f;
            }
            float p_0 = euler.angle.pitch;
            if (p_0 > 180.0f) {
                p_0 = 180.0f;
            }
            if (p_0 < -180.0f) {
                p_0 = -180.0f;
            }

            int roll = (roll_0 / 180.0f) * 255.0f;
            int yaw = -1 * (yaw_0 / 180.0f) * 255.0f;
            int pitch = (p_0 / 180.0f) * 100.0f;

            angulos ang;
            ang.roll = roll;
            ang.yaw = yaw;
            xQueueOverwrite(xQueuePos, &ang);

            cor color;

            int roll_cor = -1 * (roll_0 / 180.0f) * 100.0f;

            if (roll_cor < 0) {
                roll_cor = 0;
            }
            if (roll_0 > 100) {
                roll_cor = 100;
            }
            color.color = 2;
            color.proporcao = roll_cor;
            xQueueOverwrite(xQueueColor, &color);
            vTaskDelay(pdMS_TO_TICKS(10));

            if (pitch > 0) {
                color.color = 3;
                color.proporcao = pitch;
            } else if (pitch < 0) {
                color.color = 1;
                color.proporcao = -1 * pitch;
            }
            xQueueOverwrite(xQueueColor, &color);
        }
        gpio_put(FUSION_PIN,0);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
void pwm_task(void *p) {
    gpio_set_function(LED_R_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(LED_R_PIN);
    pwm_set_clkdiv(slice_num, 125);
    pwm_set_wrap(slice_num, 100);
    pwm_set_chan_level(slice_num, PWM_CHAN_B, 2);
    pwm_set_enabled(slice_num, true);

    gpio_set_function(LED_B_PIN, GPIO_FUNC_PWM);
    uint slice_num_1 = pwm_gpio_to_slice_num(LED_B_PIN);
    pwm_set_clkdiv(slice_num_1, 125);
    pwm_set_wrap(slice_num_1, 100);
    pwm_set_chan_level(slice_num_1, PWM_CHAN_B, 2);
    pwm_set_enabled(slice_num_1, true);

    gpio_set_function(LED_G_PIN, GPIO_FUNC_PWM);
    uint slice_num_2 = pwm_gpio_to_slice_num(LED_G_PIN);
    pwm_set_clkdiv(slice_num_2, 125);
    pwm_set_wrap(slice_num_2, 100);
    pwm_set_chan_level(slice_num_2, PWM_CHAN_A, 2);
    pwm_set_enabled(slice_num_2, true);
    while (true) {
        gpio_put(PWM_PIN,1);
        cor corzinha;
        if (xQueueReceive(xQueueColor, &corzinha, 0)) {
            if (corzinha.color == 1) {
                pwm_set_chan_level(slice_num, PWM_CHAN_B, corzinha.proporcao);
                pwm_set_enabled(slice_num, true);
            }
            if (corzinha.color == 2) {
                pwm_set_chan_level(slice_num_2, PWM_CHAN_A, corzinha.proporcao);
                pwm_set_enabled(slice_num_2, true);
            }
            if (corzinha.color == 3) {
                pwm_set_chan_level(slice_num_1, PWM_CHAN_B, corzinha.proporcao);
                pwm_set_enabled(slice_num_1, true);
            }
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        gpio_put(PWM_PIN,0);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void uart_task(void *p) {

    while (1) {
        gpio_put(UART_PIN,1);
        angulos ang;
        if (xSemaphoreTake(xSemaphoreBtn, pdMS_TO_TICKS(20)) == pdTRUE) {
            uart_putc(UART_ID, 2);
            uart_putc(UART_ID, 1);
            uart_putc(UART_ID, 0);
            uart_putc(UART_ID, -1);
            vTaskDelay(pdMS_TO_TICKS(50));
            uart_putc(UART_ID, 2);
            uart_putc(UART_ID, 0);
            uart_putc(UART_ID, 0);
            uart_putc(UART_ID, -1);
        }
        if (xQueueReceive(xQueuePos, &ang, pdMS_TO_TICKS(50))) {

            uart_putc(UART_ID, 1);
            uart_putc(UART_ID, ang.roll);
            uart_putc(UART_ID, (ang.roll >> 8));
            uart_putc(UART_ID, -1);
            vTaskDelay(pdMS_TO_TICKS(10));

            uart_putc(UART_ID, 0);
            uart_putc(UART_ID, ang.yaw);
            uart_putc(UART_ID, (ang.yaw >> 8));
            uart_putc(UART_ID, -1);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        gpio_put(UART_PIN,0);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void stack_monitor_task(void* p) {
    static TaskStatus_t tasks[16];
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(2000));
        UBaseType_t n = uxTaskGetSystemState(tasks, 16, NULL);
        printf("+------------------+-------+\n");
        printf("| %-16s | %5s |\n", "task", "free");
        printf("+------------------+-------+\n");
        for (UBaseType_t i = 0; i < n; i++) {
            printf("| %-16s | %5u |\n",
                   tasks[i].pcTaskName,
                   (unsigned)tasks[i].usStackHighWaterMark);
        }
        printf("+------------------+-------+\n");
        printf("| heap livre min   | %5u |\n",
               (unsigned)xPortGetMinimumEverFreeHeapSize());
        printf("+------------------+-------+\n\n");
    }
}
int main() {
    stdio_init_all();
    gpio_init(MPU_PIN);
    gpio_init(FUSION_PIN);
    gpio_init(PWM_PIN);
    gpio_init(UART_PIN);
    gpio_set_dir(MPU_PIN, GPIO_OUT);
    gpio_set_dir(FUSION_PIN, GPIO_OUT);
    gpio_set_dir(UART_PIN, GPIO_OUT);
    gpio_set_dir(PWM_PIN, GPIO_OUT);
    xQueueMPU = xQueueCreate(32, sizeof(dados));
    xQueuePos = xQueueCreate(1, sizeof(angulos));
    xQueueColor = xQueueCreate(1, sizeof(cor));
    xSemaphoreBtn = xSemaphoreCreateBinary();
    xTaskCreate(mpu6050_task, "mpu6050_Task 1", 8192, NULL, 1, NULL);
    xTaskCreate(fusion_task, "fusion 1", 4096, NULL, 1, NULL);
    xTaskCreate(uart_task, "uart 1", 4096, NULL, 1, NULL);
    xTaskCreate(pwm_task, "pwm_task 1", 4096, NULL, 1, NULL);
    xTaskCreate(stack_monitor_task, "stak 1", 4096, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
