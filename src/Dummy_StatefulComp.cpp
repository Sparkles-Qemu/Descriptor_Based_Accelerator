
#ifndef Dummy_StatefulComp_CPP
#define Dummy_StatefulComp_CPP

#include "systemc.h"
#include "map"
#include "vector"
#include "COMPONENT.cpp"

struct DummyStatefulComponent : StatefulComponent
{
    //------------Define Functions Here---------------------
    sc_out<sc_int< 32> > count_val;

    /**
     * @brief function automatically called on reset assertion
     * main automatically restarts
     * 
     */
    void fn_reset()
    {
        cout << "@ " << sc_time_stamp() << " Dummy Reset" << endl;
        count_val.write(0);
    }

    /**
     * @brief Main loop thread. Will keep iterating indefinitely unless
     * encounters a pause point that checks if enable is reset and pauses
     * function or encounters a reset which will restart the function. Otherwise
     * if no disable is received after the next n cycles will wake it up.
     * 
     */
    void fn_main()
    {
        cout << "@ " << sc_time_stamp() << " Dummy Writing " << count_val.read()+1 << endl;
        count_val.write(count_val.read()+1);

        wait_on_clk_or_pause_if_disable();

        cout << "@ " << sc_time_stamp() << " Dummy Writing " << count_val.read()+1 << endl;
        count_val.write(count_val.read()+1);
    }

    DummyStatefulComponent(
        ::sc_core::sc_module_name name, 
        const sc_signal<bool>& _clk, 
        const sc_signal<bool>& _reset, 
        const sc_signal<bool>& _enable,
        sc_signal<sc_int<32> >& _counter
    ) : StatefulComponent(name, _clk, _reset, _enable)
    {
        this->count_val(_counter);
        cout << "Dummy StatefulComponent Instantiated " << endl;
    }

}; // End of Module PE
#endif