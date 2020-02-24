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

    void wait_on_clk_or_pause_if_disable()
    {
        wait();
        if(enable.read() == false)
        {
            while(true)
            {
                wait();
                if(enable.read() == true && clk.read() == true)
                {
                    break;
                }
            }
        }
    }

    void main_thread()
    {
        while(true)
        {
            if(reset.read() == true)
            {
                fn_reset();
            }
            else if(enable.read() == true && clk.read() == true)
            {
                fn_main();
            }
            wait();
        }
    }

    virtual void fn_reset()
    {

    }

    virtual void fn_main()
    {
        
    }
    
    SC_HAS_PROCESS(StatefulComponent);

    StatefulComponent(sc_module_name name,  const sc_signal<bool>& _clk, const sc_signal<bool>& _reset, const sc_signal<bool>& _enable) : sc_module(name)
    {
        SC_THREAD(main_thread);
        sensitive << clk.pos();
        // sensitive << reset;
        sensitive << enable;
        async_reset_signal_is(reset,true);
        dont_initialize(); 
        this->clk(_clk);
        this->reset(_reset);
        this->enable(_enable);
    }


};



#endif