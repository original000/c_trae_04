#ifndef STACKWALKER_H
#define STACKWALKER_H

#include <windows.h>
#include <dbghelp.h>
#include <vector>
#include <string>

#pragma comment(lib, "dbghelp.lib")

class StackWalker {
public:
    static std::vector<std::string> GetCallStack() {
        std::vector<std::string> stack;
        HANDLE process = GetCurrentProcess();
        HANDLE thread = GetCurrentThread();

        SymInitialize(process, nullptr, TRUE);

        CONTEXT context;
        RtlCaptureContext(&context);

        STACKFRAME64 stackFrame;
        ZeroMemory(&stackFrame, sizeof(stackFrame));

#ifdef _M_IX86
        stackFrame.AddrPC.Offset = context.Eip;
        stackFrame.AddrPC.Mode = AddrModeFlat;
        stackFrame.AddrFrame.Offset = context.Ebp;
        stackFrame.AddrFrame.Mode = AddrModeFlat;
        stackFrame.AddrStack.Offset = context.Esp;
        stackFrame.AddrStack.Mode = AddrModeFlat;
#elif _M_X64
        stackFrame.AddrPC.Offset = context.Rip;
        stackFrame.AddrPC.Mode = AddrModeFlat;
        stackFrame.AddrFrame.Offset = context.Rbp;
        stackFrame.AddrFrame.Mode = AddrModeFlat;
        stackFrame.AddrStack.Offset = context.Rsp;
        stackFrame.AddrStack.Mode = AddrModeFlat;
#elif _M_IA64
        stackFrame.AddrPC.Offset = context.StIIP;
        stackFrame.AddrPC.Mode = AddrModeFlat;
        stackFrame.AddrFrame.Offset = context.IntSp;
        stackFrame.AddrFrame.Mode = AddrModeFlat;
        stackFrame.AddrBStore.Offset = context.RsBSP;
        stackFrame.AddrBStore.Mode = AddrModeFlat;
        stackFrame.AddrStack.Offset = context.IntSp;
        stackFrame.AddrStack.Mode = AddrModeFlat;
#endif

        char buffer[sizeof(SYMBOL_INFO) + 256 * sizeof(char)];
        ZeroMemory(buffer, sizeof(buffer));

        SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(buffer);
        symbol->MaxNameLen = 255;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

        for (int i = 0; i < 32; ++i) {
            if (!StackWalk64(
                IMAGE_FILE_MACHINE_AMD64,
                process,
                thread,
                &stackFrame,
                &context,
                nullptr,
                SymFunctionTableAccess64,
                SymGetModuleBase64,
                nullptr)) {
                break;
            }

            if (stackFrame.AddrPC.Offset != 0) {
                DWORD64 displacement = 0;
                if (SymFromAddr(process, stackFrame.AddrPC.Offset, &displacement, symbol)) {
                    stack.push_back(symbol->Name);
                } else {
                    char addr[32];
                    sprintf_s(addr, "0x%llx", stackFrame.AddrPC.Offset);
                    stack.push_back(addr);
                }
            }
        }

        SymCleanup(process);
        return stack;
    }
};

#endif // STACKWALKER_H