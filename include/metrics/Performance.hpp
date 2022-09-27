#pragma once

#include <prometheus/counter.h>
#include <prometheus/gauge.h>
#include <prometheus/registry.h>

/**
 * @brief Measures and calculates performance metrics
 */
class PerformanceTracker
{
  private:
	/// Name of the metric
	std::string metricName;
	/// ID of the tracker
	uint64_t trackerID;
	/// Set after startTimer to measure counter difference
	uint64_t lastTimeCtr;
	/// TSC clock frequency
	uint64_t tsc_hz_internal;

	/// Total counter of start/stop period
	prometheus::Counter *eventCtr;
	/// Mean value of timer in nanoseconds
	prometheus::Gauge *meanTiming;
	/// Variance of timer in nanoseconds
	prometheus::Gauge *varTiming;
	/// Maximum value of timer in nanoseconds
	prometheus::Gauge *maxTiming;
	/// Minimum value of timer in nanoseconds
	prometheus::Gauge *minTiming;

	/// Standard deviation buffer (Internal use only)
	double stdBuffTiming;

	/**
	 * @brief Update statistics with provided value
	 * @param[in] newValue New performance timing
	 */
	void updateStatistic(double newValue);

  public:
	/**
	 * @brief Construct a new Performance Tracker
	 * @param[in] reg Registry to prometheus
	 * @param[in] name Name of the metric
	 * @param[in] tsc_hz TSC clock frequency
	 * @param[in] id Optional ID to add to metric names
	 */
	PerformanceTracker(std::shared_ptr<prometheus::Registry> &reg, const std::string &name, const uint64_t tsc_hz,
					   const uint64_t id = 0);

	/**
	 * @brief Starts the chronometer
	 */
	void startTimer();

	/**
	 * @brief Ends the chronometer and updates internal statistics
	 */
	void endTimer();

	/**
	 * @brief Get the name of the metric
	 * @return std::string Metric name
	 */
	std::string getName() { return metricName; }
};
