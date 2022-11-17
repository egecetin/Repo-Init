#pragma once

#include <queue>

#include <prometheus/counter.h>
#include <prometheus/gauge.h>
#include <prometheus/info.h>
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
	uint64_t tscHzInternal;
	/// Window length of moving operations
	size_t winLenInternal;

	/// Total counter of start/stop period
	prometheus::Counter *eventCtr;
	/// Mean value of timer in nanoseconds
	prometheus::Gauge *meanTiming;
	/// Variance of timer in nanoseconds
	prometheus::Gauge *varTiming;
	/// Moving mean value of timer in nanoseconds
	prometheus::Gauge *movingMeanTiming;
	/// Variance of timer in nanoseconds
	prometheus::Gauge *movingVarTiming;
	/// Maximum value of timer in nanoseconds
	prometheus::Gauge *maxTiming;
	/// Minimum value of timer in nanoseconds
	prometheus::Gauge *minTiming;
	/// Window length for moving statistical operations
	prometheus::Info *windowLength;

	/// Standard deviation buffer (Internal use only)
	double stdBuffTiming;
	/// Moving standard deviation buffer (Internal use only)
	double movStdBuffTiming;

	/// Element queue for moving operations
	std::queue<double> qMeasurements;

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
	 * @param[in] tscHz TSC clock frequency
	 * @param[in] winLen Window length for moving operations
	 * @param[in] id Optional ID to add to metric names
	 */
	PerformanceTracker(std::shared_ptr<prometheus::Registry> &reg, const std::string &name, const uint64_t tscHz, const size_t winLen,
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
