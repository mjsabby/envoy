#pragma once

#include <memory>
#include <string>

#include "envoy/common/platform.h"
#include "envoy/common/pure.h"

namespace Envoy {
namespace Api {

class IoError;

// IoErrorCode::Again is used frequently. Define it to be a distinguishable address to avoid
// frequent memory allocation of IoError instance.
// If this is used, IoCallResult has to be instantiated with a deleter that does not
// deallocate memory for this error.
#define ENVOY_ERROR_AGAIN reinterpret_cast<Api::IoError*>(0x01)

/**
 * Base class for any I/O error.
 */
class IoError {
public:
  enum class IoErrorCode {
    // No data available right now, try again later.
    Again,
    // Not supported.
    NoSupport,
    // Address family not supported.
    AddressFamilyNoSupport,
    // During non-blocking connect, the connection cannot be completed immediately.
    InProgress,
    // Permission denied.
    Permission,
    // Bad handle
    BadHandle,
    // Other error codes cannot be mapped to any one above in getErrorCode().
    UnknownError
  };
  virtual ~IoError() {}

  // Map platform specific error into IoErrorCode.
  // Needed to hide errorCode() in case of ENVOY_ERROR_AGAIN.
  static IoErrorCode getErrorCode(const IoError& err) {
    if (&err == ENVOY_ERROR_AGAIN) {
      return IoErrorCode::Again;
    }
    return err.errorCode();
  }

  static std::string getErrorDetails(const IoError& err) {
    if (&err == ENVOY_ERROR_AGAIN) {
      return "Try again later";
    }
    return err.errorDetails();
  }

protected:
  virtual IoErrorCode errorCode() const PURE;
  virtual std::string errorDetails() const PURE;
};

using IoErrorDeleterType = void (*)(IoError*);
using IoErrorPtr = std::unique_ptr<IoError, IoErrorDeleterType>;

/**
 * Basic type for return result which has a return code and error code defined
 * according to different implementations.
 * If the call succeeds, ok() should return true and |rc_| is valid. Otherwise |err_|
 * can be passed into IoError::getErrorCode() to extract the error. In this
 * case, |rc_| is invalid.
 */
template <typename ReturnValue> struct IoCallResult {
  IoCallResult(ReturnValue rc, IoErrorPtr err) : rc_(rc), err_(std::move(err)) {}

  IoCallResult(IoCallResult<ReturnValue>&& result) noexcept
      : rc_(result.rc_), err_(std::move(result.err_)) {}

  virtual ~IoCallResult() = default;

  IoCallResult& operator=(IoCallResult&& result) noexcept {
    rc_ = result.rc_;
    err_ = std::move(result.err_);
    return *this;
  }

  /**
   * @return true if the call succeeds.
   */
  bool ok() const { return err_ == nullptr; }

  // TODO(danzh): rename it to be more meaningful, i.e. return_value_.
  ReturnValue rc_;
  //TODO(YECHIEL): Probably get rid of the commented out code below
// template <typename T> struct IoCallResult {
//   IoCallResult(T rc, IoErrorPtr err) : rc_(rc), err_(std::move(err)) {}

//   IoCallResult(IoCallResult<T>&& result) : rc_(result.rc_), err_(std::move(result.err_)) {}

//   virtual ~IoCallResult() {}

//   T rc_;
//   IoErrorPtr err_;
};

using IoCallBoolResult = IoCallResult<bool>;
using IoCallSizeResult = IoCallResult<ssize_t>;
using IoCallUint64Result = IoCallResult<uint64_t>;

inline Api::IoCallUint64Result ioCallUint64ResultNoError() {
  return IoCallUint64Result(0, IoErrorPtr(nullptr, [](IoError*) {}));
}

} // namespace Api
} // namespace Envoy
