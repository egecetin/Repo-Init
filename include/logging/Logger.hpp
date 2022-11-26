#pragma once

/**
 * @brief Main logger class
 */
class MainLogger
{
  public:
	/**
	 * @brief Constructs and prepares main logger
	 * @param[in] argc Number of application arguments (Untouched internally)
	 * @param[in] argv Application arguments (Untouched internally)
	 */
	MainLogger(int argc, char **argv);

	/**
	 * @brief Deconstructs the main logger
	 */
	~MainLogger();
};
