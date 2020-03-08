
#ifndef _SRAM_CPP
#define _SRAM_CPP

#include "systemc.h"
#include "map"
#include "vector"
#include "COMPONENT.cpp"
#include "GLOBALS.cpp"
#include "assert.h"
#include "DESCRIPTOR_INSTRUCTION.cpp"

using std::vector;

template <int SRAM_ADDR_PRECISION, typename DataType>
struct SRAM
{
    int sramSize = GLOBALS::SRAM_DEFAULT_SIZE;

    vector<DataType> memory;

    SRAM()
    {
        memory.resize(sramSize);
    }

    SRAM(unsigned int _size)
    {
        memory.resize(_size);
        sramSize = _size;
    }

    void programLoad(vector<DescriptorInstruction> prog, unsigned int offset)
    {
        assert(offset < memory.size() && (offset+prog.size()) < memory.size());
        unsigned int index = offset;
        for(auto val : prog)
        {
            set(index, val);
            index = val.nextDescPtr;
            assert(index < memory.size());
        }
    }

    void load(vector<DataType> data, unsigned int offset)
    {
        assert(offset < memory.size() && (offset+data.size()) < memory.size());
        int index = offset;
        for(auto val : data)
        {
            set(index, val);
            index++;
        }
    }
    
    DataType get(sc_int<SRAM_ADDR_PRECISION> addr)
    {
        return memory.at(addr);
    }
    
    DataType set(sc_int<SRAM_ADDR_PRECISION> addr, DataType data)
    {
        memory.at(addr) = data;
        return memory.at(addr);
    }

}; // End of Module SRAM

typedef SRAM<GLOBALS::SRAM_ADDR_PRECISION, DescriptorInstruction > InstructionSRAM; 
typedef SRAM<GLOBALS::SRAM_ADDR_PRECISION, sc_int<GLOBALS::SRAM_DATA_PRECISION > > DefaultDataSRAM; 

#endif