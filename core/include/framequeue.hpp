#pragma once
#include <array>
#include <queue>
#include <mutex>
#include "common.hpp"

typedef std::array<u32, 256*240> Frame;

/*
    Active frame - frame, that currently is processed by PPU.
    Render frame - frame, that should be rendered by the renderer.
*/
template<std::size_t N>
class FrameQueue {
public:
    using Frames = std::array<Frame, N>;
    FrameQueue()
        : activeFrame{0} {}
    inline Frame& getActiveFrame() { return frames[activeFrame]; }
    FrameQueue& incrementActiveFrame() {
        if (++activeFrame == N) activeFrame = 0;
        return *this;
    }
    FrameQueue& pushActiveFrameToQueue() {
        std::lock_guard<std::mutex> lck(queueMtx);
        renderFramesQueue.push(&frames[activeFrame]);
        return *this;
    }
    Frame* getRenderFrame() {
        std::lock_guard<std::mutex> lck(queueMtx);
        if(!renderFramesQueue.empty()) {
            Frame* res = renderFramesQueue.front();
            renderFramesQueue.pop();
            return res;
        }
        // renderer should wait, if there is no more frames to render
        else {
            return nullptr;
        }
    }
private:
    Frames frames;
    std::queue<Frame*> renderFramesQueue;
    u32 activeFrame;

    std::mutex queueMtx;
};
