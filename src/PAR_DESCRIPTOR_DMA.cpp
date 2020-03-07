
#ifndef PAR_DESCRIPTOR_DMA_CPP
#define PAR_DESCRIPTOR_DMA_CPP

#include <assert.h>
#include "systemc.h"
#include "map"
#include "vector"
#include "COMPONENT.cpp"
#include "GLOBALS.cpp"
#include "SRAM.cpp"
#include "DESCRIPTOR_INSTRUCTION.cpp"

using std::vector;
using std::cout;
using std::endl;

template <int SramAddrPrecision, typename DataType>
struct PAR_DESCRIPTOR_DMA : StatelessComponent
{
    //------------Define Globals Here---------------------
    unsigned int DESCRIPTOR_INSTRUCTION_BUFFER_SIZE = GLOBALS::DESCRIPTOR_INSTRUCTION_BUFFER_SIZE;

    //------------Local Variables Here---------------------
    enum class FetchState {IDLE, LOAD_INST_FROM_SRAM, WAIT, STORE_INST_TO_BUFFER} fetchState;
    enum class ExecuteState {IDLE, ISSUE_2D_OP, ISSUE_1D_OP, GET_NEXT_INST, WAIT} executeState;
    unsigned int fetchWaitCounter;
    unsigned int executeWaitCounter;
    unsigned int fetchIndex;
    unsigned int executeIndex;
    unsigned int sramResponseDelay = GLOBALS::SRAM_RESPONSE_DELAY;
    vector<DescriptorInstruction> instructionBuffer;
    InstructionSRAM *instructionSram;
    SRAM<SramAddrPrecision, DataType > *dataSram;

    //------------Define Functions Here---------------------
    void printInstructionBuffer()
    {
      int index = 0;
      for(auto descriptor : instructionBuffer)
      {
        cout << "Descriptor[" << index << "]" << ": Contents" << endl;
        descriptor.printDescriptor();
        index++;
      }
    }
    
    virtual void memoryOp()
    {

    }

    void fetch()
    {
      static sc_int<SramAddrPrecision> nextDescriptorAddr;

      switch(fetchState)
      {
        case FetchState::IDLE:
        {
          if((fetchIndex+1) % DESCRIPTOR_INSTRUCTION_BUFFER_SIZE != executeIndex)
          {
            fetchState = FetchState::LOAD_INST_FROM_SRAM;
          }
          else
          {
            fetchState = FetchState::IDLE;
          }
          break;
        }
        case FetchState::LOAD_INST_FROM_SRAM:
        {
          try
          {
            DescriptorInstruction currentIns = instructionBuffer.at(fetchIndex);
            nextDescriptorAddr = currentIns.nextDescPtr;
            fetchWaitCounter = sramResponseDelay;
            fetchState = FetchState::WAIT;
            fetchIndex = (fetchIndex+1) % DESCRIPTOR_INSTRUCTION_BUFFER_SIZE;
          }
          catch(const std::exception& e)
          {
            std::cerr << e.what() << '\n';
            #ifdef DEBUGGER_HOOKUP
            assert(0);
            #endif
          }
          break;
        }
        case FetchState::WAIT:
        {
          fetchWaitCounter--;
          if(fetchWaitCounter == 0)
          {
            fetchState = FetchState::STORE_INST_TO_BUFFER;
          }
          else
          {
            fetchState = FetchState::WAIT;
          }
          
          break;
        }
        case FetchState::STORE_INST_TO_BUFFER:
        {
          try
          {
            DescriptorInstruction nextIns = instructionSram->get(nextDescriptorAddr);
            instructionBuffer.at(fetchIndex) = nextIns;
            if((fetchIndex+1) % DESCRIPTOR_INSTRUCTION_BUFFER_SIZE != executeIndex)
            {
              fetchState = FetchState::LOAD_INST_FROM_SRAM;
            }
            else
            {
              fetchState = FetchState::IDLE;
            }
          }
          catch(const std::exception& e)
          {
            std::cerr << e.what() << '\n';
            #ifdef DEBUGGER_HOOKUP
            assert(0);
            #endif
          }
          break;
        }
      }
    }

    void execute()
    {
      switch(executeState)
      {
        case ExecuteState::IDLE:
        break;
        case ExecuteState::ISSUE_2D_OP:
        break;
        case ExecuteState::ISSUE_1D_OP:
        break;
        case ExecuteState::GET_NEXT_INST:
        break;
        case ExecuteState::WAIT:
        break;
      }
    }

    void computeFn()
    {
      fetch();
      execute();
    }

    void resetFn()
    {
      fetchState = FetchState::IDLE;
      executeState = ExecuteState::IDLE;
      fetchIndex = 0;
      executeIndex = 0;
      instructionBuffer.clear();
    }

    PAR_DESCRIPTOR_DMA(
        ::sc_core::sc_module_name name, 
        const sc_signal<bool>& _clk, 
        const sc_signal<bool>& _reset, 
        const sc_signal<bool>& _enable,
        InstructionSRAM *_instructionSram,
        SRAM<SramAddrPrecision, DataType > *_dataSram
      ) : StatelessComponent(name, _clk, _reset, _enable)
    {
      instructionSram = instructionSram;
      dataSram = _dataSram;
      instructionBuffer.resize(DESCRIPTOR_INSTRUCTION_BUFFER_SIZE);
    }

}; 

typedef PAR_DESCRIPTOR_DMA<GLOBALS::SRAM_ADDR_PRECISION, sc_int<GLOBALS::SRAM_DATA_PRECISION> > DefaultParallelDMA;

// End of Module Parallel DMA
#endif