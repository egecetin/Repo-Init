#include "Tracer.hpp"
#include "test-static-definitions.h"

#include <gtest/gtest.h>

TEST(Tracer_Tests, TracerUnitTests)
{
	// Arrange
	std::string serverPath = "localhost";
	std::string serverProxy = "proxy";
	std::map<std::string, std::string> annotations;
	std::vector<base::FilePath> attachments;
	std::string reportPath = "/tmp";

	std::unique_ptr<Tracer> tracer;

	ASSERT_NO_THROW(tracer = std::make_unique<Tracer>(serverPath, serverProxy, TEST_TRACER_CRASHDUMP_PATH, annotations,
													  attachments, reportPath));

	ASSERT_TRUE(tracer->isRunning());
	ASSERT_NO_THROW(tracer->restart());

	ASSERT_TRUE(tracer->dumpSharedLibraryInfo("/tmp/shared_libraries.txt"));
	ASSERT_FALSE(tracer->dumpSharedLibraryInfo("/nonexistent/shared_libraries.txt"));
}
