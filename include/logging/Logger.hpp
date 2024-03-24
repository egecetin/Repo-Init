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
	 * @param[in] lokiAddr Loki address
	 * @param[in] sentryAddr Sentry address
	 */
	MainLogger(const std::string &lokiAddr, const std::string &sentryAddr);

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
	std::shared_ptr<spdlog::logger> getLogger() const { return mainLogger; }

	/**
	 * @brief Deconstructs the main logger
	 */
	~MainLogger();
};
