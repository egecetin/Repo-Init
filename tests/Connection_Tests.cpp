#include "Utils.hpp"
#include "connection/Http.hpp"
#include "connection/Zeromq.hpp"
#include "test-static-definitions.h"

#include <chrono>
#include <future>
#include <thread>

#include <gtest/gtest.h>

TEST(Connection_Tests, HttpTests)
{
	// Launch echo server
	std::future<int> pyResult;
	pyResult = std::async(std::launch::async, []() {
		return system(("python3 " + std::string(TEST_REST_ECHO_SERVER_PY_PATH) + " >/dev/null").c_str());
	});
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	HTTP handler(TEST_HTTP_ECHO_SERVER_ADDR);

	handler.setOption(CURLOPT_SSL_VERIFYPEER, false);
	handler.setOption(CURLOPT_SSL_VERIFYHOST, false);

	HttpStatus::Code statusCode = HttpStatus::Code::xxx_max;
	std::string recvData;
	ASSERT_EQ(handler.sendPOSTRequest("", "Test POST Message", recvData, statusCode), CURLE_OK);
	ASSERT_EQ("Test POST Message", recvData);
	ASSERT_EQ(HttpStatus::Code::OK, statusCode);

	statusCode = HttpStatus::Code::xxx_max;
	recvData.clear();
	ASSERT_EQ(handler.sendGETRequest("", recvData, statusCode), CURLE_OK);
	ASSERT_EQ("", recvData);
	ASSERT_EQ(HttpStatus::Code::OK, statusCode);

	pyResult.wait();
	ASSERT_EQ(0, pyResult.get());

	// Send requests to closed server
	statusCode = HttpStatus::Code::xxx_max;
	recvData.clear();
	ASSERT_EQ(handler.sendPOSTRequest("", "Test POST Message", recvData, statusCode), CURLE_COULDNT_CONNECT);
	ASSERT_EQ("", recvData);
	ASSERT_EQ(HttpStatus::Code::xxx_max, statusCode);

	ASSERT_EQ(handler.sendGETRequest("", recvData, statusCode), CURLE_COULDNT_CONNECT);
	ASSERT_EQ("", recvData);
	ASSERT_EQ(HttpStatus::Code::xxx_max, statusCode);
}

TEST(Connection_Tests, ZeroMQTests)
{
	// Launch echo server
	std::future<int> pyResult;
	pyResult = std::async(std::launch::async, []() {
		return system(("python3 " + std::string(TEST_ZEROMQ_ECHO_SERVER_PY_PATH) + " >/dev/null").c_str());
	});
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	std::shared_ptr<zmq::context_t> ctx = std::make_shared<zmq::context_t>(1);
	ZeroMQ handler(ctx, zmq::socket_type::req, TEST_ZEROMQ_ECHO_SERVER_ADDR, false);

	handler.getSocket()->set(zmq::sockopt::linger, 1234);
	ASSERT_EQ(handler.getSocket()->get(zmq::sockopt::linger), 1234);

	ASSERT_TRUE(handler.start());
	ASSERT_FALSE(handler.start());

	int testData1 = 15;
	double testData2 = 3.14;
	std::string testData3 = "Test Message";

	std::vector<zmq::const_buffer> vSendMsg;
	vSendMsg.push_back(zmq::const_buffer(&testData1, sizeof(testData1)));
	vSendMsg.push_back(zmq::const_buffer(&testData2, sizeof(testData2)));
	vSendMsg.push_back(zmq::const_buffer(testData3.c_str(), testData3.size()));
	ASSERT_EQ(handler.sendMessages(vSendMsg), vSendMsg.size());

	auto vRecvMsgs = handler.recvMessages();
	ASSERT_EQ(vSendMsg.size(), vRecvMsgs.size());
	for (size_t idx = 0; idx < vSendMsg.size(); ++idx)
	{
		ASSERT_EQ(vSendMsg[idx].size(), vRecvMsgs[idx].size());
		ASSERT_FALSE(memcmp(vSendMsg[idx].data(), vRecvMsgs[idx].data(), vSendMsg[idx].size()));
	}

	pyResult.wait();
	ASSERT_EQ(0, pyResult.get());

	ZeroMQ handler2(zmq::socket_type::rep, TEST_ZEROMQ_ECHO_SERVER_ADDR2, true);

	ASSERT_EQ(handler2.recvMessages().size(), 0);
	ASSERT_EQ(handler2.sendMessages(vSendMsg), 0);

	handler2.stop();
	ASSERT_TRUE(handler2.start());
}
