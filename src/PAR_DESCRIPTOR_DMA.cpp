
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

enum class FetchState {INIT, IDLE, LOAD_INST_FROM_SRAM, WAIT, STORE_INST_TO_BUFFER};
enum class ExecuteState {DECODE, STOP, TIMED_WAIT, ISSUE_2D_OP, ISSUE_1D_OP};

template <int SramAddrPrecision, typename DataType>
struct PAR_DESCRIPTOR_DMA : StatelessComponent
{
    //------------Local Variables Here---------------------
    FetchState fetchState;
    ExecuteState executeState;
    unsigned int fetchWaitCounter;
    unsigned int executeWaitCounter;
    unsigned int fetchIndex;
    unsigned int prevFetchIndex;
    unsigned int executeIndex;
    unsigned int sramResponseDelay = GLOBALS::SRAM_RESPONSE_DELAY;
    unsigned int instructionStartOffset;
    unsigned int moveDataStats = 0;
    unsigned int xCount;
    unsigned int yCount;
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
    
    virtual void moveData(ExecuteState& currentState, DescriptorInstruction& currentInstruction, unsigned int& xCount, unsigned int& yCount)
    {
      moveDataStats++;
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
    
    void execute()
    {
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
              executeState = ExecuteState::ISSUE_1D_OP;
              xCount = 0;
              break;

            case DescriptorInstruction::CONFIG_FLAG_ENABLED | DescriptorInstruction::CONFIG_FLAG_ISSUE_2D:
              executeState = ExecuteState::ISSUE_2D_OP;
              xCount = 0;
              yCount = 0;
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
          currentIns.xCount += currentIns.xModify;
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
          moveData(executeState, currentIns, xCount, yCount);
          xCount += currentIns.xModify;
          if(currentIns.xCount-1 == xCount)
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
          moveData(executeState, currentIns, xCount, yCount);
          xCount += currentIns.xModify;
          if(currentIns.xCount == xCount)
          {
            yCount += currentIns.yModify;
            xCount = 0;
          }
          if(currentIns.yCount == yCount)
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
      }
    }

    void computeFn()
    {
      execute();
      fetch();
    }

    //@todo make virtual and test tb_generic_descriptor with virtual to make
    // that it calls the default first. Then override in base to add
    // outputstream clean.

    void resetFn() 
    {
      fetchState = FetchState::INIT;
      executeState = ExecuteState::DECODE;
      fetchIndex = 0;
      prevFetchIndex = (instructionBuffer.size()-1);
      executeIndex = 0;
      moveDataStats = 0;
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

    /**
     * @brief Construct a new par descriptor dma object
     * 
     * @param name 
     * @param _clk 
     * @param _reset 
     * @param _enable 
     * @param _instructionSram Default for all parallel descriptor dmas
     * @param _dataSram Rely on type checking from compiler by defining the right
     * data type in paralleldma template. Useful for making sure you're passing
     * the right pointer. 
     * @param _instructionStartOffset Where to start looking in instruction sram
     * for the first instruction. 
     * @param _instructionBufferSize Size of internal descriptor dma buffer. If
     * specified will default to default value specified in GLOBAL.cpp 
     */
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