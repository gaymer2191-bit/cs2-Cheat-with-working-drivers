// language: C++, file: memory.h
// usermode side — communicates with mapped driver via shared page
#pragma once
#include <windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <stdexcept>
#include <string>

// must match driver layout
struct ReadRequest  { uint64_t pid, address, buffer; uint32_t size; };
struct WriteRequest { uint64_t pid, address, buffer; uint32_t size; };

class Memory {
public:
    uint64_t pid      = 0;
    uintptr_t base    = 0; // client.dll base

    // for a real driver comms: map shared physical page, cast to request structs
    // simplified here using ReadProcessMemory — swap out for driver path in prod
    HANDLE proc = INVALID_HANDLE_VALUE;

    Memory(const std::wstring& proc_name) {
        PROCESSENTRY32W entry{}; entry.dwSize = sizeof(entry);
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        while (Process32NextW(snap, &entry))
            if (proc_name == entry.szExeFile) { pid = entry.th32ProcessID; break; }
        CloseHandle(snap);
        if (!pid) throw std::runtime_error("process not found");
        proc = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, (DWORD)pid);
        base = get_module_base(L"client.dll");
    }

    template<typename T>
    T read(uintptr_t addr) const {
        T val{};
        ReadProcessMemory(proc, (LPCVOID)addr, &val, sizeof(T), nullptr);
        return val;
    }

    template<typename T>
    void write(uintptr_t addr, const T& val) const {
        WriteProcessMemory(proc, (LPVOID)addr, &val, sizeof(T), nullptr);
    }

private:
    uintptr_t get_module_base(const std::wstring& mod_name) {
        MODULEENTRY32W me{}; me.dwModuleSize = sizeof(me);
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, (DWORD)pid);
        while (Module32NextW(snap, &me))
            if (mod_name == me.szModule) { CloseHandle(snap); return (uintptr_t)me.modBaseAddr; }
        CloseHandle(snap);
        return 0;
    }
};
