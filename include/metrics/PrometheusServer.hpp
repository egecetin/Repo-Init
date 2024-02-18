#pragma once

#include <prometheus/exposer.h>
#include <prometheus/registry.h>

/**
 * @class PrometheusServer
 * @brief Class representing a Prometheus server for collecting and exposing metrics.
 */
class PrometheusServer {
  private:
	/// Mutex for concurrent add tracker calls
	std::mutex guardLock;
	/// Main HTTP Server
	std::unique_ptr<prometheus::Exposer> mainExposer;
	/// All tracker registries
	std::vector<std::pair<uint64_t, std::shared_ptr<prometheus::Registry>>> vRegister;

	/// General application information
	prometheus::Family<prometheus::Info> *infoFamily;

  public:
	/**
	 * @brief Construct a new Prometheus Server
	 * @param[in] serverAddr Server address in <IP>:<Port> format
	 */
	explicit PrometheusServer(const std::string &serverAddr);

	/**
	 * @brief Get the registry with id
	 * @param[in] regId Registry id
	 * @return std::shared_ptr<prometheus::Registry> Registry pointer if found, nullptr otherwise
	 */
	std::shared_ptr<prometheus::Registry> getRegistry(uint64_t regId);

	/**
	 * @brief Create a registry for prometheus
	 * @return std::shared_ptr<prometheus::Registry> Registry pointer
	 */
	std::shared_ptr<prometheus::Registry> createNewRegistry();

	/**
	 * @brief Create a registry for prometheus and returns id of the registry
	 * @param[out] regId Registry id
	 * @return std::shared_ptr<prometheus::Registry> Registry pointer
	 */
	std::shared_ptr<prometheus::Registry> createNewRegistry(uint64_t &regId);

	/**
	 * @brief Deletes the registry with id
	 * @param[in] regId Registry id
	 * @return true If successfully removed
	 * @return false otherwise
	 */
	bool deleteRegistry(uint64_t regId);
};
