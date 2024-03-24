#pragma once

#include <spdlog/spdlog.h>

/**
 * Main logger class
 */
class MainLogger {
  private:
	std::shared_ptr<spdlog::logger> mainLogger;

  public:
	/**
	 * Constructs and prepares main logger
	 * @param[in] lokiAddr Loki address
	 * @param[in] sentryAddr Sentry address
	 */
	MainLogger(const std::string &lokiAddr, const std::string &sentryAddr);

	/// Copy constructor
	MainLogger(const MainLogger & /*unused*/) = delete;

	/// Move constructor
	MainLogger(MainLogger && /*unused*/) = delete;

	/// Copy assignment operator
	MainLogger &operator=(MainLogger /*unused*/) = delete;

	/// Move assignment operator
	MainLogger &operator=(MainLogger && /*unused*/) = delete;

	/**
	 * Returns pointer to mainlogger instance
	 * @return std::shared_ptr<spdlog::logger> Main logger
	 */
	std::shared_ptr<spdlog::logger> getLogger() const { return mainLogger; }

	/**
	 * Deconstructs the main logger
	 */
	~MainLogger();
};
