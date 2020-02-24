#ifndef COMPONENT_CPP
#define COMPONENT_CPP

#include "systemc.h"
#include "map"
#include "vector"
#include <iostream>

struct StatelessComponent : public sc_module
{
    
    sc_in<bool> clk;   // Clock input of the design
    sc_in<bool> reset; // active high, synchronous Reset input
    sc_in<bool> enable;

    //------------Code Starts Here-------------------------
    virtual void fn_main()
    {
        
    }

    StatelessComponent(sc_module_name name,  const sc_signal<bool>& _clk, const sc_signal<bool>& _reset, const sc_signal<bool>& _enable) : sc_module(name)
    {
        SC_METHOD(fn_main);
        sensitive << reset;
        sensitive << clk.pos();
        this->clk(_clk);
        this->reset(_reset);
        this->enable(_enable);
    }

    SC_HAS_PROCESS(StatelessComponent);

};

struct StatefulComponent : public sc_module
{
    
    sc_in<bool> clk;   // Clock input of the design
    sc_in<bool> reset; // active high, synchronous Reset input
    sc_in<bool> enable;

    //------------Code Starts Here-------------------------
    virtual void fn_main()
    {
        
    }

    virtual void reset_main()
    {

    }

    StatefulComponent(sc_module_name name,  const sc_signal<bool>& _clk, const sc_signal<bool>& _reset, const sc_signal<bool>& _enable) : sc_module(name)
    {
        SC_THREAD(fn_main);
        sensitive << clk.pos();

        SC_THREAD(reset_main)
        sensitive << reset;

        this->clk(_clk);
        this->reset(_reset);
        this->enable(_enable);
    }

    SC_HAS_PROCESS(StatefulComponent);

};



#endif