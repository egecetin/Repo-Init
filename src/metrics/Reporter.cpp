#include "metrics/Reporter.hpp"

#include <cpuid.h>

Reporter *mainPrometheusHandler;

Reporter::Reporter(const std::string &serverAddr)
{
	uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
	__cpuid(0x16, eax, ebx, ecx, edx);
	if (eax)
		tscHz = eax * uint64_t(1000000);
	else
		tscHz = 1;

	// Init service
	mainExposer = std::make_unique<prometheus::Exposer>(serverAddr, 1);

	struct timespec ts;
	clock_gettime(CLOCK_TAI, &ts);

	auto reg = std::make_shared<prometheus::Registry>();
	initTime = &prometheus::BuildInfo()
					.Name("start_time")
					.Help("Initialization time of the application")
					.Register(*reg)
					.Add({{"init_time", std::to_string(ts.tv_sec)}});
	vRegister.push_back(reg);

	mainExposer->RegisterCollectable(reg);
}

std::shared_ptr<PerformanceTracker> Reporter::addNewPerfTracker(const std::string &name, size_t windowLength,
																uint64_t id)
{
	std::lock_guard<std::mutex> guard(guardLock);

	auto reg = std::make_shared<prometheus::Registry>();
	auto tracker = std::make_shared<PerformanceTracker>(reg, name, tscHz, windowLength, id);
	mainExposer->RegisterCollectable(reg);

	vRegister.push_back(reg);
	vPerfTracker.push_back(tracker);

	return tracker;
}

std::shared_ptr<StatusTracker> Reporter::addNewStatTracker(const std::string &name, uint64_t id)
{
	std::lock_guard<std::mutex> guard(guardLock);

	auto reg = std::make_shared<prometheus::Registry>();
	auto tracker = std::make_shared<StatusTracker>(reg, name, id);
	mainExposer->RegisterCollectable(reg);

	vRegister.push_back(reg);
	vStatTracker.push_back(tracker);

	return tracker;
}
