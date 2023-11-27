#ifndef ARCHITECT_STREAM_H
#define ARCHITECT_STREAM_H


namespace flow {
class Stream {
public:
    Stream()          = default;
    virtual ~Stream() = default;

    virtual bool Sync() = 0;

    virtual void* GetDeviceStream() = 0;
};

} // namespace flow


#endif // ARCHITECT_STREAM_H
