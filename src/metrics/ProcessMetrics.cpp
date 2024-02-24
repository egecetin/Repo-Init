#include "metrics/ProcessMetrics.hpp"

#include <Utils.hpp>

#include <dirent.h>
#include <sys/resource.h>

size_t ProcessMetrics::countDirectoryEntries(const std::string &path)
{
	DIR *dir = nullptr;
	size_t count = 0;
	if ((dir = opendir(path.c_str())) != nullptr)
	{
		while (readdir(dir) != nullptr) // NOLINT(concurrency-mt-unsafe)
		{
			++count;
		}
		closedir(dir);
	}

	return count;
}

long int ProcessMetrics::getMemoryUsage()
{
	struct rusage r_usage {};
	getrusage(RUSAGE_SELF, &r_usage);
	return r_usage.ru_maxrss; // NOLINT(cppcoreguidelines-pro-type-union-access)
}

long int ProcessMetrics::getPageFaults()
{
	struct rusage r_usage {};
	getrusage(RUSAGE_SELF, &r_usage);
	return r_usage.ru_majflt; // NOLINT(cppcoreguidelines-pro-type-union-access)
}

double ProcessMetrics::getCpuUsage()
{
	if (oldCpuTime == 0)
	{
		oldCpuTime = times(&oldCpu);
		if (oldCpuTime == static_cast<clock_t>(-1))
		{
			oldCpuTime = 0;
		}
		return 0.0;
	}

	struct tms nowCpu {};
	auto nowCpuTime = times(&nowCpu);

	double usage =
		100.0 * static_cast<double>(nowCpu.tms_utime - oldCpu.tms_utime) / static_cast<double>(nowCpuTime - oldCpuTime);

	std::swap(oldCpu, nowCpu);
	std::swap(oldCpuTime, nowCpuTime);

	return usage;
}

std::pair<size_t, size_t> ProcessMetrics::getDiskIO()
{
	std::string buffer;
	findFromFile("/proc/self/io", "read_bytes", buffer);
	size_t readBytes = buffer.empty() ? 0 : std::stoull(buffer);

	buffer.clear();
	findFromFile("/proc/self/io", "write_bytes", buffer);
	size_t writeBytes = buffer.empty() ? 0 : std::stoull(buffer);

	std::pair<size_t, size_t> result = {readBytes - oldReadBytes, writeBytes - oldWriteBytes};

	std::swap(oldReadBytes, readBytes);
	std::swap(oldWriteBytes, writeBytes);

	return result;
}

size_t ProcessMetrics::getThreadCount() { return countDirectoryEntries("/proc/self/task"); }

size_t ProcessMetrics::getFileDescriptorCount() { return countDirectoryEntries("/proc/self/fd"); }

ProcessMetrics::ProcessMetrics(const std::shared_ptr<prometheus::Registry> &reg)
{
	memory = &prometheus::BuildGauge().Name("memory_usage").Help("Memory usage of application").Register(*reg).Add({});
	pageFaults =
		&prometheus::BuildGauge().Name("page_faults").Help("Page faults of application").Register(*reg).Add({});
	cpuUsage = &prometheus::BuildGauge().Name("cpu_usage").Help("CPU usage of application").Register(*reg).Add({});
	diskRead = &prometheus::BuildGauge().Name("disk_read").Help("Disk read of application").Register(*reg).Add({});
	diskWrite = &prometheus::BuildGauge().Name("disk_write").Help("Disk write of application").Register(*reg).Add({});
	threadCount =
		&prometheus::BuildGauge().Name("thread_count").Help("Thread count of application").Register(*reg).Add({});
	fileDescriptorCount = &prometheus::BuildGauge()
							   .Name("file_descriptor_count")
							   .Help("File descriptor count of application")
							   .Register(*reg)
							   .Add({});
}

void ProcessMetrics::update()
{
	memory->Set(static_cast<double>(getMemoryUsage()));
	pageFaults->Set(static_cast<double>(getPageFaults()));
	cpuUsage->Set(getCpuUsage());
	auto diskIO = getDiskIO();
	diskRead->Set(static_cast<double>(diskIO.first));
	diskWrite->Set(static_cast<double>(diskIO.second));
	threadCount->Set(static_cast<double>(getThreadCount()));
	fileDescriptorCount->Set(static_cast<double>(getFileDescriptorCount()));
}
