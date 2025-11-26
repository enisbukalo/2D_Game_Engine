#pragma once

#include "Input/InputEvents.h"

class IInputListener
{
public:
    virtual ~IInputListener() {}
    virtual void onKeyPressed(const KeyEvent& ev) {}
    virtual void onKeyReleased(const KeyEvent& ev) {}
    virtual void onMousePressed(const MouseEvent& ev) {}
    virtual void onMouseReleased(const MouseEvent& ev) {}
    virtual void onMouseMoved(const MouseMoveEvent& ev) {}
    virtual void onTextEntered(const TextEvent& ev) {}
    virtual void onWindowEvent(const WindowEvent& ev) {}
    virtual void onAction(const ActionEvent& ev) {}
};
