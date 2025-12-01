#pragma once
#include <windows.h>
#include <dbghelp.h>
#include <vector>
#include <string>
#include <sstream>



class StackTrace {
public:
    StackTrace() {
        void* stack[64];
        USHORT frames = CaptureStackBackTrace(2, 64, stack, nullptr);
        
        for (USHORT i = 0; i < frames; ++i) {
            char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
            PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            symbol->MaxNameLen = MAX_SYM_NAME;
            
            if (SymFromAddr(GetCurrentProcess(), (DWORD64)(stack[i]), nullptr, symbol)) {
                std::stringstream ss;
                ss << symbol->Name << " + 0x" << std::hex << symbol->Address - (DWORD64)stack[i];
                stack_trace_.push_back(ss.str());
            } else {
                std::stringstream ss;
                ss << "0x" << std::hex << (DWORD64)stack[i];
                stack_trace_.push_back(ss.str());
            }
        }
    }
    
    // 初始化符号处理
    static void InitializeSymbolHandler() {
        SymInitialize(GetCurrentProcess(), nullptr, TRUE);
    }
    
    // 清理符号处理
    static void CleanupSymbolHandler() {
        SymCleanup(GetCurrentProcess());
    }
    
    const std::vector<std::string>& GetStackTrace() const {
        return stack_trace_;
    }
    
    bool operator==(const StackTrace& other) const {
        return stack_trace_ == other.stack_trace_;
    }
    
    size_t Hash() const {
        size_t hash = 0;
        for (const auto& frame : stack_trace_) {
            hash ^= std::hash<std::string>()(frame) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
    
private:
    std::vector<std::string> stack_trace_;
};

namespace std {
    template<> struct hash<StackTrace> {
        size_t operator()(const StackTrace& st) const {
            return st.Hash();
        }
    };
}
