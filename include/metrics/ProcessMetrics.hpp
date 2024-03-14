#include <prometheus/gauge.h>
#include <prometheus/registry.h>

#include <sys/times.h>
#include <thread>

/**
 * @class ProcessMetrics
 * @brief Class that provides metrics related to the current process.
 */
class ProcessMetrics {
  private:
	std::atomic_flag _threadFlag; /**< Atomic flag to control thread loop */
    std::unique_ptr<std::thread> _thread; /**< Thread handler */
	std::shared_ptr<std::atomic_flag> _checkFlag; /**< Runtime check flag */

	prometheus::Gauge *_pMemory;				/**< Pointer to the memory usage gauge */
	prometheus::Gauge *_pPageFaults;			/**< Pointer to the page faults gauge */
	prometheus::Gauge *_pCpuUsage;			/**< Pointer to the CPU usage gauge */
	prometheus::Gauge *_pDiskRead;			/**< Pointer to the disk read gauge */
	prometheus::Gauge *_pDiskWrite;			/**< Pointer to the disk write gauge */
	prometheus::Gauge *_pThreadCount;			/**< Pointer to the thread count gauge */
	prometheus::Gauge *_pFileDescriptorCount; /**< Pointer to the file descriptor count gauge */

	size_t _oldReadBytes{0};	 /**< Variable to store the old read bytes */
	size_t _oldWriteBytes{0}; /**< Variable to store the old write bytes */

	struct tms _oldCpu {
		0, 0, 0, 0
	};					   /**< Structure to store the old CPU times */
	clock_t _oldCpuTime{0}; /**< Variable to store the old CPU time */

	/**
	 * @brief Counts the number of entries in a directory.
	 * @param path The path of the directory.
	 * @return The number of entries in the directory.
	 */
	static size_t countDirectoryEntries(const std::string &path);

  protected:
	/**
	 * @brief Gets the memory usage of the process.
	 * @return The memory usage in bytes.
	 */
	static long int getMemoryUsage();

	/**
	 * @brief Gets the number of page faults of the process.
	 * @return The number of page faults.
	 */
	static long int getPageFaults();

	/**
	 * @brief Gets the CPU usage of the process.
	 * @return The CPU usage as a percentage.
	 */
	double getCpuUsage();

	/**
	 * @brief Gets the disk read and write bytes of the process.
	 * @return A pair of the disk read and write bytes.
	 */
	std::pair<size_t, size_t> getDiskIO();

	/**
	 * @brief Gets the number of threads in the process.
	 * @return The number of threads.
	 */
	static size_t getThreadCount();

	/**
	 * @brief Gets the number of file descriptors in the process.
	 * @return The number of file descriptors.
	 */
	static size_t getFileDescriptorCount();
	
	/**
	 * @brief Updates the metrics values.
	 */
	void update();

	/**
	 * @brief Main thread function
	 */
	void threadRunner();

  public:
	/**
	 * @brief Constructs a ProcessMetrics object.
	 * @param[in] reg The Prometheus registry.
	 * @param[in] checkFlag Runtime check flag
	 */
	ProcessMetrics(const std::shared_ptr<prometheus::Registry> &reg, const std::shared_ptr<std::atomic_flag> &checkFlag);

	/**
	 * @brief Deconstructs a ProcessMetrics object.
	 */
	~ProcessMetrics();

};
