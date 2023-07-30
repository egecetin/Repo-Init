#pragma once

#include <prometheus/registry.h>

/**
 * @brief Measures and calculates performance metrics
 */
class PerformanceTracker {
  private:
	/// Set after startTimer to measure counter difference
	std::chrono::high_resolution_clock::time_point startTime;

	/// Overall performance
	prometheus::Summary *perfTiming;
	/// Maximum observed value
	prometheus::Gauge *maxTiming;
	/// Minimum observed value
	prometheus::Gauge *minTiming;

  public:
	/**
	 * @brief Construct a new Performance Tracker
	 * @param[in] reg Registry to prometheus
	 * @param[in] name Name of the metric
	 * @param[in] metricID ID to append to metric names
	 */
	PerformanceTracker(const std::shared_ptr<prometheus::Registry> &reg, const std::string &name,
					   uint64_t metricID = 0);

	/**
	 * @brief Starts the chronometer
	 */
	void startTimer();

	/**
	 * @brief Ends the chronometer and updates internal statistics
	 * @return Result of the chronometer in nanoseconds
	 */
	double endTimer();
};

/**
 * @brief RAII style wrapper for PerformanceTracker
 */
class TrackPerformance {
  private:
	PerformanceTracker &_tracker;

  public:
	explicit TrackPerformance(PerformanceTracker &tracker) : _tracker(tracker) { _tracker.startTimer(); }
	~TrackPerformance() { _tracker.endTimer(); }
};
