#include "gui/neswindow.hpp"
#include <iostream>

NESWindow::NESWindow(Logger* logger, QWidget *parent) :
    QMainWindow(parent), nes{nullptr}, renderer{nullptr}, logger{logger}, cpuStopped{false}, cpuPaused{false} {
    renderWidget = new QWidget();
    setCentralWidget(renderWidget);
    renderWidget->setFixedSize(800, 600);
    this->setFixedSize(800, 600);
    // this is HITSUYOU to prevent flickering (conflict with SDL)
    renderWidget->setUpdatesEnabled(false);

    createMenuActions();
    createMenu();

}

NESWindow::~NESWindow() {
    stopCpu();
}

void NESWindow::loadRom(const std::string& romName) {
    stopCpu();
    if(nes) nes->getPpu().detach(this);
    nes = std::move(Uptr<NES>(new NES(romName, logger)));
    nes->getPpu().attach(this);
    startCpu();
}

void NESWindow::update(PPU*, int eventType) {
    if (renderer && (eventType == (int)PPUEvent::RerenderMe)) {
        QApplication::postEvent(this, new RenderEvent(), Qt::HighEventPriority);
    }
}

void NESWindow::keyPressEvent(QKeyEvent *event) {
    if(!nes) return;
    // ignoring duplicates
    if(event->isAutoRepeat()) {
        return;
    }
    switch(event->key()) {
        case Qt::Key_Left: nes->getController(0).updateKey(StandardController::Key::Left, true); break;
        case Qt::Key_Up: nes->getController(0).updateKey(StandardController::Key::Up, true); break;
        case Qt::Key_Right: nes->getController(0).updateKey(StandardController::Key::Right, true); break;
        case Qt::Key_Down: nes->getController(0).updateKey(StandardController::Key::Down, true); break;
        case Qt::Key_Return: nes->getController(0).updateKey(StandardController::Key::Start, true); break;
        case Qt::Key_Space: nes->getController(0).updateKey(StandardController::Key::Select, true); break;
        case Qt::Key_Z: nes->getController(0).updateKey(StandardController::Key::A, true); break;
        case Qt::Key_X: nes->getController(0).updateKey(StandardController::Key::B, true); break;
        default: break;
    }
}

void NESWindow::keyReleaseEvent(QKeyEvent *event) {
    if(!nes) return;
    // ignoring duplicates
    if(event->isAutoRepeat()) {
        return;
    }
    switch(event->key()) {
        case Qt::Key_Left: nes->getController(0).updateKey(StandardController::Key::Left, false); break;
        case Qt::Key_Up: nes->getController(0).updateKey(StandardController::Key::Up, false); break;
        case Qt::Key_Right: nes->getController(0).updateKey(StandardController::Key::Right, false); break;
        case Qt::Key_Down: nes->getController(0).updateKey(StandardController::Key::Down, false); break;
        case Qt::Key_Return: nes->getController(0).updateKey(StandardController::Key::Start, false); break;
        case Qt::Key_Space: nes->getController(0).updateKey(StandardController::Key::Select, false); break;
        case Qt::Key_Z: nes->getController(0).updateKey(StandardController::Key::A, false); break;
        case Qt::Key_X: nes->getController(0).updateKey(StandardController::Key::B, false); break;
        default: break;
    }
}

bool NESWindow::event(QEvent* event) {
    switch(event->type()) {
    // request to SDL to rerender
    case RenderEvent::Type: if(renderer) renderer->render(nes->getPpu().getRenderFrame()); return true;
    default: break;
    }
    return QWidget::event(event);
}

void NESWindow::createMenuActions() {
    openAction = new QAction("&Open", this);
    openAction->setShortcuts(QKeySequence::New);
    openAction->setStatusTip("Open NES ROM");
    connect(openAction, SIGNAL(triggered(bool)), this, SLOT(openNES()));

    saveAction = new QAction("&Save", this);
    saveAction->setShortcuts(QKeySequence::Save);
    saveAction->setStatusTip("Save game");
    connect(saveAction, SIGNAL(triggered(bool)), this, SLOT(saveGame()));

    loadAction = new QAction("&Load", this);
    loadAction->setShortcuts(QKeySequence::Open);
    loadAction->setStatusTip("Load game");
    connect(loadAction, SIGNAL(triggered(bool)), this, SLOT(loadGame()));

    pauseAction = new QAction("&Pause/Resume", this);
    pauseAction->setShortcut(QKeySequence::fromString("p"));
    pauseAction->setStatusTip("Pause/Resume game");
    connect(pauseAction, SIGNAL(triggered(bool)), this, SLOT(togglePause()));

    exitAction = new QAction("&Quit", this);
    exitAction->setShortcuts(QKeySequence::Quit);
    exitAction->setStatusTip("Quit HaniwaNES");
    connect(exitAction, SIGNAL(triggered(bool)), this, SLOT(exit()));
}

void NESWindow::createMenu() {
    mainMenu = menuBar()->addMenu("&Menu");
    mainMenu->addAction(openAction);
    mainMenu->addAction(saveAction);
    mainMenu->addAction(loadAction);
    mainMenu->addAction(pauseAction);
    mainMenu->addAction(exitAction);
}

void NESWindow::cpuWork() {
    while (!cpuStopped) {
        if (!cpuPaused) nes->doInstruction();
        else std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void NESWindow::startCpu() {
    cpuStopped = false;
    cpuThread = std::thread([this]() {
        cpuWork();
    });
}

void NESWindow::stopCpu() {
    cpuStopped = true;
    if (cpuThread.joinable()) cpuThread.join();
}

void NESWindow::openNES() {
    // we should set pause to process events(other way, because of rendering, dialog window will open slowly)
    pause();
    QString openFname = QFileDialog::getOpenFileName(this, "Open ROM", "./", "NES ROMs (*.nes)");
    if(!openFname.isEmpty()) {
        loadRom(openFname.toStdString());
    }
    resume();
}

void NESWindow::saveGame() {
    pause();
    QString saveFname = QFileDialog::getSaveFileName(this, "Save game", "./", "HaniwaNES saves (*.hns)");
    if(!saveFname.isEmpty()) {
        nes->save(saveFname.toStdString());
    }
    resume();
}

void NESWindow::loadGame() {
    stopCpu();
    QString loadFname = QFileDialog::getOpenFileName(this, "Load game", "./", "HaniwaNES saves (*.hns)");
    if(!loadFname.isEmpty()) {
        nes->load(loadFname.toStdString());
    }
    startCpu();
}

void NESWindow::togglePause() {
    cpuPaused = !cpuPaused;
}

void NESWindow::pause() {
    cpuPaused = true;
}

void NESWindow::resume() {
    cpuPaused = false;
}

void NESWindow::exit() {
    QApplication::exit();
}


