#ifndef DESCRIPTOR_INSTRUCTION_CPP
#define DESCRIPTOR_INSTRUCTION_CPP

#include "GLOBALS.cpp"
#include "systemc.h"
#include <iostream>

using std::cout;
using std::endl;

struct DescriptorInstruction
{
    sc_int<GLOBALS::SRAM_DATA_PRECISION> nextDescPtr;
    sc_int<GLOBALS::SRAM_DATA_PRECISION> startAddr;
    sc_int<GLOBALS::SRAM_DATA_PRECISION> dmaConfig;
    sc_int<GLOBALS::SRAM_DATA_PRECISION> xCount;
    sc_int<GLOBALS::SRAM_DATA_PRECISION> xModify;
    sc_int<GLOBALS::SRAM_DATA_PRECISION> yCount;
    sc_int<GLOBALS::SRAM_DATA_PRECISION> yModify;

    DescriptorInstruction() = default;
    DescriptorInstruction(const DescriptorInstruction &) = default;

    DescriptorInstruction(
        sc_int<GLOBALS::SRAM_DATA_PRECISION> _nextDescPtr,
        sc_int<GLOBALS::SRAM_DATA_PRECISION> _startAddr,
        sc_int<GLOBALS::SRAM_DATA_PRECISION> _dmaConfig,
        sc_int<GLOBALS::SRAM_DATA_PRECISION> _xCount,
        sc_int<GLOBALS::SRAM_DATA_PRECISION> _xModify,
        sc_int<GLOBALS::SRAM_DATA_PRECISION> _yCount,
        sc_int<GLOBALS::SRAM_DATA_PRECISION> _yModify
    )
    {
        this->nextDescPtr = _nextDescPtr;
        this->startAddr = _startAddr;
        this->dmaConfig = _dmaConfig;
        this->xCount = _xCount;
        this->xModify = _xModify;
        this->yCount = _yCount;
        this->yModify = _yModify;
    }

    void printDescriptor()
    {
        cout << "nextDescPtr: " << nextDescPtr << endl;
        cout << "startAddr: " << startAddr << endl;
        cout << "dmaConfig: " << dmaConfig << endl;
        cout << "xCount: " << xCount << endl;
        cout << "xModify: " << xModify << endl;
        cout << "yCount: " << yCount << endl;
        cout << "yModify: " << yModify << endl; 
    }
};

#endif