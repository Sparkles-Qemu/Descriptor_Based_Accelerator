#ifndef GLOBALS_CPP
#define GLOBALS_CPP

namespace GLOBALS
{
    const int SRAM_ADDR_PRECISION = 32;
    const int SRAM_DATA_PRECISION = 32;
    const int SRAM_DEFAULT_SIZE = 1024*SRAM_PRECISION/8;
    const int SRAM_RESPONSE_DELAY = 1;
    const unsigned int PE_OPERAND_PRECISION = 32;
    const unsigned int PE_ACCUMULATION_PRECISION = 32;
    const unsigned int DESCRIPTOR_INSTRUCTION_BUFFER_SIZE = 3;
}

#endif