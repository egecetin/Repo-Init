#include "test-static-definitions.h"

#include "metrics/Performance.hpp"
#include "metrics/PrometheusServer.hpp"
#include "metrics/Status.hpp"

#include <fstream>
#include <thread>

#include <gtest/gtest.h>

bool isAllValuesExist(std::ifstream &promFile, const std::vector<std::string> &testVals,
					  std::vector<std::string> &readVals)
{
	for (size_t idx = 0; idx < testVals.size(); ++idx)
	{
		while (!promFile.eof())
		{
			std::string line;
			std::getline(promFile, line);
			if (line.rfind(testVals[idx], 0) == 0)
			{
				readVals.push_back(line.substr(line.find(' ')));

				promFile.clear();
				promFile.seekg(0);
				break;
			}
		}

		if (promFile.eof())
			return false;
	}

	return true;
}

TEST(Metrics_Tests, PrometheusServerTests)
{
	PrometheusServer reporter(TEST_PROMETHEUS_SERVER_ADDR);

	// Collect data from socket
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	ASSERT_FALSE(system(
		(std::string("curl ") + TEST_PROMETHEUS_SERVER_ADDR + "/metrics --output metrics &> /dev/null").c_str()));

	// Metrics
	std::vector<std::string> readValues;
	std::vector<std::string> testValues = {};

	// Parse output
	std::ifstream promFileStream("metrics");

	ASSERT_TRUE(promFileStream.is_open());
	ASSERT_TRUE(isAllValuesExist(promFileStream, testValues, readValues));
	ASSERT_EQ(testValues.size(), readValues.size());
}

TEST(Metrics_Tests, PerformanceTrackerTests)
{
	PrometheusServer reporter(TEST_PROMETHEUS_SERVER_ADDR_2);
	PerformanceTracker perfTracker(reporter.createNewRegistry(), "test_performance", 0);

	perfTracker.startTimer();
	perfTracker.endTimer();

	perfTracker.startTimer();
	std::this_thread::sleep_for(std::chrono::milliseconds(150));
	perfTracker.endTimer();
	perfTracker.startTimer();
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	perfTracker.endTimer();

	// Collect data from socket
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	ASSERT_FALSE(system(
		(std::string("curl ") + TEST_PROMETHEUS_SERVER_ADDR_2 + "/metrics --output metrics2 &> /dev/null").c_str()));

	// Metrics
	std::vector<std::string> readValues;
	std::vector<std::string> testValues = {"test_performance_processing_time_0",
										   "test_performance_maximum_processing_time_0",
										   "test_performance_minimum_processing_time_0"};

	// Parse output
	std::ifstream promFileStream("metrics2");

	ASSERT_TRUE(promFileStream.is_open());
	ASSERT_TRUE(isAllValuesExist(promFileStream, testValues, readValues));
	ASSERT_EQ(testValues.size(), readValues.size());

	ASSERT_TRUE(std::stoi(readValues[0]));	  // test_performance_processing_time_0
	ASSERT_GT(std::stoi(readValues[1]), 10);  // test_performance_maximum_processing_time_0
	ASSERT_LE(std::stoi(readValues[2]), 1e6); // test_performance_minimum_processing_time_0
}

TEST(Metrics_Tests, StatusTrackerTests)
{
	PrometheusServer reporter(TEST_PROMETHEUS_SERVER_ADDR_3);
	StatusTracker statTracker(reporter.createNewRegistry(), "test_status", 2);

	statTracker.incrementActive();
	statTracker.incrementActive();
	statTracker.incrementActive();
	statTracker.incrementSuccess();
	statTracker.incrementFail();

	// Collect data from socket
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	ASSERT_FALSE(system(
		(std::string("curl ") + TEST_PROMETHEUS_SERVER_ADDR_3 + "/metrics --output metrics3 &> /dev/null").c_str()));

	// Metrics
	std::vector<std::string> readValues;
	std::vector<std::string> testValues = {"test_status_total_event_ctr_2", "test_status_success_event_ctr_2",
										   "test_status_fail_event_ctr_2", "test_status_active_event_ctr_2"};

	// Parse output
	std::ifstream promFileStream("metrics3");

	ASSERT_TRUE(promFileStream.is_open());
	ASSERT_TRUE(isAllValuesExist(promFileStream, testValues, readValues));
	ASSERT_EQ(testValues.size(), readValues.size());

	ASSERT_EQ(2, std::stoi(readValues[0])); // test_status_total_event_ctr_2
	ASSERT_EQ(1, std::stoi(readValues[1])); // test_status_success_event_ctr_2
	ASSERT_EQ(1, std::stoi(readValues[2])); // test_status_fail_event_ctr_2
	ASSERT_EQ(1, std::stoi(readValues[3])); // test_status_active_event_ctr_2
}
