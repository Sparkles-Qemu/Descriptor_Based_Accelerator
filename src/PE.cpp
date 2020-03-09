
#ifndef PE_CPP
#define PE_CPP

#include "systemc.h"
#include "map"
#include "vector"
#include "COMPONENT.cpp"
#include "GLOBALS.cpp"

template <typename InputDataType, typename OutputDataType>
struct PE : StatelessComponent
{
    //------------Define Globals Here---------------------

    //------------Define Ports Here---------------------
    sc_in<InputDataType > psumIn;
    sc_in<InputDataType > pixelIn0;
    sc_out<OutputDataType > psumOut;

    //------------Local Variables Here---------------------
    InputDataType  current_weight;

    //------------Define Functions Here---------------------
    void computeFn()
    {
      psumOut.write(psumIn.read()+pixelIn0.read()*current_weight);
    }

    void resetFn()
    {
      cout << "@ " << sc_time_stamp() << " PE has been reset" << endl;
      psumOut.write(0);
    }

    PE(
        ::sc_core::sc_module_name name, 
        const sc_signal<bool>& _clk, 
        const sc_signal<bool>& _reset, 
        const sc_signal<bool>& _enable,
        const sc_signal<InputDataType >& _psumIn,
        const sc_signal<InputDataType >& _pixelIn0,
        sc_signal<OutputDataType >& _psumOut
      ) : StatelessComponent(name, _clk, _reset, _enable)
    {
      this->psumIn(_psumIn);
      this->pixelIn0(_pixelIn0);
      this->psumOut(_psumOut);
    }

}; // End of Module PE
#endif