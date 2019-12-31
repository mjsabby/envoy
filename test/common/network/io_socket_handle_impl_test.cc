#include "common/network/io_socket_error_impl.h"
#include "common/network/io_socket_handle_impl.h"

#include "gtest/gtest.h"

namespace Envoy {
namespace Network {
namespace {

TEST(IoSocketHandleImplTest, TestIoSocketError) {
#ifdef WIN32
  IoSocketError error1(WSAEWOULDBLOCK);
#else
  IoSocketError error1(EAGAIN);
#endif
  EXPECT_DEBUG_DEATH(error1.getErrorCode(),
                     ".*assert failure: .* Details: Didn't use getIoSocketEagainInstance.*");

  EXPECT_EQ(::strerror(EAGAIN), IoSocketError::getIoSocketEagainInstance()->getErrorDetails());

#ifdef WIN32
  IoSocketError error3(WSAEOPNOTSUPP);
#else
  IoSocketError error3(ENOTSUP);
#endif
  EXPECT_EQ(IoSocketError::IoErrorCode::NoSupport, error3.getErrorCode());
#ifdef WIN32
  EXPECT_EQ("The attempted operation is not supported for the type of object referenced.\r\n",
            error3.getErrorDetails());
#else
  EXPECT_EQ(::strerror(ENOTSUP), error3.getErrorDetails());
#endif

#ifdef WIN32
  IoSocketError error4(WSAEAFNOSUPPORT);
#else
  IoSocketError error4(EAFNOSUPPORT);
#endif
  EXPECT_EQ(IoSocketError::IoErrorCode::AddressFamilyNoSupport, error4.getErrorCode());
#ifdef WIN32
  EXPECT_EQ("An address incompatible with the requested protocol was used.\r\n",
            error4.getErrorDetails());
#else
  EXPECT_EQ(::strerror(EAFNOSUPPORT), error4.getErrorDetails());
#endif

#ifdef WIN32
  IoSocketError error5(WSAEINPROGRESS);
#else
  IoSocketError error5(EINPROGRESS);
#endif
  EXPECT_EQ(IoSocketError::IoErrorCode::InProgress, error5.getErrorCode());
#ifdef WIN32
  EXPECT_EQ("A blocking operation is currently executing.\r\n", error5.getErrorDetails());
#else
  EXPECT_EQ(::strerror(EINPROGRESS), error5.getErrorDetails());
#endif

#ifdef WIN32
  IoSocketError error6(WSAEACCES);
#else
  IoSocketError error6(EPERM);
#endif
  EXPECT_EQ(IoSocketError::IoErrorCode::Permission, error6.getErrorCode());
#ifdef WIN32
  EXPECT_EQ(
      "An attempt was made to access a socket in a way forbidden by its access permissions.\r\n",
      error6.getErrorDetails());
#else
  EXPECT_EQ(::strerror(EPERM), error6.getErrorDetails());
#endif

  // Random unknown error.
  IoSocketError error7(123);
  EXPECT_EQ(IoSocketError::IoErrorCode::UnknownError, error7.getErrorCode());
#ifdef WIN32
  EXPECT_EQ("The filename, directory name, or volume label syntax is incorrect.\r\n",
            error7.getErrorDetails());
#else
  EXPECT_EQ(::strerror(123), error7.getErrorDetails());
#endif
}

} // namespace
} // namespace Network
} // namespace Envoy
