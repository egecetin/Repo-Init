#pragma once

#include "metrics/Performance.hpp"
#include "metrics/Status.hpp"

#include <prometheus/exposer.h>
#include <prometheus/info.h>
#include <prometheus/registry.h>

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
	/// All mean trackers
	std::vector<std::shared_ptr<MeanVarTracker>> vMeanTracker;

	/// Mutex for concurrent add tracker calls
	std::mutex guardLock;
	/// TSC frequency
	uint64_t tscHz;
	/// Initialize time
	prometheus::Info *initTime;
	/// Performance metric is in clock counter or in seconds
	prometheus::Info *unitPerformance;

  public:
	/**
	 * @brief Construct a new Performance Reporter
	 * @param[in] serverAddr Server address in <IP>:<Port> format
	 */
	explicit Reporter(const std::string &serverAddr);

	/**
	 * @brief Adds a new Performance tracker to prometheus service
	 * @param[in] name Name of the metric
	 * @param[in] windowLength Length of the window for moving statistical properties
	 * @param[in] id Optional ID to add metric names
	 * @return std::shared_ptr<PerformanceTracker> Pointer to new performance tracker
	 */
	std::shared_ptr<PerformanceTracker> addNewPerfTracker(const std::string &name, size_t windowLength,
														  uint64_t id = 0);

	/**
	 * @brief Adds a new Status tracker to prometheus service
	 * @param[in] name Name of the metric
	 * @param[in] id Optional ID to add metric names
	 * @return std::shared_ptr<StatusTracker> Pointer to new status tracker
	 */
	std::shared_ptr<StatusTracker> addNewStatTracker(const std::string &name, uint64_t id = 0);

	/**
	 * @brief Adds a new custom mean tracker to prometheus service
	 * @param[in] name Name of the metric
	 * @param[in] windowLength Length of the window for moving statistical properties
	 * @param[in] id Optional ID to add metric names
	 * @return std::shared_ptr<MeanVarTracker> Pointer to new mean tracker
	 */
	std::shared_ptr<MeanVarTracker> addNewMeanTracker(const std::string &name, size_t windowLength, uint64_t id = 0);
};

/// Class to maintain prometheus calls
extern Reporter *mainPrometheusHandler;
