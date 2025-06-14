#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "hardware/watchdog.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"


#define MOTOR_A_PIN1 0
#define MOTOR_A_PIN2 1
#define MOTOR_B_PIN1 2
#define MOTOR_B_PIN2 3

// Store wrap values for each PWM slice
uint32_t pwm_wraps[8] = {0}; // RP2040 has 8 PWM slices

// Function to initialize PWM for a motor
void init_motor_pwm(uint gpio, uint32_t freq, float duty_cycle) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);

    // Calculate wrap value for desired frequency
    uint32_t clock = clock_get_hz(clk_sys);
    uint32_t wrap = clock / freq - 1;
    pwm_wraps[slice_num] = wrap;

    pwm_set_wrap(slice_num, wrap);
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(gpio), duty_cycle * wrap);
    pwm_set_enabled(slice_num, true);
}

// Function to initialize motors
void init_motors() {
    // Initialize PWM for each motor pin
    init_motor_pwm(MOTOR_A_PIN1, 1000, 0.5); // Motor A Pin 1
    init_motor_pwm(MOTOR_A_PIN2, 1000, 0.5); // Motor A Pin 2
}
// Function to set motor speed
void set_motor_speed(uint gpio, float speed) {
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    uint32_t wrap = pwm_wraps[slice_num];
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(gpio), speed * wrap);
}


// Function to stop motors
void stop_motors() {
    set_motor_speed(MOTOR_A_PIN1, 0);
    set_motor_speed(MOTOR_A_PIN2, 0);
    set_motor_speed(MOTOR_B_PIN1, 0);
    set_motor_speed(MOTOR_B_PIN2, 0);
}

// Function to move motors forward
void move_forward(float speed) {
    set_motor_speed(MOTOR_A_PIN1, speed);
    set_motor_speed(MOTOR_A_PIN2, 0);
    set_motor_speed(MOTOR_B_PIN1, speed);
    set_motor_speed(MOTOR_B_PIN2, 0);
}

// Function to move motors backward
void move_backward(float speed) {
    set_motor_speed(MOTOR_A_PIN1, 0);
    set_motor_speed(MOTOR_A_PIN2, speed);
    set_motor_speed(MOTOR_B_PIN1, 0);
    set_motor_speed(MOTOR_B_PIN2, speed);
}

// Function to turn motors left
void turn_left(float speed) {
    set_motor_speed(MOTOR_A_PIN1, 0);
    set_motor_speed(MOTOR_A_PIN2, speed);
    set_motor_speed(MOTOR_B_PIN1, speed);
    set_motor_speed(MOTOR_B_PIN2, 0);
}

// Function to turn motors right
void turn_right(float speed) {
    set_motor_speed(MOTOR_A_PIN1, speed);
    set_motor_speed(MOTOR_A_PIN2, 0);
    set_motor_speed(MOTOR_B_PIN1, 0);
    set_motor_speed(MOTOR_B_PIN2, speed);
}

// Function to initialize the system
void init_system() {
    stdio_init_all();
    init_motors();
}


int main()
{

    init_system();

    // Watchdog example code
    if (watchdog_caused_reboot()) {
        printf("Rebooted by Watchdog!\n");
        // Whatever action you may take if a watchdog caused a reboot
    }
    
    // Enable the watchdog, requiring the watchdog to be updated every 100ms or the chip will reboot
    // second arg is pause on debug which means the watchdog will pause when stepping through code
    watchdog_enable(100, 1);
    

    printf("System Clock Frequency is %d Hz\n", clock_get_hz(clk_sys));
    printf("USB Clock Frequency is %d Hz\n", clock_get_hz(clk_usb));
    // For more examples of clocks use see https://github.com/raspberrypi/pico-examples/tree/master/clocks

    while (true) {
        printf("Hello, world!\n");
        move_forward(0.5); // Move forward at 50% speed
        sleep_ms(1000);
        move_backward(0.5); // Move backward at 50% speed
        sleep_ms(1000);
        turn_left(0.5); // Turn left at 50% speed
        sleep_ms(1000);
        turn_right(0.5); // Turn right at 50% speed
        sleep_ms(1000);
        stop_motors(); // Stop motors
        sleep_ms(1000);
        
        // Update watchdog timer
        watchdog_update();
    }
}
