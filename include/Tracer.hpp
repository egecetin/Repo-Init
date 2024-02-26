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

	/**
	 * @brief Start the crashpad handler process
	 */
	void startHandler();

	/**
	 * @brief Check if the given process ID is running
	 * @param[in] processId The process ID to check
	 * @return true if the process is running, false otherwise
	 */
	static bool checkPidIsRunning(pid_t processId);

	/**
	 * @brief Check if the given socket ID is running
	 * @param[in] sockId The socket ID to check
	 * @return true if the socket is running, false otherwise
	 */
	static bool checkSocketIsRunning(int sockId);

	/**
	 * @brief Get the executable directory of the current application
	 * @return The executable directory path
	 */
	static inline std::string getSelfExecutableDir();

  public:
	/**
	 * @brief Construct a new Tracer object
	 * @param[in] serverPath Remote server address
	 * @param[in] serverProxy Remote server proxy
	 * @param[in] crashpadHandlerPath Path to crashpad_handler executable
	 * @param[in] annotations Annotation list
	 * @param[in] attachments Attachments to upload to remote server
	 * @param[in] reportPath Path to where dump minidump files
	 */
	explicit Tracer(std::string serverPath = "", std::string serverProxy = "",
					const std::string &crashpadHandlerPath = "", std::map<std::string, std::string> annotations = {},
					std::vector<base::FilePath> attachments = {}, const std::string &reportPath = "");

	/**
	 * @brief Check if the crashpad_handler process is running
	 * @return true if the crashpad_handler is running, false otherwise
	 */
	bool isRunning();

	/**
	 * @brief Check and restart the crashpad_handler process if it is not running
	 */
	void restart();

	/**
	 * @brief Dump shared library information to a file
	 * 
	 * @param[in] filePath File path to dump the information
	 * @return true If the operation is successful
	 * @return false Otherwise
	 */
	static bool dumpSharedLibraryInfo(const std::string &filePath);
};
