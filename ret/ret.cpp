#include "pin.H"
#include <fstream>
#include <iostream>

ofstream OutFile;

INT32 Usage()
{
    cerr << "This tool print out the return address on every ret instructions" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

VOID OnRET(ADDRINT ip, ADDRINT* sp)
{
    ADDRINT ret;
    PIN_SafeCopy(&ret, sp, sizeof(ADDRINT));

    if ( OutFile )
        OutFile << "[" << ip << "] ret to " << ret << endl;
    else
        cerr << "[" << ip << "] ret to " << ret << endl;
}

VOID insert_hooks(INS ins, VOID *val)
{
    if ( INS_IsRet(ins) ) {
        INS_InsertCall(
            ins, IPOINT_BEFORE, (AFUNPTR)OnRET,
            IARG_INST_PTR,
            IARG_REG_VALUE, REG_ESP,
            IARG_END);
    }
}

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "None", "specify output file name");

VOID Fini(INT32 code, VOID *v)
{
    if ( OutFile )
        OutFile.close();
}

int main(int argc, char *argv[])
{
    // Initialize PIN library. Print help message if -h(elp) is specified
    // in the command line or the command line is invalid
    if( PIN_Init(argc, argv) )
        return Usage();

    if ( KnobOutputFile.Value() != "None" ) {
        OutFile.open(KnobOutputFile.Value().c_str());
        OutFile.setf(ios::showbase);
        OutFile << hex;
    }

    cerr.setf(ios::showbase);
    cerr << hex;

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(insert_hooks, NULL);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
