#include "common/network/io_socket_handle_impl.h"

#include "test/test_common/test_base.h"

namespace Envoy {
namespace Network {
namespace {

TEST(IoSocketHandleImplTest, TestIoSocketError) {
#ifdef WIN32
  IoSocketError error1(WSAEWOULDBLOCK);
#else
  IoSocketError error1(EAGAIN);
#endif
  EXPECT_DEATH(Api::IoError::getErrorCode(error1), "");

  EXPECT_EQ("Try again later", Api::IoError::getErrorDetails(*ENVOY_ERROR_AGAIN));

#ifdef WIN32
  IoSocketError error3(WSAEOPNOTSUPP);
#else
  IoSocketError error3(ENOTSUP);
#endif
  EXPECT_EQ(IoSocketError::IoErrorCode::NoSupport, Api::IoError::getErrorCode(error3));
#ifdef WIN32
  EXPECT_EQ("The attempted operation is not supported for the type of object referenced.\r\n",
            Api::IoError::getErrorDetails(error3));
#else
  EXPECT_EQ(::strerror(ENOTSUP), Api::IoError::getErrorDetails(error3));
#endif

#ifdef WIN32
  IoSocketError error4(WSAEAFNOSUPPORT);
#else
  IoSocketError error4(EAFNOSUPPORT);
#endif
  EXPECT_EQ(IoSocketError::IoErrorCode::AddressFamilyNoSupport, Api::IoError::getErrorCode(error4));
#ifdef WIN32
  EXPECT_EQ("An address incompatible with the requested protocol was used.\r\n",
            Api::IoError::getErrorDetails(error4));
#else
  EXPECT_EQ(::strerror(EAFNOSUPPORT), Api::IoError::getErrorDetails(error4));
#endif

#ifdef WIN32
  IoSocketError error5(WSAEINPROGRESS);
#else
  IoSocketError error5(EINPROGRESS);
#endif
  EXPECT_EQ(IoSocketError::IoErrorCode::InProgress, Api::IoError::getErrorCode(error5));
#ifdef WIN32
  EXPECT_EQ("A blocking operation is currently executing.\r\n",
            Api::IoError::getErrorDetails(error5));
#else
  EXPECT_EQ(::strerror(EINPROGRESS), Api::IoError::getErrorDetails(error5));
#endif

#ifdef WIN32
  IoSocketError error6(WSAEACCES);
#else
  IoSocketError error6(EPERM);
#endif
  EXPECT_EQ(IoSocketError::IoErrorCode::Permission, Api::IoError::getErrorCode(error6));
#ifdef WIN32
  EXPECT_EQ(
      "An attempt was made to access a socket in a way forbidden by its access permissions.\r\n",
      Api::IoError::getErrorDetails(error6));
#else
  EXPECT_EQ(::strerror(EPERM), Api::IoError::getErrorDetails(error6));
#endif

  // Random unknown error.
  IoSocketError error7(123);
  EXPECT_EQ(IoSocketError::IoErrorCode::UnknownError, Api::IoError::getErrorCode(error7));
#ifdef WIN32
  EXPECT_EQ("The filename, directory name, or volume label syntax is incorrect.\r\n",
            Api::IoError::getErrorDetails(error7));
#else
  EXPECT_EQ(::strerror(123), Api::IoError::getErrorDetails(error7));
#endif
}

} // namespace
} // namespace Network
} // namespace Envoy
