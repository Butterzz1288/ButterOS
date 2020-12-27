#include <lemon/core/shell.h>
#include <lemon/gui/window.h>
#include <lemon/ipc/interface.h>

#include <map>

class ShellWindow{
public:
	long id;
	std::string title;
	short state;
	short lastState;
};

class ShellInstance {
    Lemon::Interface shellSrv;

    Lemon::GUI::Window* taskbar;
    Lemon::GUI::Window* menu;

    void PollCommands();
public:
    std::map<long, ShellWindow*> windows;
    ShellWindow* active = nullptr;
    bool showMenu = true;

    ShellInstance(handle_t svc, const char* ifName);

    void SetMenu(Lemon::GUI::Window* menu);
    void SetTaskbar(Lemon::GUI::Window* taskbar);

    void Update();
    void Open(char* path);

    void SetWindowState(ShellWindow* win);

    void(*AddWindow)(ShellWindow*) = nullptr;
    void(*RemoveWindow)(ShellWindow*) = nullptr;
    void(*RefreshWindows)(void) = nullptr;
};