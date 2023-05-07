#pragma once

#include <prometheus/registry.h>

/**
 * @brief Measures status counters for prometheus
 *
 */
class StatusTracker {
  private:
	/// Active number of events
	prometheus::Gauge *activeCtr;
	/// Total number of counters
	prometheus::Counter *totalCtr;
	/// Number of success
	prometheus::Counter *successCtr;
	/// Number of fail
	prometheus::Counter *failedCtr;

  public:
	/**
	 * @brief Construct a new Status Tracker
	 * @param[in] reg Registry to prometheus
	 * @param[in] name Name of the metric
	 * @param[in] metricID ID to append to metric names
	 */
	StatusTracker(const std::shared_ptr<prometheus::Registry> &reg, const std::string &name, uint64_t metricID = 0);

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
};
