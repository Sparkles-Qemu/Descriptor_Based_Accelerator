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
    //------------Local Variables-------------------------
    unsigned int mainPeriod;

    //------------Code Starts Here-------------------------
    /**
     * @brief Waits for n-cycles before checking if not paused. If paused and
     * unpaused briefly while waiting for n-cycles effect is the same. fn_main
     * continue as if nothing happened because effectively the external
     * controller of enable wants the module to continue.
     * 
     * @param cycles Number of cycles before continuing fn_main or checking for
     * and pausing
     */
    void wait_on_clk_or_pause_if_disable(unsigned int cycles)
    {
        wait(cycles);
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
            wait(mainPeriod);
        }
    }

    virtual void fn_reset()
    {

    }

    virtual void fn_main()
    {
        
    }
    
    SC_HAS_PROCESS(StatefulComponent);

    StatefulComponent(sc_module_name name,  const sc_signal<bool>& _clk, const sc_signal<bool>& _reset, const sc_signal<bool>& _enable, unsigned int _mainPeriod) : sc_module(name)
    {
        SC_THREAD(main_thread);
        sensitive << clk.pos();
        sensitive << enable;
        async_reset_signal_is(reset,true);
        dont_initialize(); 
        this->clk(_clk);
        this->reset(_reset);
        this->enable(_enable);
        mainPeriod = _mainPeriod;
    }


};



#endif