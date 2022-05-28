#pragma once

#include "Utils.h"

#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <spdlog/spdlog.h>

#define LOG_LEVEL_ID 1
#define ENABLE_MIRROR_ID 2
#define DISABLE_MIRROR_ID 3

/**
 * @brief Thread function to receive control messages or config changes from ZMQ connection
 */
void controllerThread();
