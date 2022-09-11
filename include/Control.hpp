#pragma once

#include "Utils.h"
#include "telnet/TelnetServer.h"

constexpr uint32_t LOG_LEVEL_ID = (('L') | ('O' << 8) | ('G' << 16) | ('L' << 24));

/**
 * @brief Telnet session connection start callback
 * @param[in] session Handle to session
 */
void TelnetConnectedCallback(SP_TelnetSession session);

/**
 * @brief Telnet session message received callback
 * @param[in] session Handle to session
 * @param[in] line Received message
 */
bool TelnetMessageCallback(SP_TelnetSession session, std::string line);

/**
 * @brief Telnet session TAB received callback
 * @param[in] session Handle to session
 * @param[in] line Received message
 * @return std::string Command to complete
 */
std::string TelnetTabCallback(SP_TelnetSession session, std::string line);

/**
 * @brief Thread function to receive control messages or config changes from Telnet connections
 */
void telnetControlThread();

/**
 * @brief Thread function to receive control messages or config changes from ZMQ connection
 */
void zmqControlThread();
