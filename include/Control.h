#pragma once

#include "Utils.h"

constexpr uint32_t LOG_LEVEL_ID = (('L') | ('O' << 8) | ('G' << 16) | ('L' << 24));

/**
 * @brief Thread function to receive control messages or config changes from ZMQ connection
 */
void controllerThread();
