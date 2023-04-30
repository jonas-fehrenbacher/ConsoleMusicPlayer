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

    class StateMachine 
    {
    public:
        std::vector<State*> states;

        void add(State* state);
        void remove(State* state);
        void update();
        void handleEvent();
        void draw();
    };
}