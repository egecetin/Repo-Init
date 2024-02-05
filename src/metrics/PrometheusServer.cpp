#include "metrics/PrometheusServer.hpp"
#include "Utils.hpp"
#include "Version.h"

#include <date/date.h>
#include <prometheus/info.h>

#include <limits>

PrometheusServer::PrometheusServer(const std::string &serverAddr)
	: mainExposer(std::make_unique<prometheus::Exposer>(serverAddr, 1))
{
	auto reg = std::make_shared<prometheus::Registry>();

	// Basic information
	infoFamily =
		&prometheus::BuildInfo().Name(PROJECT_NAME).Help(std::string(PROJECT_NAME) + " information").Register(*reg);

	infoFamily->Add({{"init_time", date::format("%FT%TZ", date::floor<std::chrono::nanoseconds>(
															  std::chrono::high_resolution_clock::now()))}});
	infoFamily->Add({{"version", get_version()}});

	vRegister.emplace_back(std::numeric_limits<uint64_t>::max(), reg);
	mainExposer->RegisterCollectable(reg);
}

std::shared_ptr<prometheus::Registry> PrometheusServer::getRegistry(uint64_t regId)
{
	const std::lock_guard<std::mutex> guard(guardLock);

	auto iter = std::find_if(
		vRegister.begin(), vRegister.end(),
		[regId](const std::pair<uint64_t, std::shared_ptr<prometheus::Registry>> &val) { return regId == val.first; });

	if (iter != vRegister.end())
	{
		return iter->second;
	}
	return nullptr;
}

std::shared_ptr<prometheus::Registry> PrometheusServer::createNewRegistry()
{
	uint64_t regId = 0;
	return createNewRegistry(regId);
}

std::shared_ptr<prometheus::Registry> PrometheusServer::createNewRegistry(uint64_t &regId)
{
	const std::lock_guard<std::mutex> guard(guardLock);

	// Create registry
	auto reg = std::make_shared<prometheus::Registry>();
	mainExposer->RegisterCollectable(reg);

	// Push to vector (At least information registry always exist so back is valid)
	regId = vRegister.back().first + 1;
	vRegister.emplace_back(vRegister.back().first + 1, reg);
	return reg;
}

bool PrometheusServer::deleteRegistry(uint64_t regId)
{
	if (regId == std::numeric_limits<uint64_t>::max())
	{
		return false;
	}

	const std::lock_guard<std::mutex> guard(guardLock);

	vRegister.erase(std::remove_if(
		vRegister.begin(), vRegister.end(),
		[regId](const std::pair<uint64_t, std::shared_ptr<prometheus::Registry>> &val) { return regId == val.first; }));

	return true;
}
