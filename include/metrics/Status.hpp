#pragma once

#include <prometheus/registry.h>

/**
 * @class StatusTracker
 * @brief Class for tracking the status of events and counting successes and failures.
 */
class StatusTracker {
  private:
	prometheus::Gauge *activeCtr;	 ///< Active number of events
	prometheus::Counter *totalCtr;	 ///< Total number of counters
	prometheus::Counter *successCtr; ///< Number of success
	prometheus::Counter *failedCtr;	 ///< Number of fail

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
	 * @brief Decrement number of current events
	 */
	void decrementActive();

	/**
	 * @brief Increment number of success
	 */
	void incrementSuccess();

	/**
	 * @brief Increment number of fail
	 */
	void incrementFail();
};

/**
 * @brief RAII style wrapper for StatusTracker
 */
class TrackStatus {
  private:
	StatusTracker &_tracker;

  public:
	explicit TrackStatus(StatusTracker &tracker) : _tracker(tracker) { _tracker.incrementActive(); }
	~TrackStatus() { _tracker.decrementActive(); }

	/// @brief Copy constructor
	TrackStatus(const TrackStatus & /*unused*/) = delete;

	/// @brief Move constructor
	TrackStatus(TrackStatus && /*unused*/) = delete;

	/// @brief Copy assignment operator
	TrackStatus &operator=(TrackStatus /*unused*/) = delete;

	/// @brief Move assignment operator
	TrackStatus &operator=(TrackStatus && /*unused*/) = delete;
};
