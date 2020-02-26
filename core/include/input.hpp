#pragma once
#include "core/include/common.hpp"

class StandardController {
public:

    enum class Key {
        A, B, Select, Start, Up, Down, Left, Right
    };

    StandardController();
    StandardController& strobe(bool high);
    StandardController& updateKey(Key key, bool pressed);
    bool read();
private:
    /*
        Status bits correspond to the following keys:
        0 - A
        1 - B
        2 - Select
        3 - Start
        4 - Up
        5 - Down
        6 - Left
        7 - Right
    */
    // status for CPU
    u8 status;
    // status of keys at the moment
    u8 keysStatus;
    bool _strobe;
    u8 lowStrobeRead;
};
