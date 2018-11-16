#include <chrono>
#include <string>

#include "common/api/api_impl.h"

#include "test/test_common/environment.h"

#include "gtest/gtest.h"

namespace Envoy {
namespace Api {

TEST(ApiImplTest, readFileToEnd) {
  Impl api(std::chrono::milliseconds(10000));

  const std::string data = "test read To End\nWith new lines.";
  const std::string file_path = TestEnvironment::writeStringToFileForTest("test_api_envoy", data);

  EXPECT_EQ(data, api.fileReadToEnd(file_path));
}

TEST(ApiImplTest, fileExists) {
  Impl api(std::chrono::milliseconds(10000));

#if !defined(WIN32)
  EXPECT_TRUE(api.fileExists("/dev/null"));
#else
  const std::string file_path =
      TestEnvironment::writeStringToFileForTest("an_existing_file", "hello");
  EXPECT_TRUE(api.fileExists(file_path));
#endif
  EXPECT_FALSE(api.fileExists("/dev/blahblahblah"));
}

} // namespace Api
} // namespace Envoy
