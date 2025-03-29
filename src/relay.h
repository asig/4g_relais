#pragma once

#include <stdbool.h>

void relay_init();

bool relay_switch(int relay, bool state);
bool relay_state(int relay);
