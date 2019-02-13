#ifndef WIN32
#include <netinet/in.h>

#endif

#include <string>

#include "envoy/common/platform.h"

#include "common/api/os_sys_calls_impl.h"

#include "test/test_common/environment.h"
#include "test/test_common/utility.h"

#include "gtest/gtest.h"

namespace Envoy {
namespace Api {

class OsSysCallsImplTest : public testing::TestWithParam<Network::Address::IpVersion> {};
INSTANTIATE_TEST_CASE_P(IpVersions, OsSysCallsImplTest,
                        testing::ValuesIn(TestEnvironment::getIpVersionsForTest()),
                        TestUtility::ipTestParamsToString);

TEST_P(OsSysCallsImplTest, SocketPair) {
#ifndef WIN32
  // we test socketpair on Windows, since we have our own implementation of it
  // (Windows does not provide one). No need to test on any other OS
  return;
#endif
  Network::Address::IpVersion ip_version = GetParam();
  const int family = ip_version == Network::Address::IpVersion::v4 ? AF_INET : AF_INET6;

  OsSysCallsImpl os_sys_calls{};
  SOCKET_FD socks[2];
  SysCallIntResult r1 = os_sys_calls.socketpair(family, SOCK_STREAM, IPPROTO_TCP, socks);
  EXPECT_EQ(r1.rc_, 0);
  EXPECT_TRUE(SOCKET_VALID(socks[0]));
  EXPECT_TRUE(SOCKET_VALID(socks[1]));

  sockaddr_storage sa;
  socklen_t sa_len = sizeof(sa);
  r1 = os_sys_calls.getsockname(socks[0], reinterpret_cast<sockaddr*>(&sa), &sa_len);
  EXPECT_EQ(r1.rc_, 0);
  EXPECT_EQ(sa.ss_family, family);

  r1 = os_sys_calls.getsockname(socks[1], reinterpret_cast<sockaddr*>(&sa), &sa_len);
  EXPECT_EQ(r1.rc_, 0);
  EXPECT_EQ(sa.ss_family, family);

  int send = 1;
  int recv = 0;
  SysCallSizeResult r2 = os_sys_calls.writeSocket(socks[0], &send, sizeof(send));
  EXPECT_EQ(r2.rc_, sizeof(send));
  r2 = os_sys_calls.recv(socks[1], &recv, sizeof(recv), 0);
  EXPECT_EQ(r2.rc_, sizeof(send));
  EXPECT_EQ(recv, 1);

  send = 2;
  r2 = os_sys_calls.writeSocket(socks[1], &send, sizeof(send));
  EXPECT_EQ(r2.rc_, sizeof(send));
  r2 = os_sys_calls.recv(socks[0], &recv, sizeof(recv), 0);
  EXPECT_EQ(r2.rc_, sizeof(send));
  EXPECT_EQ(recv, 2);

#ifdef WIN32
  EXPECT_EQ(::closesocket(socks[0]), 0);
  EXPECT_EQ(::closesocket(socks[1]), 0);
#endif
}

} // namespace Api
} // namespace Envoy
