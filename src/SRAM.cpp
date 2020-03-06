
#ifndef SRAM_CPP
#define SRAM_CPP

#include "systemc.h"
#include "map"
#include "vector"
#include "COMPONENT.cpp"
#include "GLOBALS.cpp"

using std::vector;

template <typename DataType>
struct SRAM
{
    static const int SRAM_ADDR_PRECISION = GLOBALS::SRAM_ADDR_PRECISION;
    static const int SRAM_DEFAULT_SIZE = GLOBALS::SRAM_DEFAULT_SIZE;

    vector<DataType> memory;

    SRAM()
    {
        memory.resize(SRAM_DEFAULT_SIZE);
    }

    SRAM(unsigned int _size)
    {
        memory.resize(_size);
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
#endif