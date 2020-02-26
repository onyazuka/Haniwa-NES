TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++17

INCLUDEPATH += /usr/local/include
LIBS += -L/usr/local/lib -lSDL2 -ldl -lpthread

SOURCES += main.cpp \
    core/cpu.cpp \
    core/memory.cpp \
    log/log.cpp \
    core/rom.cpp \
    core/mappers/mapper0.cpp \
    core/common.cpp \
    core/ppu.cpp \
    core/ppumemory.cpp \
    core/mappers/mapperinterface.cpp \
    nes.cpp \
    core/mappers/mappers.cpp \
    serialize/serializer.cpp \
    core/input.cpp

HEADERS += \
    core/include/cpu.hpp \
    core/include/common.hpp \
    core/include/memory.hpp \
    log/log.hpp \
    core/include/rom.hpp \
    core/include/mappers/mapper0.hpp \
    core/include/mappers/mapperinterface.hpp \
    core/include/mappers/mappers.hpp \
    debug/debug.hpp \
    core/include/ppu.hpp \
    core/include/ppumemory.hpp \
    core/include/eventqueue.hpp \
    observer/observer.hpp \
    nes.hpp \
    serialize/serializer.hpp \
    core/include/input.hpp
