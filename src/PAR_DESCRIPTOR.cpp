
#ifndef PAR_DESCRIPTOR_CPP
#define PAR_DESCRIPTOR_CPP

#include <assert.h>
#include "systemc.h"
#include "map"
#include "vector"
#include "COMPONENT.cpp"
#include "GLOBALS.cpp"
#include "SRAM.cpp"
#include "DESCRIPTOR_INSTRUCTION.cpp"

using std::vector;

struct PAR_DESCRIPTOR : StatelessComponent
{
    //------------Define Globals Here---------------------
    static const unsigned int PAR_DESCRIPTOR_ADDR_PRECISION = GLOBALS::SRAM_ADDR_PRECISION;
    static const unsigned int PAR_DESCRIPTOR_DATA_PRECISION = GLOBALS::SRAM_DATA_PRECISION;
    static const unsigned int SRAM_RESPONSE_DELAY = GLOBALS::SRAM_RESPONSE_DELAY;
    static const unsigned int DESCRIPTOR_INSTRUCTION_BUFFER_SIZE = GLOBALS::DESCRIPTOR_INSTRUCTION_BUFFER_SIZE;

    //------------Local Variables Here---------------------
    enum FetchState {IDLE, LOAD_INST_FROM_SRAM, WAIT, STORE_INST_TO_BUFFER} fetchState;
    enum ExecuteState {IDLE, ISSUE_2D_OP, ISSUE_1D_OP, GET_NEXT_INST, WAIT} executeState;
    unsigned int fetchWaitCounter;
    unsigned int executeWaitCounter;
    unsigned int fetchIndex;
    unsigned int executeIndex;
    sc_int<PAR_DESCRIPTOR_ADDR_PRECISION> addr;
    vector<DescriptorInstruction> instructionBuffer;
    SRAM<DescriptorInstruction> *instructionSram;
    SRAM<sc_int<PAR_DESCRIPTOR_DATA_PRECISION> > *dataSram;

    //------------Define Functions Here---------------------
    virtual void memoryOp()
    {

    }

    void fetch()
    {
      static sc_int<PAR_DESCRIPTOR_ADDR_PRECISION> nextDescriptorAddr;

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
            fetchWaitCounter = SRAM_RESPONSE_DELAY;
            fetchState = FetchState::WAIT;
            fetchIndex = (fetchIndex+1) % DESCRIPTOR_INSTRUCTION_BUFFER_SIZE;
          }
          catch(const std::exception& e)
          {
            std::cerr << e.what() << '\n';
            assert(0);
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
            assert(0);
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
    }

    PAR_DESCRIPTOR(
        ::sc_core::sc_module_name name, 
        const sc_signal<bool>& _clk, 
        const sc_signal<bool>& _reset, 
        const sc_signal<bool>& _enable,
        SRAM<DescriptorInstruction> *_instructionSram,
        SRAM<sc_int<PAR_DESCRIPTOR_DATA_PRECISION> > *_dataSram
      ) : StatelessComponent(name, _clk, _reset, _enable)
    {
    instructionSram = instructionSram;
    dataSram = _dataSram;
    instructionBuffer.resize(DESCRIPTOR_INSTRUCTION_BUFFER_SIZE);
    }

}; // End of Module PE
#endif