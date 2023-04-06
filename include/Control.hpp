#pragma once

#include <cstdint>

constexpr uint32_t LOG_LEVEL_ID = (('L') | ('O' << 8) | ('G' << 16) | ('L' << 24));
constexpr uint32_t VERSION_INFO_ID = (('V') | ('E' << 8) | ('R' << 16) | ('I' << 24));
/* ################################################################################### */
/* ############################# MAKE MODIFICATIONS HERE ############################# */
/* ################################################################################### */

/* ################################################################################### */
/* ################################ END MODIFICATIONS ################################ */
/* ################################################################################### */

/**
 * @brief Thread function to receive control messages or config changes from Telnet connections
 */
void telnetControlThread();

/**
 * @brief Thread function to receive control messages or config changes from ZMQ connection
 */
void zmqControlThread();
