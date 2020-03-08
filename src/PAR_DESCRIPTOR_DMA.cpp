
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
#include <cmath>

using std::vector;
using std::cout;
using std::endl;

template <int SramAddrPrecision, typename DataType>
struct PAR_DESCRIPTOR_DMA : StatelessComponent
{
    //------------Local Variables Here---------------------
    enum class FetchState {INIT, IDLE, LOAD_INST_FROM_SRAM, WAIT, STORE_INST_TO_BUFFER} fetchState;
    enum class ExecuteState {IDLE, ISSUE_2D_OP, ISSUE_1D_OP, GET_NEXT_INST, WAIT} executeState;
    unsigned int fetchWaitCounter;
    unsigned int executeWaitCounter;
    unsigned int fetchIndex;
    unsigned int prevFetchIndex;
    unsigned int executeIndex;
    unsigned int sramResponseDelay = GLOBALS::SRAM_RESPONSE_DELAY;
    unsigned int instructionStartOffset;
    vector<DescriptorInstruction> instructionBuffer;
    InstructionSRAM *instructionSram;
    SRAM<SramAddrPrecision, DataType > *dataSram;
    sc_int<SramAddrPrecision> nextDescriptorAddr;

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

    void printSpecial()
    {
      int index = 0;
      for(auto descriptor : instructionBuffer)
      {
        cout << "Descriptor[" << index << "]" << ": SRAM_ADDR(" << descriptor.startAddr << ") : nextDescriptor(" << descriptor.nextDescPtr << ")" << endl;
        index++;
      }
    }
    
    virtual void memoryOp()
    {

    }

    void fetch()
    {


      switch(fetchState)
      {
        /**
         * Magic load of internal buffer for the first time
         * 
         */
        case FetchState::INIT:
        {
          try
          {
            unsigned int offset = instructionStartOffset;
            for (auto it = instructionBuffer.begin(); it != instructionBuffer.end(); ++it)
            {
              *it = instructionSram->get(offset);
              offset = it->nextDescPtr;
            }
          }
          catch(const std::exception& e)
          {
            std::cerr << e.what() << '\n';
            assert(0);
            exit(EXIT_FAILURE);
          }
          fetchState = FetchState::IDLE;
          break;
        }
        case FetchState::IDLE:
        {
          if(abs(executeIndex-fetchIndex) > 0)
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
            DescriptorInstruction currentIns = instructionBuffer.at(prevFetchIndex);
            nextDescriptorAddr = currentIns.nextDescPtr;
            fetchWaitCounter = sramResponseDelay;
            fetchState = FetchState::WAIT;
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
            prevFetchIndex = fetchIndex;
            fetchIndex = (fetchIndex+1) % instructionBuffer.size();            
            if(abs(executeIndex-fetchIndex) > 0)
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
      fetchState = FetchState::INIT;
      executeState = ExecuteState::IDLE;
      fetchIndex = 0;
      prevFetchIndex = (instructionBuffer.size()-1);
      executeIndex = 0;
      clearInstructionBuffer();
    }

    void setIndexOfFirstInstruction()
    {
      try
      {
        //check valid offset
        assert(instructionSram->sramSize > instructionStartOffset);
        //set first index where first instruction exists
        instructionBuffer.begin()->nextDescPtr = instructionStartOffset;
      }
      catch(const std::exception& e)
      {
        std::cerr << e.what() << '\n';
      }
    }

    void clearInstructionBuffer()
    {
      for(auto descriptor : instructionBuffer)
      {
        descriptor.clear();
      }
    }

    PAR_DESCRIPTOR_DMA(
        ::sc_core::sc_module_name name, 
        const sc_signal<bool>& _clk, 
        const sc_signal<bool>& _reset, 
        const sc_signal<bool>& _enable,
        InstructionSRAM *_instructionSram,
        SRAM<SramAddrPrecision, DataType > *_dataSram,
        const unsigned int _instructionStartOffset
      ) : PAR_DESCRIPTOR_DMA(
        name, 
        _clk, 
        _reset, 
        _enable, 
        _instructionSram, 
        _dataSram, 
        _instructionStartOffset, 
        GLOBALS::DESCRIPTOR_INSTRUCTION_BUFFER_SIZE)
    {
    }

    PAR_DESCRIPTOR_DMA(
        ::sc_core::sc_module_name name, 
        const sc_signal<bool>& _clk, 
        const sc_signal<bool>& _reset, 
        const sc_signal<bool>& _enable,
        InstructionSRAM *_instructionSram,
        SRAM<SramAddrPrecision, DataType > *_dataSram,
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

}; 

typedef PAR_DESCRIPTOR_DMA<GLOBALS::SRAM_ADDR_PRECISION, sc_int<GLOBALS::SRAM_DATA_PRECISION> > DefaultParallelDMA;

// End of Module Parallel DMA
#endif