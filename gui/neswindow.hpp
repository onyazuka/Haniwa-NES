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

class NESWindow : public QMainWindow, public Observer<PPU>
{
    Q_OBJECT
public:
    NESWindow(Logger* logger, QWidget *parent = 0);
    ~NESWindow();

    inline const QWidget* getRenderWidget () { return renderWidget; }
    inline NESWindow& setRenderer(GuiSDL* sdlGui) { renderer = sdlGui; return *this; }
    void loadRom(const std::string& romName);

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent* event);
    bool event(QEvent* event);
    void update(PPU*, int);
private:
    void createMenuActions();
    void createMenu();

    void cpuWork();
    void startCpu();
    void stopCpu();

    Uptr<NES> nes;
    QWidget* renderWidget;
    GuiSDL* renderer;
    Logger* logger;
    QMenu* mainMenu;
    QAction* openAction;
    QAction* saveAction;
    QAction* loadAction;
    QAction* pauseAction;
    QAction* exitAction;

    bool cpuStopped;
    bool cpuPaused;
    std::thread cpuThread;

private slots:
    void openNES();
    void saveGame();
    void loadGame();
    void pause();
    void resume();
    void togglePause();
    void exit();
};
