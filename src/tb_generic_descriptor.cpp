#include "systemc.h"
#include "PE.cpp"

using std::cout;
using std::endl;

#define MAX_RESET_CYCLES 10
#define MAX_SIM_CYCLES 10

int sc_main(int argc, char *argv[]) 
{

    /**
     * Signal Decleration
     */
    sc_signal<bool > clk("clk"); 
    sc_signal<bool > reset("reset"); 
    sc_signal<bool > enable("enable"); 

    /**
     * Component Decleration
     */

    /**
     * Loading Descriptors into SRAM at Index 0
     */
    
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

    /**
     * Enable Component
     * 
     */
    enable = 1;
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

