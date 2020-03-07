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
     * Signal Decleration
     */
    sc_signal<bool > clk("clk"); 
    sc_signal<bool > reset("reset"); 
    sc_signal<bool > enable("enable"); 
    sc_signal<sc_int<GLOBALS::PE_ACCUMULATION_PRECISION> > psumIn("psumIn");
    sc_signal<sc_int<GLOBALS::PE_OPERAND_PRECISION> > pixelIn0("pixelIn0");
    sc_signal<sc_int<GLOBALS::PE_ACCUMULATION_PRECISION> > psumOut("psumOut");

    /**
     * Component Decleration
     */
    PE<GLOBALS::PE_OPERAND_PRECISION, GLOBALS::PE_ACCUMULATION_PRECISION> pe1("pe1", clk, reset, enable, psumIn, pixelIn0, psumOut);

    /**
     * Loading a weight into a PE
     */
    pe1.current_weight = PE_CURRENT_WEIGHT;

    /**
     * Setting up tracing. Time unit set to 500 for async reset done later
     */
    sc_trace_file *wf = sc_create_vcd_trace_file("./src/traces/sim_signals.trace");
    wf->set_time_unit(500, sc_core::SC_PS);
    sc_trace(wf, clk, "clk");
    sc_trace(wf, reset, "reset");
    sc_trace(wf, psumIn, "psumIn");
    sc_trace(wf, pixelIn0, "pixelIn0");    
    sc_trace(wf, psumOut, "psumOut");
    
    /**
     * First Cycle, nothing happens
     */
    reset = 0;
    sc_start(1, SC_NS);

    /**
     * Resetting Component
     */
    reset = 1;
    cout << "@ " << sc_time_stamp() << " Asserting reset" << endl;
    for (int i = 0; i < MAX_RESET_CYCLES; i++)
    {
        sc_start(1, SC_NS); 
    }
    cout << "@ " << sc_time_stamp() << " Deasserting reset" << endl;
    reset = 0; 
    enable = 1;
    psumIn = 1;
    pixelIn0 = 1;
    sc_start(1, SC_NS); 

    /**
     * Test bench Start
     */
    cout << "@ " << sc_time_stamp() << " Start Compute" << endl;
    for (int i = 0; i < MAX_SIM_CYCLES; i++)
    {
        /**
        * Set Data
        */
        psumIn = (i+1);
        pixelIn0 = (i+1);
        clk = 0;
        sc_start(1, SC_NS);
        
        /**
        * Pulse Clock
        */
        clk = 1;
        sc_start(1, SC_NS);

        /**
        * Validate Output
        */
        cout << "@ " << sc_time_stamp() << " psum_out: " << psumOut.read()  << endl;
        assert(psumOut.read() == (i+1) + (i+1)*PE_CURRENT_WEIGHT);
    }

    /**
    * Test Final Async Reset
    */

    cout << "@ " << sc_time_stamp() << " Compute complete, testing async reset" << endl;
    sc_start(1.5, SC_NS); 
    reset = 1;
    sc_start(1.5, SC_NS);
    
    cout << "@ " << sc_time_stamp() << " Sim complete, Simulation terminating .... " << endl;


    return 0; // Terminate simulation
}

