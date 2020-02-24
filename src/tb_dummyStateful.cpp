#include "systemc.h"
#include "Dummy_StatefulComp.cpp"
#include "assert.h"

using std::cout;
using std::endl;

#define MAX_RESET_CYCLES 10
#define MAX_SIM_CYCLES 10

#define PE_CURRENT_WEIGHT 2

/**
 * @brief Simulation entry point note sc_main not main.... 
 * a systemc idiosyncrasy related to the linking behavior. 
 * 
 * @param argc number of arguments passed (just like regular C )
 * @param argv arguments passed (just like regular C )
 * @return int 
 */
int sc_main(int argc, char *argv[]) 
{
    /**
     * @brief Module instantiated with a name. This is necessary because the
     * runtime may pick up on runtime errors and describe them in relation to
     * signals connected to modules. 
     */
    sc_signal<bool > clk("clk"); 
    sc_signal<bool > reset("reset"); 
    sc_signal<bool > enable("enable"); 
    sc_signal<sc_int<32> > counter("counter"); 

    DummyStatefulComponent dummy("dummy", clk, reset, enable, counter);
    /**
     * @brief Here's how you log specific signals you would want to watch in
     * gtkwave. Just call sc_trace on whatever signal you want and give it a
     * name so that you can find it in the signal heirarchy in gtkwave.
     * 
     */

    sc_trace_file *wf = sc_create_vcd_trace_file("./src/traces/sim_signals.trace");
    sc_trace(wf, clk, "clk");
    sc_trace(wf, reset, "reset");
    sc_trace(wf, enable, "enable");
    sc_trace(wf, counter, "counter");

    reset = 0;
    /**
     * @brief Steps the simulation one ns forward. The SC_NS value is an enum
     * and it can be milliseconds or seconds. 
     * 
     */
    sc_start(1, SC_NS);

    reset = 1;
    cout << "@ " << sc_time_stamp() << " Asserting reset" << endl;
    for (int i = 0; i < MAX_RESET_CYCLES; i++)
    {
        sc_start(1, SC_NS); 
    }
    cout << "@ " << sc_time_stamp() << " Deasserting reset" << endl;
    reset = 0; 
    
    cout << "@ " << sc_time_stamp() << " Start Compute" << endl;
    enable = 1;

    for (int i = 0; i < MAX_SIM_CYCLES; i++)
    {
        if(i > (MAX_SIM_CYCLES / 2) - 1)
        {
            enable = 0;
        }
        clk = 0;
        sc_start(1, SC_NS);

        clk = 1;
        sc_start(1, SC_NS);
    }

    enable = 1;

    for (int i = 0; i < MAX_SIM_CYCLES; i++)
    {
        if(i > (MAX_SIM_CYCLES / 2) - 1)
        {
            reset = 1;
        }        clk = 0;
        sc_start(1, SC_NS);
        clk = 1;
        sc_start(1, SC_NS);
    }



    return 0; // Terminate simulation
}

