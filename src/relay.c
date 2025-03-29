#include "relay.h"

#include "bsp.h"			/* Low-level hardware driver */
#include <stdbool.h>

enum {
    NOF_RELAYS = 2
};

static int relay_pins[NOF_RELAYS] = {
    GPIO_PIN_9,
    GPIO_PIN_8
};

static bool relay_states[NOF_RELAYS];

#define RELAY_PORT GPIOB
#define RELAY_GPIO_CLK_ENABLE()    	__HAL_RCC_GPIOB_CLK_ENABLE()

void relay_init() {
    RELAY_GPIO_CLK_ENABLE();
    for (int i = 0; i < sizeof(relay_pins)/sizeof(relay_pins[0]); i++) {
        int pin = relay_pins[i];
        GPIO_InitTypeDef gpio_init;
        gpio_init.Pin = pin;
        gpio_init.Mode = GPIO_MODE_OUTPUT_PP;   			/* Set to push-pull output */
        gpio_init.Pull = GPIO_NOPULL;                		/* Disable pull-up and pull-down resistors */
        gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;  			/* GPIO speed level */
        HAL_GPIO_Init(RELAY_PORT, &gpio_init);	
        HAL_GPIO_WritePin(RELAY_PORT, pin, GPIO_PIN_RESET); // Set initial state to LOW
        relay_states[i] = false;
    }
}

bool relay_switch(int relay, bool state) {
    if (relay < 0 || relay >= NOF_RELAYS) {
        return false; // Invalid relay number
    }
    if (state) {
        HAL_GPIO_WritePin(RELAY_PORT, relay_pins[relay], GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(RELAY_PORT, relay_pins[relay], GPIO_PIN_RESET);
    }
    relay_states[relay] = state;
    return true;    
}

bool relay_state(int relay) {
    if (relay < 0 || relay >= NOF_RELAYS) {
        return false; // Invalid relay number
    }
    return relay_states[relay];
}
