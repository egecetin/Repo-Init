#pragma once

#include <atomic>

#include "Tracer.hpp"
#include "metrics/PrometheusServer.hpp"

/**
 * @brief Thread function to receive control messages or config changes from Telnet connections
 *
 * This function runs as a separate thread and is responsible for receiving control messages or config changes
 * from Telnet connections. It takes a pointer to the main Prometheus server, the Telnet port number, and a flag
 * for runtime check as input parameters.
 *
 * @param[in] mainPrometheusServer Pointer to the main Prometheus server
 * @param[in] telnetPort Port number to serve the Telnet server
 * @param[in] checkFlag Flag for runtime check
 */
void telnetControlThread(const std::unique_ptr<PrometheusServer> &mainPrometheusServer, uint16_t telnetPort,
						 const std::unique_ptr<std::atomic_flag> &checkFlag);

/**
 * @brief Thread function to receive control messages or config changes from ZMQ connection
 *
 * This function runs as a separate thread and is responsible for receiving control messages or config changes
 * from a ZeroMQ connection. It takes a pointer to the main Prometheus server, the server address, and a flag
 * for runtime check as input parameters.
 *
 * @param[in] mainPrometheusServer Pointer to the main Prometheus server
 * @param[in] serverAddr Address to serve the ZeroMQ server
 * @param[in] checkFlag Flag for runtime check
 */
void zmqControlThread(const std::unique_ptr<PrometheusServer> &mainPrometheusServer, const std::string &serverAddr,
					  const std::unique_ptr<std::atomic_flag> &checkFlag);

/**
 * @brief Thread function to handle crashpad
 *
 * This function runs as a separate thread and is responsible for handling crashpad. It takes the remote server address,
 * remote proxy address, crashdump executable path, annotations, attachment file paths, report file location, and a flag
 * for runtime check as input parameters.
 *
 * @param[in] remoteAddr Remote server address
 * @param[in] proxyAddr Remote proxy address
 * @param[in] exeDir Crashdump executable path
 * @param[in] annotations Annotations
 * @param[in] attachments Attachment file paths
 * @param[in] reportPath Minidump file location
 * @param[in] checkFlag Flag for runtime check
 */
void crashpadControlThread(const std::string &remoteAddr, const std::string &proxyAddr, const std::string &exeDir,
						   const std::map<std::string, std::string> &annotations,
						   const std::vector<base::FilePath> &attachments, const std::string &reportPath,
						   const std::unique_ptr<std::atomic_flag> &checkFlag);

/**
 * @brief Thread function to handle self metrics
 *
 * This function runs as a separate thread and is responsible for handling self metrics. It takes a pointer to the main
 * Prometheus server and a flag for runtime check as input parameters.
 *
 * @param mainPrometheusServer Pointer to the main Prometheus server
 * @param checkFlag Flag for runtime check
 */
void selfMonitorThread(const std::unique_ptr<PrometheusServer> &mainPrometheusServer,
					   const std::unique_ptr<std::atomic_flag> &checkFlag);
