#include "envoy/common/exception.h"
#include "envoy/common/platform.h"

#include "common/api/os_sys_calls_impl.h"
#include "common/network/listen_socket_impl.h"
#include "common/network/utility.h"

#include "test/mocks/network/mocks.h"
#include "test/test_common/environment.h"
#include "test/test_common/network_utility.h"
#include "test/test_common/utility.h"

#include "gtest/gtest.h"

using testing::_;
using testing::Return;

namespace Envoy {
namespace Network {

class ListenSocketImplTest : public testing::TestWithParam<Address::IpVersion> {
protected:
  ListenSocketImplTest() : version_(GetParam()) {}
  const Address::IpVersion version_;
};

INSTANTIATE_TEST_CASE_P(IpVersions, ListenSocketImplTest,
                        testing::ValuesIn(TestEnvironment::getIpVersionsForTest()),
                        TestUtility::ipTestParamsToString);

TEST_P(ListenSocketImplTest, BindSpecificPort) {
  // Pick a free port.
  auto addr_fd = Network::Test::bindFreeLoopbackPort(version_, Address::SocketType::Stream);
  auto addr = addr_fd.first;
  ASSERT_TRUE(SOCKET_VALID(addr_fd.second));

  // Confirm that we got a reasonable address and port.
  ASSERT_EQ(Address::Type::Ip, addr->type());
  ASSERT_EQ(version_, addr->ip()->version());
  ASSERT_LT(0U, addr->ip()->port());

  // Release the socket and re-bind it.
  // WARNING: This test has a small but real risk of flaky behavior if another thread or process
  // should bind to our assigned port during the interval between closing the fd and re-binding.
  // TODO(jamessynge): Consider adding a loop or other such approach to this test so that a
  // bind failure (in the TcpListenSocket ctor) once isn't considered an error.
  auto& os_sys_calls = Api::OsSysCallsSingleton::get();
  EXPECT_EQ(0, os_sys_calls.closeSocket(addr_fd.second).rc_);

  auto option = std::make_unique<MockSocketOption>();
  auto options = std::make_shared<std::vector<Network::Socket::OptionConstSharedPtr>>();
  EXPECT_CALL(*option, setOption(_, envoy::api::v2::core::SocketOption::STATE_PREBIND))
      .WillOnce(Return(true));
  options->emplace_back(std::move(option));
  TcpListenSocket socket1(addr, options, true);
  EXPECT_EQ(0, os_sys_calls.listen(socket1.fd(), 0).rc_);
  EXPECT_EQ(addr->ip()->port(), socket1.localAddress()->ip()->port());
  EXPECT_EQ(addr->ip()->addressAsString(), socket1.localAddress()->ip()->addressAsString());

  auto option2 = std::make_unique<MockSocketOption>();
  auto options2 = std::make_shared<std::vector<Network::Socket::OptionConstSharedPtr>>();
  EXPECT_CALL(*option2, setOption(_, envoy::api::v2::core::SocketOption::STATE_PREBIND))
      .WillOnce(Return(true));
  options2->emplace_back(std::move(option2));
  // The address and port are bound already, should throw exception.
  EXPECT_THROW(Network::TcpListenSocket socket2(addr, options2, true), EnvoyException);

  // Test the case of a socket with fd and given address and port.
  const SOCKET_FD dup_socket = TestUtility::duplicateSocket(socket1.fd());
  TcpListenSocket socket3(dup_socket, addr, nullptr);
  EXPECT_EQ(addr->asString(), socket3.localAddress()->asString());
}

// Validate that we get port allocation when binding to port zero.
TEST_P(ListenSocketImplTest, BindPortZero) {
  auto loopback = Network::Test::getCanonicalLoopbackAddress(version_);
  TcpListenSocket socket(loopback, nullptr, true);
  EXPECT_EQ(Address::Type::Ip, socket.localAddress()->type());
  EXPECT_EQ(version_, socket.localAddress()->ip()->version());
  EXPECT_EQ(loopback->ip()->addressAsString(), socket.localAddress()->ip()->addressAsString());
  EXPECT_GT(socket.localAddress()->ip()->port(), 0U);
}

} // namespace Network
} // namespace Envoy
