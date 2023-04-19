#pragma once

/**
 * @brief Thread function to receive control messages or config changes from Telnet connections
 */
void telnetControlThread();

/**
 * @brief Thread function to receive control messages or config changes from ZMQ connection
 */
void zmqControlThread();
