#pragma once

#include "Utils.h"
#include "telnet/TelnetServer.h"

#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <spdlog/spdlog.h>

constexpr uint32_t LOG_LEVEL_ID = (('L') | ('O' << 8) | ('G' << 16) | ('L' << 24));

/**
 * @brief Thread function to receive control messages or config changes from ZMQ connection
 */
void controllerThread();
