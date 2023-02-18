#pragma once

#include <queue>

#include <prometheus/counter.h>
#include <prometheus/gauge.h>
#include <prometheus/info.h>
#include <prometheus/registry.h>

/**
 * @brief Measures and calculates metrics for given values
 */
class MeanVarTracker
{
  protected:
	/// Name of the metric
	std::string metricName;
	/// ID of the tracker
	uint64_t trackerID;
	/// Window length of moving operations
	size_t winLenInternal;

	/// Total counter of start/stop period
	prometheus::Counter *eventCtr;
	/// Mean value of provided values
	prometheus::Gauge *meanVal;
	/// Variance of provided values
	prometheus::Gauge *varVal;
	/// Moving mean value of provided values
	prometheus::Gauge *movingMeanVal;
	/// Variance of provided values
	prometheus::Gauge *movingVarVal;
	/// Maximum value of provided values
	prometheus::Gauge *maxVal;
	/// Minimum value of provided values
	prometheus::Gauge *minVal;
	/// Window length for moving statistical operations
	prometheus::Info *windowLength;

	/// Standard deviation buffer (Internal use only)
	double stdBuffVal;
	/// Moving standard deviation buffer (Internal use only)
	double movStdBuffVal;

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
	 * @param[in] winLen Window length for moving operations
	 * @param[in] id Optional ID to add to metric names
	 */
	MeanVarTracker(std::shared_ptr<prometheus::Registry> &reg, const std::string &name,
					   const size_t winLen, const uint64_t id = 0);

	/**
	 * @brief Updates internal statistics with value
	 * @param[in] val Value
	 */
	void updateValue(double val);

	/**
	 * @brief Get the name of the metric
	 * @return std::string Metric name
	 */
	std::string getName() { return metricName; }
};
