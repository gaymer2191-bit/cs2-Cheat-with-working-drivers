// language: C++, file: driver.cpp, target: Windows 11 kernel, MSVC WDK
// manual-mapped via kdmapper — no device object registration, comms via shared memory

#include <ntifs.h>
#include <ntddk.h>

#define POOL_TAG 'CS2D'

typedef struct _READ_REQUEST {
    ULONGLONG target_pid;
    ULONGLONG address;
    ULONGLONG buffer;
    ULONG     size;
} READ_REQUEST, *PREAD_REQUEST;

typedef struct _WRITE_REQUEST {
    ULONGLONG target_pid;
    ULONGLONG address;
    ULONGLONG buffer;
    ULONG     size;
} WRITE_REQUEST, *PWRITE_REQUEST;

// shared memory tag — usermode maps this to communicate
static volatile READ_REQUEST*  g_read_req  = nullptr;
static volatile WRITE_REQUEST* g_write_req = nullptr;
static volatile BOOLEAN        g_read_done = FALSE;
static volatile BOOLEAN        g_write_done = FALSE;

NTSTATUS ReadProcessMem(ULONGLONG pid, ULONGLONG addr, PVOID buf, ULONG size) {
    PEPROCESS proc;
    if (!NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)pid, &proc))) return STATUS_NOT_FOUND;

    SIZE_T bytes = 0;
    NTSTATUS status = MmCopyVirtualMemory(
        proc, (PVOID)addr,
        PsGetCurrentProcess(), buf,
        size, KernelMode, &bytes
    );
    ObDereferenceObject(proc);
    return status;
}

NTSTATUS WriteProcessMem(ULONGLONG pid, ULONGLONG addr, PVOID buf, ULONG size) {
    PEPROCESS proc;
    if (!NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)pid, &proc))) return STATUS_NOT_FOUND;

    SIZE_T bytes = 0;
    NTSTATUS status = MmCopyVirtualMemory(
        PsGetCurrentProcess(), buf,
        proc, (PVOID)addr,
        size, KernelMode, &bytes
    );
    ObDereferenceObject(proc);
    return status;
}

// entry — called by kdmapper, no DriverEntry signature needed for manual map
NTSTATUS DriverMain(PDRIVER_OBJECT, PUNICODE_STRING) {
    // allocate shared comms page
    PHYSICAL_ADDRESS max_addr = { 0 };
    max_addr.QuadPart = MAXULONG64;

    PVOID shared = MmAllocateContiguousMemory(PAGE_SIZE, max_addr);
    if (!shared) return STATUS_INSUFFICIENT_RESOURCES;

    RtlZeroMemory(shared, PAGE_SIZE);

    // layout: [0] = read_req, [64] = write_req, [128] = flags
    g_read_req   = (READ_REQUEST*)((UCHAR*)shared + 0);
    g_write_req  = (WRITE_REQUEST*)((UCHAR*)shared + 64);

    // spin loop — replace with proper synch (event/mutex) for prod
    while (TRUE) {
        if (g_read_req->size > 0) {
            UCHAR buf[0x1000] = {};
            ULONG sz = min(g_read_req->size, (ULONG)sizeof(buf));
            ReadProcessMem(g_read_req->target_pid, g_read_req->address, buf, sz);
            RtlCopyMemory((PVOID)g_read_req->buffer, buf, sz);
            g_read_req->size = 0;
        }
        if (g_write_req->size > 0) {
            UCHAR buf[0x1000] = {};
            ULONG sz = min(g_write_req->size, (ULONG)sizeof(buf));
            RtlCopyMemory(buf, (PVOID)g_write_req->buffer, sz);
            WriteProcessMem(g_write_req->target_pid, g_write_req->address, buf, sz);
            g_write_req->size = 0;
        }
        KeDelayExecutionThread(KernelMode, FALSE, (LARGE_INTEGER*)&(LARGE_INTEGER){.QuadPart = -100LL});
    }
    return STATUS_SUCCESS;
}
