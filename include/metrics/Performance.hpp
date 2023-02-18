#pragma once

#include "metrics/MeanVar.hpp"

#include <queue>

/**
 * @brief Measures and calculates performance metrics
 */
class PerformanceTracker : private MeanVarTracker
{
  private:
	/// Set after startTimer to measure counter difference
	uint64_t lastTimeCtr;
	/// TSC clock frequency
	uint64_t tscHzInternal;

  public:
	/**
	 * @brief Construct a new Performance Tracker
	 * @param[in] reg Registry to prometheus
	 * @param[in] name Name of the metric
	 * @param[in] tscHz TSC clock frequency
	 * @param[in] winLen Window length for moving operations
	 * @param[in] id Optional ID to add to metric names
	 */
	PerformanceTracker(std::shared_ptr<prometheus::Registry> &reg, const std::string &name, const uint64_t tscHz,
					   const size_t winLen, const uint64_t id = 0);

	/**
	 * @brief Starts the chronometer
	 */
	void startTimer();

	/**
	 * @brief Ends the chronometer and updates internal statistics
	 * @return Result of the chronometer
	 */
	double endTimer();

	/**
	 * @brief Get the name of the metric
	 * @return std::string Metric name
	 */
	std::string getName() { return metricName; }
};
