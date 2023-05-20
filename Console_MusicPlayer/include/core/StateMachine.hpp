#pragma once

#include <vector>

namespace core
{
    class State 
    {
    public:
        State();
        virtual void init() = 0;
        virtual void terminate() = 0;
        virtual void update() = 0;
        virtual void handleEvent() = 0;
        virtual void draw() = 0;
        bool isUpdating;
        bool isDrawing;
    protected:
        ~State();
    };

    /** Useful if there can be only one state active at a time. */
    class ActiveState 
    {
    public:
        ActiveState();
        /** Calls automatically State::terminate() on the old state and State::init() on the new one. Use nullptr to remove the current state. */
        void set(State* state);
        void update();
        void handleEvents();
        void draw();
    private:
        State* data;
    };
}