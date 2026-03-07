#include "ZeroMQEchoServer.hpp"
#include "ZeroMQTestClient.hpp"
#include "metrics/PrometheusServer.hpp"
#include "test-static-definitions.h"
#include "zeromq/ZeroMQServer.hpp"

#include <chrono>
#include <memory>
#include <thread>

#include <gtest/gtest.h>

TEST(ZeroMQ_Tests, ZeroMQUnitTests)
{
	std::string echoServerAddr = "tcp://127.0.0.1:8001";

	{
		// Launch echo server
		auto echoServer = std::make_unique<ZeroMQEchoServer>(echoServerAddr, 1);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		std::shared_ptr<zmq::context_t> ctx = std::make_shared<zmq::context_t>(1);
		ZeroMQ handler(ctx, zmq::socket_type::req, echoServerAddr, false);

		ASSERT_EQ(ctx, handler.getContext());

		ASSERT_NO_THROW(handler.getSocket()->set(zmq::sockopt::linger, 1234));
		ASSERT_EQ(handler.getSocket()->get(zmq::sockopt::linger), 1234);

		ASSERT_TRUE(handler.start());
		ASSERT_FALSE(handler.start());

		int testData1 = 15;
		double testData2 = 3.14;
		std::string testData3 = "Test Message";

		std::vector<zmq::message_t> vExpectedMsg;
		vExpectedMsg.push_back(zmq::message_t(&testData1, sizeof(testData1)));
		vExpectedMsg.push_back(zmq::message_t(&testData2, sizeof(testData2)));
		vExpectedMsg.push_back(zmq::message_t(testData3.c_str(), testData3.size()));

		std::vector<zmq::message_t> vSendMsg;
		vSendMsg.push_back(zmq::message_t(&testData1, sizeof(testData1)));
		vSendMsg.push_back(zmq::message_t(&testData2, sizeof(testData2)));
		vSendMsg.push_back(zmq::message_t(testData3.c_str(), testData3.size()));
		ASSERT_EQ(handler.sendMessages(vSendMsg), vSendMsg.size());

		auto vRecvMsgs = handler.recvMessages();
		ASSERT_EQ(vSendMsg.size(), vRecvMsgs.size());
		for (size_t idx = 0; idx < vSendMsg.size(); ++idx)
		{
			ASSERT_TRUE(vSendMsg[idx].empty());

			ASSERT_EQ(vExpectedMsg[idx].size(), vRecvMsgs[idx].size());
			ASSERT_FALSE(memcmp(vExpectedMsg[idx].data(), vRecvMsgs[idx].data(), vExpectedMsg[idx].size()));
		}

		echoServer->wait();
	}

	{
		ZeroMQ handler(zmq::socket_type::rep, echoServerAddr, true);

		std::vector<zmq::message_t> vSendMsg;
		ASSERT_EQ(handler.recvMessages().size(), 0);
		ASSERT_EQ(handler.sendMessages(vSendMsg), 0);

		ASSERT_NO_THROW(handler.stop());
		ASSERT_TRUE(handler.start());
	}
}

TEST(ZeroMQ_Tests, ZeroMQServerUnitTests)
{
	std::string zeromqServerAddr = "tcp://127.0.0.1:8300";
	std::string promServerAddr = "localhost:8301";

	// For internal statistics
	PrometheusServer reporter(promServerAddr);

	// Init ZeroMQ Server
	std::shared_ptr<std::atomic_flag> checkFlag;
	auto zeromqServerPtr = std::make_shared<ZeroMQServer>(zeromqServerAddr, checkFlag);
	ASSERT_TRUE(zeromqServerPtr->initialise());
	ASSERT_FALSE(zeromqServerPtr->initialise());
	ASSERT_NO_THROW(zeromqServerPtr->shutdown());

	zeromqServerPtr = std::make_shared<ZeroMQServer>(zeromqServerAddr, checkFlag, reporter.createNewRegistry());
	zeromqServerPtr->messageCallback(ZeroMQServerMessageCallback);
	ASSERT_TRUE(zeromqServerPtr->initialise());

	const uint64_t CMD_VERSION = 1230128470;   // "version" hash
	const uint64_t CMD_LOG_LEVEL = 1279741772; // "log level" hash
	const uint64_t CMD_PING = 1196312912;	   // "ping" hash
	const uint64_t CMD_STATUS = 1263027027;	   // "status" hash

	std::vector<std::vector<zmq::message_t>> msgArray;

	// Ask version (success)
	msgArray.push_back(makeMessageVector(CMD_VERSION));
	// Ask version (fail - extra data)
	msgArray.push_back(makeMessageVector(CMD_VERSION, "dummy"));
	// Ask log level (info - "v")
	msgArray.push_back(makeMessageVector(CMD_LOG_LEVEL, "v"));
	// Ask log level (debug - "vv")
	msgArray.push_back(makeMessageVector(CMD_LOG_LEVEL, "vv"));
	// Ask log level (trace - "vvv")
	msgArray.push_back(makeMessageVector(CMD_LOG_LEVEL, "vvv"));
	// Ask log level (fail - extra data)
	msgArray.push_back(makeMessageVector(CMD_LOG_LEVEL, "v", "dummy"));
	// Ask ping (success)
	msgArray.push_back(makeMessageVector(CMD_PING));
	// Ask ping (fail - extra data)
	msgArray.push_back(makeMessageVector(CMD_PING, "dummy"));
	// Ask status (success)
	msgArray.push_back(makeMessageVector(CMD_STATUS));
	// Ask status (fail - extra data)
	msgArray.push_back(makeMessageVector(CMD_STATUS, "dummy"));

	// Launch test client
	auto testClient = std::make_unique<ZeroMQTestClient>(zeromqServerAddr);
	testClient->sendTestMessages(msgArray);

	ASSERT_NO_THROW(zeromqServerPtr->shutdown());
	ASSERT_NO_THROW(zeromqServerPtr->shutdown());
}

TEST(ZeroMQ_Tests, ZeroMQMonitorUnitTests)
{
	ZeroMQMonitor monitor;
	ASSERT_THROW(monitor.startMonitoring(nullptr, ""), std::invalid_argument);
	ASSERT_NO_THROW(monitor.testInternals());
}
