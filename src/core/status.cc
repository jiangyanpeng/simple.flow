#include "core/status.h"
namespace flow {
int GetStatusCodeValue(StatusCode code) {
    return static_cast<int>(code);
}

Status::Status() : code_(StatusCode::kOk), msg_("ok") {}

Status::Status(StatusCode code, std::string msg) : code_(code), msg_(std::move(msg)) {}

Status::~Status() {}

Status::Status(const Status& rhs) = default;

Status& Status::operator=(const Status& rhs) {
    if (this != &rhs) {
        code_ = rhs.code_;
        msg_  = rhs.msg_;
    }
    return *this;
}

Status::Status(Status&& rhs) noexcept {
    code_ = rhs.code_;
    msg_  = rhs.msg_;
}

Status& Status::operator=(Status&& rhs) {
    code_ = rhs.code_;
    msg_  = rhs.msg_;
    return *this;
}

Status Status::OkStatus() {
    return Status();
}

Status Status::InvalidArgument(std::string msg) {
    return Status(StatusCode::kInvalidArgument, msg);
}

StatusCode Status::Code() const {
    return code_;
}

std::string Status::Msg() const {
    return msg_;
}

bool Status::IsOk() const {
    return code_ == StatusCode::kOk;
}

} // namespace flow
