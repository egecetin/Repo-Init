#pragma once

#include <prometheus/counter.h>
#include <prometheus/gauge.h>
#include <prometheus/registry.h>

/**
 * @brief Measures status counters for prometheus
 * 
 */
class StatusTracker
{
  private:
	/// Name of the metric
	std::string metricName;
	/// ID of the tracker
	uint64_t trackerID;

	/// Total number of counters
	prometheus::Counter *totalCtr;
	/// Number of success
	prometheus::Counter *successCtr;
	/// Number of fail
	prometheus::Counter *failedCtr;
	/// Active number of events
	prometheus::Gauge *activeCtr;

  public:
	/**
	 * @brief Construct a new Status Tracker
	 * @param[in] reg Registry to prometheus
	 * @param[in] name Name of the metric
	 * @param[in] id Optional ID to add to metric names
	 */
	StatusTracker(std::shared_ptr<prometheus::Registry> &reg, const std::string &name, const uint64_t id = 0);

	/**
	 * @brief Increment number of current events
	 */
	void incrementActive();

	/**
	 * @brief Increment number of success
	 */
	void incrementSuccess();

	/**
	 * @brief Increment number of fail
	 */
	void incrementFail();

	/**
	 * @brief Get the name of the metric
	 * @return std::string Metric name
	 */
	std::string getName() { return metricName; }
};