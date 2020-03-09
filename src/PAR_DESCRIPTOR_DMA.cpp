
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
    enum class ExecuteState {DECODE, STOP, TIMED_WAIT, ISSUE_2D_OP, ISSUE_1D_OP, SRAM_WAIT} executeState;
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

    void incrementBufferIndex(unsigned int& index)
    {
      index = (index + 1) % instructionBuffer.size();
    }
    
    virtual void moveData(ExecuteState& currentState, DescriptorInstruction& currentInstruction)
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
            incrementBufferIndex(fetchIndex);
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

    void modifyXCounter(DescriptorInstruction& instruction)
    {
      instruction.xCount += instruction.xModify;
    }

    void modifyYCounter(DescriptorInstruction& instruction)
    {
      instruction.yCount += instruction.yModify;
    }

    void modifyXYCounter(DescriptorInstruction& instruction)
    {
      instruction.xCount += instruction.xModify;
      instruction.yCount += instruction.yModify;
    }

    void execute()
    {
      static ExecuteState stateAfterWait;
      DescriptorInstruction& currentIns = instructionBuffer.at(executeIndex);

      switch(executeState)
      {
        case ExecuteState::DECODE:
        {
          switch (currentIns.dmaConfig)
          {
            case DescriptorInstruction::CONFIG_FLAG_ENABLED | DescriptorInstruction::CONFIG_FLAG_WAIT:
              executeState = ExecuteState::DECODE;
              break;

            case DescriptorInstruction::CONFIG_FLAG_ENABLED | DescriptorInstruction::CONFIG_FLAG_STOP:
              executeState = ExecuteState::STOP;
              break;
            
            case DescriptorInstruction::CONFIG_FLAG_ENABLED | DescriptorInstruction::CONFIG_FLAG_TIMED_WAIT:
              executeState = ExecuteState::TIMED_WAIT;
              break;
            
            case DescriptorInstruction::CONFIG_FLAG_ENABLED | DescriptorInstruction::CONFIG_FLAG_ISSUE_1D:
              stateAfterWait = ExecuteState::ISSUE_1D_OP;
              executeState = ExecuteState::SRAM_WAIT;
              executeWaitCounter = sramResponseDelay;
              break;

            case DescriptorInstruction::CONFIG_FLAG_ENABLED | DescriptorInstruction::CONFIG_FLAG_ISSUE_2D:
              stateAfterWait = ExecuteState::ISSUE_2D_OP;
              executeState = ExecuteState::SRAM_WAIT;
              executeWaitCounter = sramResponseDelay;
              break;
          }
          break;
        }
        case ExecuteState::STOP:
        {
          executeState = ExecuteState::STOP;
          break;
        }
        case ExecuteState::TIMED_WAIT:
        {
          modifyXCounter(currentIns);
          if(currentIns.xCount == 0)
          {
            incrementBufferIndex(executeIndex);
            executeState = ExecuteState::DECODE;
          }
          else
          {
            executeState = ExecuteState::TIMED_WAIT;
          }
          break;
        }
        case ExecuteState::ISSUE_1D_OP:
        {
          moveData(executeState, currentIns);
          modifyXCounter(currentIns);          
          if(currentIns.xCount == 0)
          {
            incrementBufferIndex(executeIndex);
            executeState = ExecuteState::DECODE;
          }
          else
          {
            executeState = ExecuteState::ISSUE_1D_OP;
          }
          break;
        }
        case ExecuteState::ISSUE_2D_OP:
        {
          moveData(executeState, currentIns);
          modifyXYCounter(currentIns);          
          if(currentIns.xCount == 0 && currentIns.yCount == 0)
          {
            incrementBufferIndex(executeIndex);
            executeState = ExecuteState::DECODE;
          }
          else
          {
            executeState = ExecuteState::ISSUE_2D_OP;
          }
          break;
        }

        case ExecuteState::SRAM_WAIT:
        {
          executeWaitCounter--;
          if(executeWaitCounter == 0)
          {
            executeState = stateAfterWait;
          }
          else
          {
            executeState = ExecuteState::SRAM_WAIT;
          }
          break;
        }        
      }
    }

    void computeFn()
    {
      execute();
      fetch();
    }

    void resetFn()
    {
      fetchState = FetchState::INIT;
      executeState = ExecuteState::DECODE;
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
      for(auto descriptor = instructionBuffer.begin(); descriptor != instructionBuffer.end(); descriptor++)
      {
        descriptor->clear();
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