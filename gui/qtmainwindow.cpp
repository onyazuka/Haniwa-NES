#include "gui/qtmainwindow.hpp"
#include <iostream>

QtMainWindow::QtMainWindow(NES& nes, QWidget *parent) :
    QMainWindow(parent), nes{nes}, renderer{nullptr} {
    renderWidget = new QWidget();
    setCentralWidget(renderWidget);
    this->resize(800, 600);
    renderWidget->resize(800, 600);
}

void QtMainWindow::update(PPU*, int eventType) {
    if ((eventType == (int)PPUEvent::RerenderMe) && renderer) {
        QApplication::postEvent(this, new RenderEvent(), Qt::HighEventPriority);
    }
}

void QtMainWindow::keyPressEvent(QKeyEvent *event) {
    switch(event->key()) {
        case Qt::Key_Left: nes.getController(0).updateKey(StandardController::Key::Left, true); break;
        case Qt::Key_Up: nes.getController(0).updateKey(StandardController::Key::Up, true); break;
        case Qt::Key_Right: nes.getController(0).updateKey(StandardController::Key::Right, true); break;
        case Qt::Key_Down: nes.getController(0).updateKey(StandardController::Key::Down, true); break;
        case Qt::Key_Return: nes.getController(0).updateKey(StandardController::Key::Start, true); break;
        case Qt::Key_Space: nes.getController(0).updateKey(StandardController::Key::Select, true); break;
        case Qt::Key_Z: nes.getController(0).updateKey(StandardController::Key::A, true); break;
        case Qt::Key_X: nes.getController(0).updateKey(StandardController::Key::B, true); break;
        default: break;
    }
}

void QtMainWindow::keyReleaseEvent(QKeyEvent *event) {
    switch(event->key()) {
        case Qt::Key_Left: nes.getController(0).updateKey(StandardController::Key::Left, false); break;
        case Qt::Key_Up: nes.getController(0).updateKey(StandardController::Key::Up, false); break;
        case Qt::Key_Right: nes.getController(0).updateKey(StandardController::Key::Right, false); break;
        case Qt::Key_Down: nes.getController(0).updateKey(StandardController::Key::Down, false); break;
        case Qt::Key_Return: nes.getController(0).updateKey(StandardController::Key::Start, false); break;
        case Qt::Key_Space: nes.getController(0).updateKey(StandardController::Key::Select, false); break;
        case Qt::Key_Z: nes.getController(0).updateKey(StandardController::Key::A, false); break;
        case Qt::Key_X: nes.getController(0).updateKey(StandardController::Key::B, false); break;
        default: break;
    }
}

bool QtMainWindow::event(QEvent* event) {
    switch(event->type()) {
    case RenderEvent::Type: if(renderer) renderer->render(); return true;
    default: break;
    }
    return QWidget::event(event);
}

