#pragma once
#include <queue>

enum class EventType {
    InterruptNMI,
    OAMDMAWrite,
};

class EventQueue {
public:
    inline std::queue<EventType>& get() { return queue; }
private:
    std::queue<EventType> queue;
};
