#pragma once

#include <cstdint>

/**
 * @brief Thread function to receive control messages or config changes from Telnet connections
 * @param[in] telnetPort Port number to serve Telnet server
 */
void telnetControlThread(uint16_t telnetPort);

/**
 * @brief Thread function to receive control messages or config changes from ZMQ connection
 */
void zmqControlThread();
