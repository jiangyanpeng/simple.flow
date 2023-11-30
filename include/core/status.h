#ifndef SIMPLE_FLOW_STATUS_H_
#define SIMPLE_FLOW_STATUS_H_

#include <string>

namespace flow {
enum class StatusCode : int {
    kOk                = 0,
    kCancelled         = 1,
    kUnknown           = 2,
    kInvalidArgument   = 3,
    kNotFound          = 4,
    kAlreadyExists     = 5,
    kPermissionDenied  = 6,
    kResourceExhausted = 7,
    kAborted           = 8,
    kOutOfRange        = 9,
    kUnimplemented     = 10,
    kInternal          = 11,
    kFlowControl       = 12,

};

int GetStatusCodeValue(StatusCode code);
class Status final {
public:
    Status();
    Status(StatusCode code, std::string msg);

    ~Status();
    Status(const Status& rhs);
    Status& operator=(const Status& rhs);
    Status(Status&& rhs) noexcept;
    Status& operator=(Status&& rhs);

    StatusCode Code() const;
    std::string Msg() const;

    bool IsOk() const;

    static Status OkStatus();
    static Status InvalidArgument(std::string msg);

private:
    StatusCode code_;
    std::string msg_;
};
} // namespace flow
#endif // SIMPLE_FLOW_STATUS_H_
