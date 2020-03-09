#ifndef DESCRIPTOR_INSTRUCTION_CPP
#define DESCRIPTOR_INSTRUCTION_CPP

#include "GLOBALS.cpp"
#include "systemc.h"
#include <iostream>

using std::cout;
using std::endl;

struct DescriptorInstruction
{

    static const unsigned int CONFIG_FLAG_ENABLED           = 0b000001;
    static const unsigned int CONFIG_FLAG_STOP              = 0b000010;
    static const unsigned int CONFIG_FLAG_TIMED_WAIT        = 0b000100;
    static const unsigned int CONFIG_FLAG_ISSUE_1D          = 0b001000;
    static const unsigned int CONFIG_FLAG_ISSUE_2D          = 0b010000;
    static const unsigned int CONFIG_FLAG_WAIT              = 0b100000;

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
        cout << "\tnextDescPtr: " << nextDescPtr << endl;
        cout << "\tstartAddr: " << startAddr << endl;
        cout << "\tdmaConfig: " << dmaConfig << endl;
        cout << "\txCount: " << xCount << endl;
        cout << "\txModify: " << xModify << endl;
        cout << "\tyCount: " << yCount << endl;
        cout << "\tyModify: " << yModify << endl; 
    }

    bool operator==(const DescriptorInstruction& rhs)
    {
        bool result = true;
        result &= (this->nextDescPtr == rhs.nextDescPtr);
        result &= (this->startAddr == rhs.startAddr);
        result &= (this->dmaConfig == rhs.dmaConfig);
        result &= (this->xCount == rhs.xCount);
        result &= (this->xModify == rhs.xModify);
        result &= (this->yCount == rhs.yCount);
        result &= (this->yModify == rhs.yModify);
        return result;
    }

    DescriptorInstruction& operator=(const DescriptorInstruction& rhs)
    {
        if(this != &rhs)
        {
            this->nextDescPtr = rhs.nextDescPtr;
            this->startAddr = rhs.startAddr;
            this->dmaConfig = rhs.dmaConfig;
            this->xCount = rhs.xCount;
            this->xModify = rhs.xModify;
            this->yCount = rhs.yCount;
            this->yModify = rhs.yModify;
        }
        return *this;
    }

    void clear()
    {
        nextDescPtr = 0;
        startAddr = 0;
        dmaConfig = 0;
        xCount = 0;
        xModify = 0;
        yCount = 0;
        yModify = 0;
    }
};

#endif