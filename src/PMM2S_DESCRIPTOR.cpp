
#ifndef PMM2S_DESCRIPTOR_CPP
#define PMM2S_DESCRIPTOR_CPP

#include "systemc.h"
#include "map"
#include "vector"
#include "COMPONENT.cpp"
#include "GLOBALS.cpp"
#include "PAR_DESCRIPTOR_DMA.cpp"

template <int SramAddrPrecision, typename InputDataType, typename OutputDataType>
struct PMM2S_DESCRIPTOR : PAR_DESCRIPTOR_DMA
{
    //------------Define Globals Here---------------------

    //------------Define Ports Here---------------------
    sc_out<OutputDataType> outputStream; 

    //------------Local Variables Here---------------------

    //------------Define Functions Here---------------------
    //@todo override virtual function moveData here depending on ports 


    PMM2S_DESCRIPTOR(
        ::sc_core::sc_module_name name, 
        const sc_signal<bool>& _clk, 
        const sc_signal<bool>& _reset, 
        const sc_signal<bool>& _enable,
        //@todo pass references to signals with same type as ports to bind them
    ) : StatelessComponent(name, _clk, _reset, _enable)
    {
        
    }

    PMM2S_DESCRIPTOR(
        ::sc_core::sc_module_name name, 
        const sc_signal<bool>& _clk, 
        const sc_signal<bool>& _reset, 
        const sc_signal<bool>& _enable,
        InstructionSRAM *_instructionSram,
        SRAM<SramAddrPrecision, DataType > *_dataSram,
        sc_signal<OutputDataType>& _outputStream,
        const unsigned int _instructionStartOffset
      ) : PAR_DESCRIPTOR_DMA(
        name, 
        _clk, 
        _reset, 
        _enable, 
        _instructionSram, 
        _dataSram, 
        _instructionStartOffset, 
        _outputStream,
        GLOBALS::DESCRIPTOR_INSTRUCTION_BUFFER_SIZE)
    {
    }

    PMM2S_DESCRIPTOR(
        ::sc_core::sc_module_name name,
        const sc_signal<bool>& _clk,
        const sc_signal<bool>& _reset,
        const sc_signal<bool>& _enable,
        InstructionSRAM *_instructionSram,
        SRAM<SramAddrPrecision, InputDataType > *_dataSram,
        sc_signal<OutputDataType>& _outputStream,
        const unsigned int _instructionStartOffset,
        const unsigned int _instructionBufferSize
      ) : StatelessComponent(name, _clk, _reset, _enable)
    {
      instructionSram = _instructionSram;
      dataSram = _dataSram;
      instructionStartOffset = _instructionStartOffset;
      instructionBuffer.resize(_instructionBufferSize);
      resetFn();
    }

}; // End of Module PE
#endif