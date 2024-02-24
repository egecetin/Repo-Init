#include <prometheus/gauge.h>
#include <prometheus/registry.h>

#include <sys/times.h>

/**
 * @class ProcessMetrics
 * @brief Class that provides metrics related to the current process.
 */
class ProcessMetrics {
  private:
	prometheus::Gauge *memory;				/**< Pointer to the memory usage gauge */
	prometheus::Gauge *pageFaults;			/**< Pointer to the page faults gauge */
	prometheus::Gauge *cpuUsage;			/**< Pointer to the CPU usage gauge */
	prometheus::Gauge *diskRead;			/**< Pointer to the disk read gauge */
	prometheus::Gauge *diskWrite;			/**< Pointer to the disk write gauge */
	prometheus::Gauge *threadCount;			/**< Pointer to the thread count gauge */
	prometheus::Gauge *fileDescriptorCount; /**< Pointer to the file descriptor count gauge */

	size_t oldReadBytes{0};	 /**< Variable to store the old read bytes */
	size_t oldWriteBytes{0}; /**< Variable to store the old write bytes */

	struct tms oldCpu;	   /**< Structure to store the old CPU times */
	clock_t oldCpuTime{0}; /**< Variable to store the old CPU time */

	/**
	 * @brief Counts the number of entries in a directory.
	 * @param path The path of the directory.
	 * @return The number of entries in the directory.
	 */
	size_t countDirectoryEntries(const std::string &path);

  protected:
	/**
	 * @brief Gets the memory usage of the process.
	 * @return The memory usage in bytes.
	 */
	long int getMemoryUsage();

	/**
	 * @brief Gets the number of page faults of the process.
	 * @return The number of page faults.
	 */
	long int getPageFaults();

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
	size_t getThreadCount();

	/**
	 * @brief Gets the number of file descriptors in the process.
	 * @return The number of file descriptors.
	 */
	size_t getFileDescriptorCount();

  public:
	/**
	 * @brief Constructs a ProcessMetrics object.
	 * @param reg The Prometheus registry.
	 */
	ProcessMetrics(const std::shared_ptr<prometheus::Registry> &reg);

	/**
	 * @brief Updates the metrics values.
	 */
	void update();
};