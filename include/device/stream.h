#ifndef SIMPLE_FLOW_STREAM_H_
#define SIMPLE_FLOW_STREAM_H_

namespace flow {
class Stream {
public:
    Stream()          = default;
    virtual ~Stream() = default;

    virtual bool Sync() = 0;

    virtual void* GetDeviceStream() = 0;
};

} // namespace flow
#endif // SIMPLE_FLOW_STREAM_H_
