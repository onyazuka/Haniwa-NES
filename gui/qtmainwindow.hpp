#pragma once
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <mutex>
#include <condition_variable>
#include "observer/observer.hpp"
#include "core/include/ppu.hpp"
#include "gui/sdlgui.hpp"

class RenderEvent : public QEvent {
public:
    static const QEvent::Type Type = static_cast<QEvent::Type>(QEvent::User + 0);
    RenderEvent() : QEvent(RenderEvent::type()) {}
    virtual ~RenderEvent() {}
    inline QEvent::Type type() const { return Type; }
};

class QtMainWindow : public QMainWindow, public Observer<PPU>
{
    Q_OBJECT
public:
    QtMainWindow(NES& nes, QWidget *parent = 0);
    inline const QWidget* getRenderWidget () { return renderWidget; }
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent* event);

    inline QtMainWindow& setRenderer(GuiSDL* sdlGui) { renderer = sdlGui; return *this; }
    bool event(QEvent* event);
    void update(PPU*, int);
private:
    NES& nes;
    QWidget* renderWidget;
    GuiSDL* renderer;

    std::mutex redrawMtx;
    std::condition_variable eventStepCv;

    bool redraw;
};
