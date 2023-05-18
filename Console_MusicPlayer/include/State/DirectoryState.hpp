#pragma once
#include "core/ScrollableList.hpp"

class App;

class DirectoryState
{
public:
    void init(App* app);
    void terminate();
    void update();
    void handleEvent();
    void draw();
    void loseFocus();
    void gainFocus();
    bool isTrappedOnTop();
    void scrollToTop();
    void onConsoleResize();
private:
    App*                 app;
    core::ScrollableList list;
};