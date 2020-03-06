
#ifndef PAR_DESCRIPTOR_CPP
#define PAR_DESCRIPTOR_CPP

#include "systemc.h"
#include "map"
#include "vector"
#include "COMPONENT.cpp"

struct PAR_DESCRIPTOR : StatelessComponent
{
    //------------Define Globals Here---------------------
    static const unsigned int PE_OPERAND_PRECISION = 32;
    static const unsigned int PE_ACCUMULATION_PRECISION = 32;

    //------------Define Ports Here---------------------
    sc_in<sc_int<PE_ACCUMULATION_PRECISION> > psumIn;
    sc_in<sc_int<PE_OPERAND_PRECISION> > pixelIn0;
    sc_out<sc_int<PE_ACCUMULATION_PRECISION> > psumOut;

    //------------Local Variables Here---------------------
    sc_uint<PE_OPERAND_PRECISION>  current_weight;

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

    PAR_DESCRIPTOR(
        ::sc_core::sc_module_name name, 
        const sc_signal<bool>& _clk, 
        const sc_signal<bool>& _reset, 
        const sc_signal<bool>& _enable,
        const sc_signal<sc_int<PE_ACCUMULATION_PRECISION> >& _psumIn,
        const sc_signal<sc_int<PE_OPERAND_PRECISION> >& _pixelIn0,
        sc_signal<sc_int<PE_ACCUMULATION_PRECISION> >& _psumOut
      ) : StatelessComponent(name, _clk, _reset, _enable)
    {
      this->psumIn(_psumIn);
      this->pixelIn0(_pixelIn0);
      this->psumOut(_psumOut);
    }

}; // End of Module PE
#endif