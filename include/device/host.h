#ifndef SIMPLE_FLOW_STREAM_H_
#define SIMPLE_FLOW_STREAM_H_

namespace flow {
class Host {
public:
    Host()          = default;
    virtual ~Host() = default;

    virtual bool Sync() = 0;

    virtual void* GetDeviceStream() = 0;
};

} // namespace flow
#endif // SIMPLE_FLOW_STREAM_H_
