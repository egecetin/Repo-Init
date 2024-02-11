#pragma once

#include "client/crashpad_client.h"

/**
 * @brief Tracer class to handle operations of Crashpad
 */
class Tracer {
  private:
	std::string _serverPath;
	std::string _serverProxy;
	std::string _handlerPath;
	std::map<std::string, std::string> _annotations;
	std::vector<base::FilePath> _attachments;
	std::string _reportPath;
	std::unique_ptr<crashpad::CrashpadClient> _clientHandler;

	// Start crashpad handler
	void startHandler();
	// Checks the given process is running
	bool checkPidIsRunning(pid_t processId);
	// Checks the given socket is alive
	bool checkSocketIsRunning(int sockId);

	// Gets the executable path of application
	static inline std::string getSelfExecutableDir();

  public:
	/**
	 * @brief Construct a new Tracer object
	 * @param[in] serverPath Remote server address
	 * @param[in] serverProxy Remote server proxy
	 * @param[in] crashpadHandlerPath Path to crashpad_handler executable
	 * @param[in] annotations Annotation list
	 * @param[in] attachments Attachments to upload remote server
	 * @param[in] reportPath Path to where dump minidump files
	 */
	explicit Tracer(const std::string &serverPath = "", const std::string &serverProxy = "",
					const std::string &crashpadHandlerPath = "",
					const std::map<std::string, std::string> &annotations = {},
					const std::vector<base::FilePath> &attachments = {},
					const std::string &reportPath = "");

	/**
	 * @brief Checks the crashpad_handler still running
	 * @return true Running
	 * @return false Otherwise
	 */
	bool isRunning();	

	/**
	 * @brief Checks and restarts if the crashpad_handler is not running
	 */
	void restart();
};
