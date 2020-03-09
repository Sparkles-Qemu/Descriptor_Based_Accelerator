#include "systemc.h"
#include "SRAM.cpp"
#include "PAR_DESCRIPTOR_DMA.cpp"
#include "GLOBALS.cpp"
#include "DESCRIPTOR_INSTRUCTION.cpp"
#include <assert.h>

using std::cout;
using std::endl;
using std::vector;

#define MAX_RESET_CYCLES 10
#define MAX_SIM_CYCLES 2
#define MAX_INSTRUCTION_SEPERATION 1
#define INSTRUCTION_PAYLOAD_SIZE 5

void pulse(sc_signal<bool>& clk)
{
    clk = 1;
    sc_start(1, SC_NS);
    clk = 0;
    sc_start(1, SC_NS);
}

int sc_main(int argc, char *argv[]) 
{

    /**
     * Signal Decleration
     */
    sc_signal<bool > clk("clk"); 
    sc_signal<bool > reset("reset"); 
    sc_signal<bool > enable("enable"); 

    /**
     * Default SRAM declarations. Use defaults defined in Globals
     * 
     */
    InstructionSRAM instSRAM;
    DefaultDataSRAM dataSRAM;

    /**
     * Component Decleration
     * Default Parallel Descriptor DMA,
     * Relies of defaults defined in GLOBALS. If parallel DMA attached to SRAM 
     * of different address precision or data type, compiler will fail. 
     */
    const unsigned int firstInstructionOffset = 0;
    DefaultParallelDMA parallelDma("Par_Descriptor",clk, reset, enable, &instSRAM, &dataSRAM, firstInstructionOffset);

    /**
     * Loading Descriptors into SRAM starting at Index 0
     */
    vector<DescriptorInstruction > instructionPayload;
    srand(123);
    for (unsigned int i = 0, index = 1, lastIndex = 0; i < INSTRUCTION_PAYLOAD_SIZE+1; i++, lastIndex = index, index += 1 + rand() % MAX_INSTRUCTION_SEPERATION)
    {
        if(i == INSTRUCTION_PAYLOAD_SIZE-1 || i == INSTRUCTION_PAYLOAD_SIZE)
        {
            instructionPayload.push_back(DescriptorInstruction(
                index,
                lastIndex,
                DescriptorInstruction::CONFIG_FLAG_ENABLED | DescriptorInstruction::CONFIG_FLAG_STOP, 
                0, //xCount
                0, //xModify
                0, //yCount
                0  //yModify  
                ));
        }
        else
        {
            instructionPayload.push_back(DescriptorInstruction(
                index,
                lastIndex,
                DescriptorInstruction::CONFIG_FLAG_ENABLED | DescriptorInstruction::CONFIG_FLAG_TIMED_WAIT, 
                2, //xCount
                -1, //xModify
                0, //yCount
                0  //yModify  
                ));            
        }
    }
    instSRAM.programLoad(instructionPayload,firstInstructionOffset);

    /**
     * Checking Contents of SRAM
     */
    cout << "@ " << sc_time_stamp() << " Instruction SRAM Program Load Test" << endl;
    for (unsigned int i = 0, index = 0; i < instructionPayload.size(); index = instructionPayload.at(i).nextDescPtr, i++)
    {
        assert(instSRAM.get(index) == instructionPayload.at(i));
    }
    cout << "@ " << sc_time_stamp() << " Instruction SRAM Program Load Test ... PASS!" << endl;
    
    /**
     * Fetch and Execute with timed wait test
     */
    reset = 0;
    sc_start(1, SC_NS);

    for (unsigned int simLoop = 0; simLoop < MAX_SIM_CYCLES; simLoop++)
    {
        cout << "@ " << sc_time_stamp() << " Starting Simulation Cycle " << simLoop << endl;

        /**
         * Resetting Component
         */
        reset = 1;
        cout << "@ " << sc_time_stamp() << " Asserting reset" << endl;
        for (int i = 0; i < MAX_RESET_CYCLES; i++)
        {
            sc_start(1, SC_NS); 
        }
        cout << "@ " << sc_time_stamp() << " Deasserting reset" << endl;
        reset = 0; 

        /**
         * Start Magic load
         */
        // Enable Component
        enable = 1;
        sc_start(1, SC_NS); 
        pulse(clk);

        /**
         * Validate Magic Load
         */
        cout << "@ " << sc_time_stamp() << " Magic Load Test" << endl;
        for (unsigned int i = 0, index = 0; i < parallelDma.instructionBuffer.size(); index = instructionPayload.at(i).nextDescPtr, i++)
        {       
            assert(instSRAM.get(index) == parallelDma.instructionBuffer.at(i));
            assert(parallelDma.instructionBuffer.at(i) == instructionPayload.at(i));
        }
        assert(parallelDma.fetchState == DefaultParallelDMA::FetchState::IDLE);
        cout << "@ " << sc_time_stamp() << " Magic Load Pass!" << endl;
        
        /**
        * Start New Instruction Load Test
        */
        cout << "@ " << sc_time_stamp() << " Instruction Load Test" << endl;

        // load all instructions other than the first few that were magically loaded
        // internal buffer. 
        unsigned int lastExecute;
        unsigned int fetchedInstructionIndex = parallelDma.instructionBuffer.size();
        for (unsigned int fetchCycle = 0; fetchCycle < instructionPayload.size()-parallelDma.instructionBuffer.size(); fetchCycle++)
        {
            
            cout << "@ " << sc_time_stamp() << " Buffer Contents" << endl;
            parallelDma.printSpecial();

            if(fetchCycle == 0)
            {
                // Increment executeIndex to trigger fetch operations
                lastExecute = parallelDma.executeIndex;
                assert(parallelDma.fetchState == DefaultParallelDMA::FetchState::IDLE);
                // Allow Fetch State machine to realize that the execute index
                // has changed and that it needs to fetch a new instruction
                pulse(clk);
                pulse(clk);
                pulse(clk);
            }

            assert(parallelDma.fetchState == DefaultParallelDMA::FetchState::LOAD_INST_FROM_SRAM);

            pulse(clk);

            assert(parallelDma.fetchState == DefaultParallelDMA::FetchState::WAIT);

            pulse(clk);

            assert(parallelDma.fetchState == DefaultParallelDMA::FetchState::STORE_INST_TO_BUFFER);

            pulse(clk);

            assert(parallelDma.instructionBuffer.at(lastExecute) == instructionPayload.at(fetchedInstructionIndex));
            fetchedInstructionIndex++;

            lastExecute = (lastExecute + 1) % parallelDma.instructionBuffer.size();

        }

        /**
         * Allow fetch state machine to save last Descriptor 
         * 
         */
        pulse(clk);
        pulse(clk);
        pulse(clk);
        
        cout << "@ " << sc_time_stamp() << " Buffer Contents" << endl;
        parallelDma.printSpecial();

        assert(parallelDma.fetchState == DefaultParallelDMA::FetchState::IDLE);

        assert(parallelDma.executeIndex == parallelDma.fetchIndex);
        
        cout << "@ " << sc_time_stamp() << " Instruction Load Test Pass!" << endl;
        
    }
    

    /**
     * Execute Test 1D and 2D
     */
    instructionPayload.clear();
    instructionPayload.push_back(DescriptorInstruction(
        1, // next descriptor
        0, // start address
        DescriptorInstruction::CONFIG_FLAG_ENABLED | DescriptorInstruction::CONFIG_FLAG_ISSUE_1D, 
        5, // xCount
        1, // xModify
        0, // yCount
        0  // yModify  
        ));            
    instructionPayload.push_back(DescriptorInstruction(
        2, // next descriptor
        1, // start address
        DescriptorInstruction::CONFIG_FLAG_ENABLED | DescriptorInstruction::CONFIG_FLAG_ISSUE_2D, 
        2, //xCount
        1, //xModify
        2, //yCount
        1  //yModify  
        )); 
    instructionPayload.push_back(DescriptorInstruction(
        3, // next descriptor
        2, // start address
        DescriptorInstruction::CONFIG_FLAG_ENABLED | DescriptorInstruction::CONFIG_FLAG_ISSUE_2D, 
        2, //xCount
        1, //xModify
        2, //yCount
        1  //yModify  
        ));     
    instructionPayload.push_back(DescriptorInstruction(
        4, // next descriptor
        3, // start address
        DescriptorInstruction::CONFIG_FLAG_ENABLED | DescriptorInstruction::CONFIG_FLAG_ISSUE_1D, 
        2, //xCount
        1, //xModify
        0, //yCount
        0  //yModify  
        )); 
    instructionPayload.push_back(DescriptorInstruction(
        4, // next descriptor
        4, // start address
        DescriptorInstruction::CONFIG_FLAG_ENABLED | DescriptorInstruction::CONFIG_FLAG_STOP, 
        0, //xCount
        0, //xModify
        0, //yCount
        0  //yModify  
        ));   
    
    instSRAM.programLoad(instructionPayload,firstInstructionOffset);
    reset = 0;
    sc_start(1, SC_NS);

    for (unsigned int simLoop = 0; simLoop < MAX_SIM_CYCLES; simLoop++)
    {
        cout << "@ " << sc_time_stamp() << " Starting Simulation Cycle " << simLoop << endl;

        /**
         * Resetting Component
         */
        reset = 1;
        cout << "@ " << sc_time_stamp() << " Asserting reset" << endl;
        for (int i = 0; i < MAX_RESET_CYCLES; i++)
        {
            sc_start(1, SC_NS); 
        }
        cout << "@ " << sc_time_stamp() << " Deasserting reset" << endl;
        reset = 0; 

        /**
         * Start Magic load
         */
        // Enable Component
        enable = 1;
        sc_start(1, SC_NS); 
        pulse(clk);

        /**
         * Validate Magic Load
         */
        cout << "@ " << sc_time_stamp() << " Magic Load Test" << endl;
        for (unsigned int i = 0, index = 0; i < parallelDma.instructionBuffer.size(); index = instructionPayload.at(i).nextDescPtr, i++)
        {       
            assert(instSRAM.get(index) == parallelDma.instructionBuffer.at(i));
            assert(parallelDma.instructionBuffer.at(i) == instructionPayload.at(i));
        }
        assert(parallelDma.fetchState == DefaultParallelDMA::FetchState::IDLE);
        cout << "@ " << sc_time_stamp() << " Magic Load Pass!" << endl;

        
        /**
        * Start DataMoveTest
        */
        cout << "@ " << sc_time_stamp() << " Start Data Move Test " << endl;
        while(parallelDma.executeState != DefaultParallelDMA::ExecuteState::STOP)
        {
            pulse(clk);
        }

        assert(15 == parallelDma.moveDataStats);
        cout << "@ " << sc_time_stamp() << " Data Move Test Pass! ... " << endl;

    }

    /**
    * Test Final Async Reset
    */
    cout << "@ " << sc_time_stamp() << " Compute complete, testing async reset" << endl;
    sc_start(1.5, SC_NS); 
    reset = 1;
    sc_start(1.5, SC_NS);
    
    cout << "@ " << sc_time_stamp() << " Sim complete, Simulation terminating .... " << endl;


    return 0; // Terminate simulation
}

