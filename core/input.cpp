#include "core/include/input.hpp"

StandardController::StandardController()
    : status{0}, keysStatus{0}, _strobe{false}, lowStrobeRead{0} {}

 StandardController& StandardController::strobe(bool high) {
     _strobe = high;
     status = keysStatus;
     // resetting counters
     lowStrobeRead = 0;
     return *this;
 }

StandardController& StandardController::updateKey(Key key, bool pressed) {
    keysStatus ^= keysStatus & (1 << (int)key);
    if(pressed) keysStatus |= (1 << (int)key);
    return *this;
}

bool StandardController::read() {
    // returns 1 if everything has already been read
    if(lowStrobeRead == 8) return true;
    // returns status of key A if strobe is high
    if(_strobe) return status & 1;
    bool res = status & 1;
    status >>= 1;
    return res;
}
