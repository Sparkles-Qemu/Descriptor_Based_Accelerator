
#ifndef SRAM_CPP
#define SRAM_CPP

#include "systemc.h"
#include "map"
#include "vector"
#include "COMPONENT.cpp"
#include "GLOBALS.cpp"

using std::vector;

struct SRAM
{
    static const int SRAM_PRECISION = GLOBALS::SRAM_PRECISION;
    static const int SRAM_DEFAULT_SIZE = GLOBALS::SRAM_DEFAULT_SIZE;

    vector<sc_int<SRAM_PRECISION> > memory;

    SRAM()
    {
        memory.resize(SRAM_DEFAULT_SIZE);
    }

    SRAM(unsigned int _size)
    {
        memory.resize(_size);
    }

    sc_int<SRAM_PRECISION> get(sc_int<SRAM_PRECISION> addr)
    {
        return memory.at(addr);
    }

    sc_int<SRAM_PRECISION> set(sc_int<SRAM_PRECISION> addr, sc_int<SRAM_PRECISION> data)
    {
        memory.at(addr) = data;
        return memory.at(addr);
    }

}; // End of Module SRAM
#endif