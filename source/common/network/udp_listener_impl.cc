#include "common/network/udp_listener_impl.h"

#ifndef WIN32
#include <sys/un.h>
#endif

#include <cerrno>
#include <csetjmp>
#include <cstring>

#include "envoy/buffer/buffer.h"
#include "envoy/common/exception.h"

#include "common/api/os_sys_calls_impl.h"
#include "common/common/assert.h"
#include "common/common/empty_string.h"
#include "common/common/fmt.h"
#include "common/common/stack_array.h"
#include "common/event/dispatcher_impl.h"
#include "common/network/address_impl.h"

#include "event2/listener.h"

#define ENVOY_UDP_LOG(LEVEL, FORMAT, ...)                                                          \
  ENVOY_LOG_TO_LOGGER(ENVOY_LOGGER(), LEVEL, "Listener at {} :" FORMAT,                            \
                      this->localAddress()->asString(), ##__VA_ARGS__)

namespace Envoy {
namespace Network {

// Max UDP payload.
static const uint64_t MAX_UDP_PACKET_SIZE = 1500;

UdpListenerImpl::UdpListenerImpl(Event::DispatcherImpl& dispatcher, Socket& socket,
                                 UdpListenerCallbacks& cb, TimeSource& time_source)
    : BaseListenerImpl(dispatcher, socket), cb_(cb), time_source_(time_source) {
  file_event_ = dispatcher_.createFileEvent(
      socket.ioHandle().fd(), [this](uint32_t events) -> void { onSocketEvent(events); },
// libevent only supports level trigger on Windows
#ifdef WIN32
      Event::FileTriggerType::Level, Event::FileReadyType::Read | Event::FileReadyType::Write);
#else
      Event::FileTriggerType::Edge, Event::FileReadyType::Read | Event::FileReadyType::Write);
#endif

  ASSERT(file_event_);

  if (!Network::Socket::applyOptions(socket.options(), socket,
                                     envoy::api::v2::core::SocketOption::STATE_BOUND)) {
    throw CreateListenerException(fmt::format("cannot set post-bound socket option on socket: {}",
                                              socket.localAddress()->asString()));
  }
}

UdpListenerImpl::~UdpListenerImpl() {
  disable();
  file_event_.reset();
}

void UdpListenerImpl::disable() { file_event_->setEnabled(0); }

void UdpListenerImpl::enable() {
  file_event_->setEnabled(Event::FileReadyType::Read | Event::FileReadyType::Write);
}

void UdpListenerImpl::onSocketEvent(short flags) {
  ASSERT((flags & (Event::FileReadyType::Read | Event::FileReadyType::Write)));
  ENVOY_UDP_LOG(trace, "socket event: {}", flags);

  if (flags & Event::FileReadyType::Read) {
    handleReadCallback();
  }

  if (flags & Event::FileReadyType::Write) {
    handleWriteCallback();
  }
}

void UdpListenerImpl::handleReadCallback() {
  ENVOY_UDP_LOG(trace, "handleReadCallback");
  // TODO(danzh) make this variable configurable to support jumbo frames.
  const uint64_t read_buffer_length = MAX_UDP_PACKET_SIZE;
  do {
#if 1 //TODO: Pivotal review
    Buffer::InstancePtr buffer = std::make_unique<Buffer::OwnedImpl>();
    Buffer::RawSlice slice;
    const uint64_t num_slices = buffer->reserve(read_buffer_length, &slice, 1);
    ASSERT(num_slices == 1);

    IoHandle::RecvMsgOutput output(&packets_dropped_);
    uint32_t old_packets_dropped = packets_dropped_;
    MonotonicTime receive_time = time_source_.monotonicTime();
    Api::IoCallUint64Result result = socket_.ioHandle().recvmsg(
        &slice, num_slices, socket_.localAddress()->ip()->port(), output);

    if (!result.ok()) {
      // No more to read or encountered a system error.
      if (result.err_->getErrorCode() != Api::IoError::IoErrorCode::Again) {
        ENVOY_UDP_LOG(error, "recvfrom result {}: {}",
                      static_cast<int>(result.err_->getErrorCode()),
                      result.err_->getErrorDetails());
        cb_.onReceiveError(UdpListenerCallbacks::ErrorCode::SyscallError,
                           result.err_->getErrorCode());
#else //ENVOY
    ReceiveResult recv_result = doRecvFrom(addr, addr_len);
    if ((recv_result.result_.rc_ < 0)) {
#ifdef WIN32
      if (recv_result.result_.errno_ != WSAEWOULDBLOCK) {
#else
      if (recv_result.result_.errno_ != EAGAIN) {
#endif
        cb_.onError(UdpListenerCallbacks::ErrorCode::SyscallError, recv_result.result_.errno_);
#endif
      }
      // Stop reading.
      return;
    }

    if (result.rc_ == 0) {
      // TODO(conqerAtapple): Is zero length packet interesting? If so add stats
      // for it. Otherwise remove the warning log below.
      ENVOY_UDP_LOG(trace, "received 0-length packet");
    }

    RELEASE_ASSERT(output.local_address_ != nullptr, "fail to get local address from IP header");

    if (packets_dropped_ != old_packets_dropped) {
      // The kernel tracks SO_RXQ_OVFL as a uint32 which can overflow to a smaller
      // value. So as long as this count differs from previously recorded value,
      // more packets are dropped by kernel.
      uint32_t delta = (packets_dropped_ > old_packets_dropped)
                           ? (packets_dropped_ - old_packets_dropped)
                           : (packets_dropped_ +
                              (std::numeric_limits<uint32_t>::max() - old_packets_dropped) + 1);
      // TODO(danzh) add stats for this.
      ENVOY_UDP_LOG(debug, "Kernel dropped {} more packets. Consider increase receive buffer size.",
                    delta);
    }
<<<<<<< HEAD
=======
    case AF_INET6: {
      const struct sockaddr_in6* sin6 = reinterpret_cast<const struct sockaddr_in6*>(&addr);
      ASSERT(AF_INET6 == sin6->sin6_family);
      if (IN6_IS_ADDR_V4MAPPED(&sin6->sin6_addr)) {
#ifdef __APPLE__
        struct sockaddr_in sin = {
            {}, AF_INET, sin6->sin6_port, {sin6->sin6_addr.__u6_addr.__u6_addr32[3]}, {}};
#elif WIN32
        struct in_addr in_v4 = {};
        in_v4.S_un.S_addr = reinterpret_cast<const uint32_t*>(sin6->sin6_addr.u.Byte)[3];
        struct sockaddr_in sin = {AF_INET, sin6->sin6_port, in_v4, {}};
#else
      struct sockaddr_in sin = {AF_INET, sin6->sin6_port, {sin6->sin6_addr.s6_addr32[3]}, {}};
#endif
        peer_address = std::make_shared<Address::Ipv4Instance>(&sin);
      } else {
        peer_address = std::make_shared<Address::Ipv6Instance>(*sin6, true);
      }
>>>>>>> Envoy builds on Windows

    // Adjust used memory length.
    slice.len_ = std::min(slice.len_, static_cast<size_t>(result.rc_));
    buffer->commit(&slice, 1);

    ENVOY_UDP_LOG(trace, "recvmsg bytes {}", result.rc_);

    RELEASE_ASSERT(output.peer_address_ != nullptr,
                   fmt::format("Unable to get remote address for fd: {}, local address: {} ",
                               socket_.ioHandle().fd(), socket_.localAddress()->asString()));

    // Unix domain sockets are not supported
    RELEASE_ASSERT(output.peer_address_->type() == Address::Type::Ip,
                   fmt::format("Unsupported remote address: {} local address: {}, receive size: "
                               "{}",
                               output.peer_address_->asString(), socket_.localAddress()->asString(),
                               result.rc_));

    UdpRecvData recvData{std::move(output.local_address_), std::move(output.peer_address_),
                         std::move(buffer), receive_time};
    cb_.onData(recvData);
  } while (true);
}

void UdpListenerImpl::handleWriteCallback() {
  ENVOY_UDP_LOG(trace, "handleWriteCallback");
  cb_.onWriteReady(socket_);
}

Event::Dispatcher& UdpListenerImpl::dispatcher() { return dispatcher_; }

<<<<<<< HEAD
const Address::InstanceConstSharedPtr& UdpListenerImpl::localAddress() const {
  return socket_.localAddress();
}
=======
  } while (true);
} // namespace Network
>>>>>>> Envoy builds on Windows

Api::IoCallUint64Result UdpListenerImpl::send(const UdpSendData& send_data) {
  ENVOY_UDP_LOG(trace, "send");
  Buffer::Instance& buffer = send_data.buffer_;
  uint64_t num_slices = buffer.getRawSlices(nullptr, 0);
  STACK_ARRAY(slices, Buffer::RawSlice, num_slices);
  buffer.getRawSlices(slices.begin(), num_slices);
  Api::IoCallUint64Result send_result =
      socket_.ioHandle().sendmsg(slices.begin(), num_slices, 0, *send_data.send_address_);

  if (send_result.ok()) {
    ASSERT(send_result.rc_ == buffer.length());
    ENVOY_UDP_LOG(trace, "sendmsg sent:{} bytes", send_result.rc_);
  } else {
    ENVOY_UDP_LOG(debug, "sendmsg failed with error {}. Ret {}",
                  static_cast<int>(send_result.err_->getErrorCode()), send_result.rc_);
  }

  // The send_result normalizes the rc_ value to 0 in error conditions.
  // The drain call is hence 'safe' in success and failure cases.
  buffer.drain(send_result.rc_);

  return send_result;
}

} // namespace Network
} // namespace Envoy
