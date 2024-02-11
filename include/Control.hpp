#pragma once

#include <atomic>

#include "metrics/PrometheusServer.hpp"

/**
 * @brief Thread function to receive control messages or config changes from Telnet connections
 * @param[in] mainPrometheusServer Pointer to prometheus server
 * @param[in] telnetPort Port number to serve Telnet server
 * @param[in] checkFlag Flag for runtime check
 */
void telnetControlThread(const std::unique_ptr<PrometheusServer> &mainPrometheusServer, uint16_t telnetPort,
						 const std::unique_ptr<std::atomic_flag> &checkFlag);

/**
 * @brief Thread function to receive control messages or config changes from ZMQ connection
 * @param[in] mainPrometheusServer Pointer to prometheus server
 * @param[in] serverAddr Address to serve ZeroMQ server
 * @param[in] checkFlag Flag for runtime check
 */
void zmqControlThread(const std::unique_ptr<PrometheusServer> &mainPrometheusServer, const std::string &serverAddr,
					  const std::unique_ptr<std::atomic_flag> &checkFlag);

/**
 * @brief Thread function to handle crashpad
 * @param[in] remoteAddr Remote server address
 * @param[in] proxyAddr Remote proxy address
 * @param[in] exeDir Crashdump executable path
 * @param[in] annotations Annotations
 * @param[in] reportPath Minidump file location
 * @param[in] checkFlag Flag for runtime check
 */
void crashpadControlThread(const std::string &remoteAddr, const std::string &proxyAddr, const std::string &exeDir,
						   const std::map<std::string, std::string> &annotations, const std::string &reportPath,
						   const std::unique_ptr<std::atomic_flag> &checkFlag);
