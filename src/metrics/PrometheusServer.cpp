#include "metrics/PrometheusServer.hpp"
#include "Utils.hpp"
#include "Version.h"

#include <date/date.h>
#include <prometheus/info.h>

#include <limits>

PrometheusServer *mainPrometheusServer;

PrometheusServer::PrometheusServer(const std::string &serverAddr)
{
	// Init service
	mainExposer = std::make_unique<prometheus::Exposer>(serverAddr, 1);

	auto reg = std::make_shared<prometheus::Registry>();

	// Basic information
	infoFamily =
		&prometheus::BuildInfo().Name(PROJECT_NAME).Help(std::string(PROJECT_NAME) + " information").Register(*reg);

	infoFamily->Add({{"init_time", date::format("%FT%TZ", date::floor<std::chrono::nanoseconds>(
															  std::chrono::high_resolution_clock::now()))}});
	infoFamily->Add({{"version", get_version()}});

	vRegister.push_back(std::make_pair(std::numeric_limits<uint64_t>::max(), reg));
	mainExposer->RegisterCollectable(reg);
}

std::shared_ptr<prometheus::Registry> PrometheusServer::getRegistry(uint64_t id)
{
	std::lock_guard<std::mutex> guard(guardLock);

	auto it = std::find_if(
		vRegister.begin(), vRegister.end(),
		[id](const std::pair<uint64_t, std::shared_ptr<prometheus::Registry>> &val) { return id == val.first; });

	if (it != vRegister.end())
		return it->second;
	return nullptr;
}

std::shared_ptr<prometheus::Registry> PrometheusServer::createNewRegistry()
{
	uint64_t id;
	return createNewRegistry(id);
}

std::shared_ptr<prometheus::Registry> PrometheusServer::createNewRegistry(uint64_t &id)
{
	std::lock_guard<std::mutex> guard(guardLock);

	// Create registry
	auto reg = std::make_shared<prometheus::Registry>();
	mainExposer->RegisterCollectable(reg);

	// Push to vector (At least information registry always exist so back is valid)
	id = vRegister.back().first + 1;
	vRegister.push_back(std::make_pair(vRegister.back().first + 1, reg));
	return reg;
}

bool PrometheusServer::deleteRegistry(uint64_t id)
{
	if (id == std::numeric_limits<uint64_t>::max())
		return false;

	std::lock_guard<std::mutex> guard(guardLock);

	vRegister.erase(std::remove_if(
		vRegister.begin(), vRegister.end(),
		[id](const std::pair<uint64_t, std::shared_ptr<prometheus::Registry>> &val) { return id == val.first; }));

	return true;
}
