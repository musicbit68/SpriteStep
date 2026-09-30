#pragma once
namespace ss {
class AudioEngine {
public:
    bool start() { running_ = true; return true; }
    void stop() { running_ = false; }
    bool running() const { return running_; }
private:
    bool running_ = false;
};
}
