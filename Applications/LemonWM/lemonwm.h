#pragma once

#include <gfx/graphics.h>
#include <gfx/surface.h>

#include <core/msghandler.h>
#include <core/event.h>
#include <gui/window.h>

#include <list>

#define WINDOW_BORDER_COLOUR {32,32,32}
#define WINDOW_TITLEBAR_HEIGHT 24
#define WINDOW_BORDER_THICKNESS 2

using WindowBuffer = Lemon::GUI::WindowBuffer;

class WMInstance;

enum WMButtonState{
    ButtonStateUp,
    ButtonStateHover,
    ButtonStatePressed,
};

enum ResizePoint {
    Top,
    Bottom,
    Left,
    Right,
    BottomLeft,
    BottomRight,
    TopLeft,
    TopRight,
};

class WMWindow {
protected:
    unsigned long sharedBufferKey;

    WindowBuffer* windowBufferInfo;
    uint8_t* buffer1;
    uint8_t* buffer2;
    unsigned long bufferKey;

    WMInstance* wm;

    rect_t closeRect, minimizeRect;
public:
    WMWindow(WMInstance* wm, unsigned long key);
    ~WMWindow();

    vector2i_t pos;
    vector2i_t size;
    char* title;
    uint32_t flags = 0;
    bool minimized = false;

    short closeBState = ButtonStateUp; 
    short minimizeBState = ButtonStateUp;

    int clientFd = 0;

    void Draw(surface_t* surface);

    void Minimize(bool state);
    void Resize(vector2i_t size, unsigned long bufferKey);
    void RecalculateRects();

    rect_t GetCloseRect();
    rect_t GetMinimizeRect();

    rect_t GetBottomBorderRect();
    rect_t GetTopBorderRect();
    rect_t GetLeftBorderRect();
    rect_t GetRightBorderRect();

    void RecalculateButtonRects();
};

struct MouseState {
    vector2i_t pos;
    bool left, middle, right;
};

struct KeyboardState{
	bool caps, control, shift, alt;
};

class InputManager{
protected:
    WMInstance* wm;

public:
    MouseState mouse;
    KeyboardState keyboard;

    InputManager(WMInstance* wm);
    void Poll();
};

class CompositorInstance{
protected:
    WMInstance* wm;

public:
    CompositorInstance(WMInstance* wm);
    void Paint();

    surface_t windowButtons;
};

class WMInstance {
protected:
    Lemon::MessageServer server;
    Lemon::MessageClient shellClient;

    WMWindow* active;
    bool drag = false;
    vector2i_t dragOffset;
    bool resize = false;
    int resizePoint = Right;
    vector2i_t resizeStartPos;

    void Poll();
    void PostEvent(Lemon::LemonEvent& ev, WMWindow* win);
    WMWindow* FindWindow(int id);

    void MinimizeWindow(WMWindow* win, bool state);
    void MinimizeWindow(int id, bool state);
public:
    surface_t surface;

    std::list<WMWindow*> windows;

    InputManager input = InputManager(this);
    CompositorInstance compositor = CompositorInstance(this);

    WMInstance(surface_t& surface, sockaddr_un address);
    
    void Update();

    void MouseDown();
    void MouseUp();
    void MouseMove();
    void KeyUpdate(int key, bool pressed);
};

static inline bool PointInWindow(WMWindow* win, vector2i_t point){
	int windowHeight = (win->flags & WINDOW_FLAGS_NODECORATION) ? win->size.y : (win->size.y + WINDOW_TITLEBAR_HEIGHT + (WINDOW_BORDER_THICKNESS * 3)); // Account for titlebar and borders
	int windowWidth = (win->flags & WINDOW_FLAGS_NODECORATION) ? win->size.x : (win->size.x + (WINDOW_BORDER_THICKNESS * 3)); // Account for borders and extend the window a little bit so it is easier to resize

    vector2i_t windowOffset = (win->flags & WINDOW_FLAGS_NODECORATION) ? (vector2i_t){0, 0} : (vector2i_t){-1, -1};

	return Lemon::Graphics::PointInRect({{win->pos + windowOffset},{windowWidth, windowHeight}}, point);
}

static inline bool PointInWindowProper(WMWindow* win, vector2i_t point){
	int windowYOffset = (win->flags & WINDOW_FLAGS_NODECORATION) ? 0 : (WINDOW_TITLEBAR_HEIGHT + WINDOW_BORDER_THICKNESS); // Account for titlebar
    int windowXOffset = (win->flags & WINDOW_FLAGS_NODECORATION) ? 0 : (WINDOW_BORDER_THICKNESS); // Account for titlebar
	return Lemon::Graphics::PointInRect({{win->pos + (vector2i_t){windowXOffset, windowYOffset}},{win->size.x, win->size.y}}, point);
}