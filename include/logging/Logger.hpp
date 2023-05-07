#pragma once

#include <spdlog/spdlog.h>

/**
 * @brief Main logger class
 */
class MainLogger {
  private:
	std::shared_ptr<spdlog::logger> mainLogger;

  public:
	/**
	 * @brief Constructs and prepares main logger
	 * @param[in] argc Number of application arguments (Untouched internally)
	 * @param[in] argv Application arguments (Untouched internally)
	 * @param[in] configPath Path to the config json
	 */
	MainLogger(int argc, char **argv, const std::string &configPath);

	/// @brief Copy constructor
	MainLogger(const MainLogger & /*unused*/) = delete;

	/// @brief Move constructor
	MainLogger(MainLogger && /*unused*/) = delete;

	/// @brief Copy assignment operator
	MainLogger &operator=(MainLogger /*unused*/) = delete;

	/// @brief Move assignment operator
	MainLogger &operator=(MainLogger && /*unused*/) = delete;

	/**
	 * @brief Returns pointer to mainlogger instance
	 * @return std::shared_ptr<spdlog::logger> Main logger
	 */
	std::shared_ptr<spdlog::logger> getLogger() { return mainLogger; }

	/**
	 * @brief Deconstructs the main logger
	 */
	~MainLogger();
};
