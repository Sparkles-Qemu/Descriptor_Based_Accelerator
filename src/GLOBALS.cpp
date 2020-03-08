#ifndef GLOBALS_CPP
#define GLOBALS_CPP

#define DEBUGGER_HOOKUP

namespace GLOBALS
{
    const int SRAM_ADDR_PRECISION = 32;
    const int SRAM_DEFAULT_SIZE = 1024;
    const int SRAM_DATA_PRECISION = 32;
    const int SRAM_RESPONSE_DELAY = 1;
    const unsigned int PE_OPERAND_PRECISION = 32;
    const unsigned int PE_ACCUMULATION_PRECISION = 32;
    // @todo monitor changes at higher buffer sizes
    const unsigned int DESCRIPTOR_INSTRUCTION_BUFFER_SIZE = 2;
}

#endif