
#ifndef PMM2S_DESCRIPTOR_CPP
#define PMM2S_DESCRIPTOR_CPP

#include "systemc.h"
#include "PAR_DESCRIPTOR_DMA.cpp"

template <int SramAddrPrecision, typename InputDataType, typename OutputDataType>
struct PMM2S_DESCRIPTOR : PAR_DESCRIPTOR_DMA<SramAddrPrecision, InputDataType>
{
    //------------Define Globals Here---------------------

    //------------Define Ports Here---------------------
    sc_out<OutputDataType> outputStream; 
    //------------Local Variables Here---------------------

    //------------Define Functions Here---------------------
    // void moveData(ExecuteState& currentState, DescriptorInstruction& currentInstruction, unsigned int& xCount, unsigned int& yCount)
    // {
        
    // }


    PMM2S_DESCRIPTOR(
        ::sc_core::sc_module_name name, 
        const sc_signal<bool>& _clk, 
        const sc_signal<bool>& _reset, 
        const sc_signal<bool>& _enable,
        InstructionSRAM *_instructionSram,
        SRAM<SramAddrPrecision, InputDataType> *_dataSram,
        sc_signal<OutputDataType>& _outputStream,
        const unsigned int _instructionStartOffset,
        const unsigned int _instructionBufferSize
      ) : PAR_DESCRIPTOR_DMA<SramAddrPrecision, InputDataType>(
        name, 
        _clk, 
        _reset, 
        _enable, 
        _instructionSram, 
        _dataSram, 
        _instructionStartOffset, 
        _instructionBufferSize)
    {
        this->outputStream(_outputStream);
    }

}; // End of Module PE

typedef PMM2S_DESCRIPTOR<GLOBALS::SRAM_ADDR_PRECISION, sc_int<GLOBALS::SRAM_DATA_PRECISION>, sc_int<GLOBALS::SRAM_DATA_PRECISION> > DefaultPMM2S;

#endif