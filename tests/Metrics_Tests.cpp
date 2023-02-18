#include "test-static-definitions.h"

#include "metrics/Reporter.hpp"

#include <chrono>
#include <cmath>
#include <fstream>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

TEST(Metrics_Tests, ReporterTests)
{
	Reporter reporter(TEST_PROMETHEUS_SERVER_ADDR);

	// Create Performance Tracker
	auto ptr1 = reporter.addNewPerfTracker("testPerformanceTracker", 3, 1);
	ASSERT_NE(nullptr, ptr1);

	ASSERT_EQ("testPerformanceTracker_timing", ptr1->getName());

	// Trigger internal functions
	ptr1->startTimer();
	ptr1->endTimer();

	// At least twice because std requires at least two data points
	ptr1->startTimer();
	ptr1->endTimer();
	ptr1->startTimer();
	ptr1->endTimer();

	// Create Status Tracker
	auto ptr2 = reporter.addNewStatTracker("testStatTracker", 2);
	ASSERT_NE(nullptr, ptr2);

	ASSERT_EQ("testStatTracker", ptr2->getName());
	// Trigger internal functions
	ptr2->incrementActive();
	ptr2->incrementActive();
	ptr2->incrementActive();
	ptr2->incrementSuccess();
	ptr2->incrementFail();

	// Create Mean Tracker
	auto ptr3 = reporter.addNewMeanTracker("testCustomMean", 3, 3);
	ASSERT_NE(nullptr, ptr3);

	ASSERT_EQ("testCustomMean", ptr3->getName());
	// Trigger internal functions
	ptr3->updateValue(10);
	ptr3->updateValue(15);
	ptr3->updateValue(20);
	ptr3->updateValue(25);
	ptr3->updateValue(40);

	// Collect data from socket
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	ASSERT_FALSE(system((std::string("curl ") + TEST_PROMETHEUS_SERVER_ADDR + "/metrics --output metrics").c_str()));

	// Parse output
	std::ifstream promFileStream("metrics");
	ASSERT_TRUE(promFileStream.is_open());

	// Metrics
	std::vector<std::string> readValues;
	std::vector<std::string> testValues = {"testPerformanceTracker_timing_event_ctr_1",
										   "testPerformanceTracker_timing_mean_1",
										   "testPerformanceTracker_timing_var_1",
										   "testPerformanceTracker_timing_moving_mean_1",
										   "testPerformanceTracker_timing_moving_var_1",
										   "testPerformanceTracker_timing_max_1",
										   "testPerformanceTracker_timing_min_1",
										   "testStatTracker_total_event_ctr_2",
										   "testStatTracker_success_event_ctr_2",
										   "testStatTracker_fail_event_ctr_2",
										   "testStatTracker_active_event_ctr_2",
										   "testCustomMean_event_ctr_3",
										   "testCustomMean_mean_3",
										   "testCustomMean_var_3",
										   "testCustomMean_moving_mean_3",
										   "testCustomMean_moving_var_3",
										   "testCustomMean_max_3",
										   "testCustomMean_min_3"};

	// Test all values exist?
	for (size_t idx = 0; idx < testValues.size(); ++idx)
	{
		while (!promFileStream.eof())
		{
			std::string line;
			std::getline(promFileStream, line);
			if (line.rfind(testValues[idx], 0) == 0)
			{
				readValues.push_back(line.substr(line.find(' ')));

				promFileStream.clear();
				promFileStream.seekg(0);
				break;
			}
		}

		ASSERT_FALSE(promFileStream.eof());
	}

	ASSERT_EQ(testValues.size(), readValues.size());

	ASSERT_EQ(3, std::stoi(readValues[0]));						  // testPerformanceTracker_timing__event_ctr_1
	ASSERT_TRUE(std::stoi(readValues[1]));						  // testPerformanceTracker_timing__mean_timing_1
	ASSERT_TRUE(std::stoi(readValues[2]));						  // testPerformanceTracker_timing__var_timing_1
	ASSERT_TRUE(std::stoi(readValues[3]));						  // testPerformanceTracker_timing__moving_mean_timing_1
	ASSERT_TRUE(std::stoi(readValues[4]));						  // testPerformanceTracker_timing__moving_var_timing_1
	ASSERT_GT(std::stoi(readValues[5]), 10);					  // testPerformanceTracker_timing__max_timing_1
	ASSERT_LE(std::stoi(readValues[6]), 1e6);					  // testPerformanceTracker_timing__min_timing_1
	ASSERT_EQ(2, std::stoi(readValues[7]));						  // testStatTracker_total_event_ctr_2
	ASSERT_EQ(1, std::stoi(readValues[8]));						  // testStatTracker_success_event_ctr_2
	ASSERT_EQ(1, std::stoi(readValues[9]));						  // testStatTracker_fail_event_ctr_2
	ASSERT_EQ(1, std::stoi(readValues[10]));					  // testStatTracker_active_event_ctr_2
	ASSERT_EQ(5, std::stoi(readValues[11]));					  // "testCustomMean_event_ctr_3"
	ASSERT_EQ(22, std::stoi(readValues[12]));					  // "testCustomMean_mean_3"
	ASSERT_EQ(132.5, std::stod(readValues[13]));				  // "testCustomMean_var_3"
	ASSERT_LE(fabs(85.0 / 3 - std::stod(readValues[14])), 1e-5);  // "testCustomMean_moving_mean_3"
	ASSERT_LE(fabs(325.0 / 3 - std::stod(readValues[15])), 1e-5); // "testCustomMean_moving_var_3"
	ASSERT_EQ(40, std::stoi(readValues[16]));					  // "testCustomMean_max_3"
	ASSERT_EQ(10, std::stoi(readValues[17]));					  // "testCustomMean_min_3"
}
