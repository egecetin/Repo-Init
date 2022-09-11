#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <prometheus/counter.h>
#include <prometheus/exposer.h>
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

/**
 * @brief Wrapper class for Prometheus
 *
 */
class Reporter
{
  private:
	/// Main HTTP Server
	std::unique_ptr<prometheus::Exposer> mainExposer;
	/// All registered trackers
	std::vector<std::shared_ptr<prometheus::Registry>> vRegister;
	/// All performance trackers
	std::vector<std::shared_ptr<PerformanceTracker>> vPerfTracker;
	/// All status trackers
	std::vector<std::shared_ptr<StatusTracker>> vStatTracker;

	/// Mutex for concurrent add tracker calls
	std::mutex guardLock;
	/// TSC frequency
	uint64_t tsc_hz;
	/// Initialize time
	prometheus::Info *initTime;

  public:
	/**
	 * @brief Construct a new Performance Reporter
	 * @param[in] serverAddr Server address in <IP>:<Port> format
	 */
	Reporter(const std::string &serverAddr);

	/**
	 * @brief Adds a new Performance tracker to prometheus service
	 * @param[in] name Name of the metric
	 * @param[in] id Optional ID to add metric names
	 * @return std::shared_ptr<PerformanceTracker> Pointer to new performance tracker
	 */
	std::shared_ptr<PerformanceTracker> addNewPerfTracker(const std::string &name, uint64_t id = 0);

	/**
	 * @brief Adds a new Status tracker to prometheus service
	 * @param[in] name Name of the metric
	 * @param[in] id Optional ID to add metric names
	 * @return std::shared_ptr<StatusTracker> Pointer to new status tracker
	 */
	std::shared_ptr<StatusTracker> addNewStatTracker(const std::string &name, uint64_t id = 0);
};

/// Class to maintain prometheus calls
extern Reporter *mainPrometheusHandler;
