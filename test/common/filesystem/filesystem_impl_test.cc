#include <string>

#include "common/filesystem/filesystem_impl.h"

#include "test/test_common/environment.h"
#include "test/test_common/utility.h"

#include "gtest/gtest.h"

namespace Envoy {

#ifdef WIN32
const std::string devNull = "NUL";
#else
const std::string devNull = "/dev/null";
#endif

class FileSystemImplTest : public testing::Test {
protected:
#ifdef WIN32
  Filesystem::InstanceImplWin32 file_system_;
#else
  Filesystem::InstanceImplPosix file_system_;
#endif
};

TEST_F(FileSystemImplTest, fileExists) {
  EXPECT_FALSE(file_system_.fileExists("/dev/blahblahblah"));
#ifdef WIN32
  const std::string file_path = TestEnvironment::writeStringToFileForTest("test_envoy", "x");
  EXPECT_TRUE(file_system_.fileExists(file_path));
  EXPECT_TRUE(file_system_.fileExists("c:/windows"));
#else
  EXPECT_TRUE(file_system_.fileExists(devNull));
  EXPECT_TRUE(file_system_.fileExists("/dev"));
#endif
}

TEST_F(FileSystemImplTest, directoryExists) {
#ifdef WIN32
  const std::string file_path = TestEnvironment::writeStringToFileForTest("test_envoy", "x");
  EXPECT_FALSE(file_system_.directoryExists(file_path));
  EXPECT_TRUE(file_system_.directoryExists("c:/windows"));
#else
  EXPECT_FALSE(file_system_.directoryExists(devNull));
  EXPECT_TRUE(file_system_.directoryExists("/dev"));
#endif
  EXPECT_FALSE(file_system_.directoryExists("/dev/blahblah"));
}

TEST_F(FileSystemImplTest, fileSize) {
  EXPECT_EQ(0, file_system_.fileSize(devNull));
  EXPECT_EQ(-1, file_system_.fileSize("/dev/blahblahblah"));
  const std::string data = "test string\ntest";
  const std::string file_path = TestEnvironment::writeStringToFileForTest("test_envoy", data);
  EXPECT_EQ(data.length(), file_system_.fileSize(file_path));
}

TEST_F(FileSystemImplTest, fileReadToEndSuccess) {
  const std::string data = "test string\ntest";
  const std::string file_path = TestEnvironment::writeStringToFileForTest("test_envoy", data);

  EXPECT_EQ(data, file_system_.fileReadToEnd(file_path));
}

// Files are read into std::string; verify that all bytes (eg non-ascii characters) come back
// unmodified
TEST_F(FileSystemImplTest, fileReadToEndSuccessBinary) {
  std::string data;
  for (unsigned i = 0; i < 256; i++) {
    data.push_back(i);
  }
  const std::string file_path = TestEnvironment::writeStringToFileForTest("test_envoy", data);

  const std::string read = file_system_.fileReadToEnd(file_path);
  const std::vector<uint8_t> binary_read(read.begin(), read.end());
  EXPECT_EQ(binary_read.size(), 256);
  for (unsigned i = 0; i < 256; i++) {
    EXPECT_EQ(binary_read.at(i), i);
  }
}

TEST_F(FileSystemImplTest, fileReadToEndDoesNotExist) {
  ::unlink(TestEnvironment::temporaryPath("envoy_this_not_exist").c_str());
  EXPECT_THROW(file_system_.fileReadToEnd(TestEnvironment::temporaryPath("envoy_this_not_exist")),
               EnvoyException);
}

#ifndef WIN32
TEST_F(FileSystemImplTest, CanonicalPathSuccess) {
  EXPECT_EQ("/", file_system_.canonicalPath("//"));
}

TEST_F(FileSystemImplTest, CanonicalPathFail) {
  EXPECT_THROW_WITH_MESSAGE(file_system_.canonicalPath("/_some_non_existent_file"), EnvoyException,
                            "Unable to determine canonical path for /_some_non_existent_file");
}
#endif

TEST_F(FileSystemImplTest, IllegalPath) {
  EXPECT_FALSE(file_system_.illegalPath("/"));
#ifdef WIN32
  EXPECT_FALSE(file_system_.illegalPath("/dev"));
  EXPECT_FALSE(file_system_.illegalPath("/dev/"));
  EXPECT_FALSE(file_system_.illegalPath("/proc"));
  EXPECT_FALSE(file_system_.illegalPath("/proc/"));
  EXPECT_FALSE(file_system_.illegalPath("/sys"));
  EXPECT_FALSE(file_system_.illegalPath("/sys/"));
  EXPECT_FALSE(file_system_.illegalPath("/_some_non_existent_file"));
#else
  EXPECT_FALSE(file_system_.illegalPath("//"));
  EXPECT_TRUE(file_system_.illegalPath("/dev"));
  EXPECT_TRUE(file_system_.illegalPath("/dev/"));
  EXPECT_TRUE(file_system_.illegalPath("/proc"));
  EXPECT_TRUE(file_system_.illegalPath("/proc/"));
  EXPECT_TRUE(file_system_.illegalPath("/sys"));
  EXPECT_TRUE(file_system_.illegalPath("/sys/"));
  EXPECT_TRUE(file_system_.illegalPath("/_some_non_existent_file"));
#endif
}

TEST_F(FileSystemImplTest, BadFile) {
  EXPECT_THROW(file_system_.createFile(""), EnvoyException);
}

TEST_F(FileSystemImplTest, OpenExisting) {
  const std::string file_path =
      TestEnvironment::writeStringToFileForTest("test_envoy", "existing file");

  {
    Filesystem::FilePtr file = file_system_.createFile(file_path);
    std::string data(" new data");
    const Api::SysCallSizeResult result = file->write(data.data(), data.length());
    EXPECT_EQ(data.length(), result.rc_);
  }

  auto contents = TestEnvironment::readFileToStringForTest(file_path);
  EXPECT_EQ("existing file new data", contents);
}

TEST_F(FileSystemImplTest, CreateNew) {
  const std::string new_file =
      TestEnvironment::temporaryPath("envoy_this_not_exist");
  ::unlink(new_file.c_str());

  {
    Filesystem::FilePtr file = file_system_.createFile(new_file);
    std::string data(" new data");
    const Api::SysCallSizeResult result = file->write(data.data(), data.length());
    EXPECT_EQ(data.length(), result.rc_);
  }

  auto contents = TestEnvironment::readFileToStringForTest(new_file);
  EXPECT_EQ(" new data", contents);
}

} // namespace Envoy
