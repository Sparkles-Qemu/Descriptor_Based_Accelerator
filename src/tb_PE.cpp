#include "systemc.h"
#include "PE.cpp"

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
    sc_signal<sc_int<PE::PE_ACCUMULATION_PRECISION> > psumIn("psumIn");
    sc_signal<sc_int<PE::PE_OPERAND_PRECISION> > pixelIn0("pixelIn0");
    sc_signal<sc_int<PE::PE_ACCUMULATION_PRECISION> > psumOut("psumOut");

    PE pe1("pe1", clk, reset, enable, psumIn, pixelIn0, psumOut);

    pe1.current_weight = PE_CURRENT_WEIGHT;

    /**
     * @brief Here's how you log specific signals you would want to watch in
     * gtkwave. Just call sc_trace on whatever signal you want and give it a
     * name so that you can find it in the signal heirarchy in gtkwave.
     * 
     */

    sc_trace_file *wf = sc_create_vcd_trace_file("./src/traces/sim_signals.trace");
    wf->set_time_unit(500, sc_core::SC_PS);
    sc_trace(wf, clk, "clk");
    sc_trace(wf, reset, "reset");
    sc_trace(wf, psumIn, "psumIn");
    sc_trace(wf, pixelIn0, "pixelIn0");    
    sc_trace(wf, psumOut, "psumOut");
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
        /**
         * @brief Since rest is level triggered, update will only be called once
         * 
         */
        sc_start(1, SC_NS); 
    }
    cout << "@ " << sc_time_stamp() << " Deasserting reset" << endl;
    reset = 0; 
    psumIn = 1;
    pixelIn0 = 1;
    sc_start(1, SC_NS); 

    
    cout << "@ " << sc_time_stamp() << " Start Compute" << endl;
    for (int i = 0; i < MAX_SIM_CYCLES; i++)
    {
        psumIn = (i+1);
        pixelIn0 = (i+1);
        clk = 0;
        sc_start(1, SC_NS);
        clk = 1;
        sc_start(1, SC_NS);
        cout << "@ " << sc_time_stamp() << " psum_out: " << psumOut.read()  << endl;
        assert(psumOut.read() == (i+1) + (i+1)*PE_CURRENT_WEIGHT);
    }

    cout << "@ " << sc_time_stamp() << " Done with compute, testing async reset" << endl;
    
    /**
     * @brief Random reset not tied to a particular clk cycle. Note change in precision
     * when outputing sim time in module.
     * 
     */
    cout << "@ " << sc_time_stamp() << " Compute complete, testing async reset" << endl;
    sc_start(1.5, SC_NS); 
    reset = 1;
    sc_start(1.5, SC_NS);
    
    cout << "@ " << sc_time_stamp() << " Sim complete, Simulation terminating .... " << endl;


    return 0; // Terminate simulation
}

