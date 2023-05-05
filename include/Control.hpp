#pragma once

#include "metrics/PrometheusServer.hpp"

/**
 * @brief Thread function to receive control messages or config changes from Telnet connections
 * @param[in] mainPrometheusServer Pointer to prometheus server
 * @param[in] telnetPort Port number to serve Telnet server
 */
void telnetControlThread(const std::unique_ptr<PrometheusServer> &mainPrometheusServer, uint16_t telnetPort);

/**
 * @brief Thread function to receive control messages or config changes from ZMQ connection
 * @param[in] mainPrometheusServer Pointer to prometheus server
 * @param[in] serverAddr Address to serve ZeroMQ server
 */
void zmqControlThread(const std::unique_ptr<PrometheusServer> &mainPrometheusServer, const std::string &serverAddr);
