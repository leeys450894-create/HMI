# -*- coding: utf-8 -*-
"""
VTM_FOR_FDC shared-memory reader for ELP/FDC verification.

Reads stMainWData[] from the file mapping created by ALPHA.exe
(CMCCLogMgr::CreateMemoryMapFDC) and prints non-empty slots.
Run while ALPHA.exe is running.

Usage:
  python fdc_sharemem_reader.py                 # ELP slots 400~439
  python fdc_sharemem_reader.py 0 100           # slots 0~99 (VERSION etc.)
  python fdc_sharemem_reader.py 400 440 NAME    # override the mapping name

Struct (FDCSheardDefine.h):
  VTM_FOR_FDC { int nMainDataCount; RDATA stMainWData[1000];
                DWORD lLiveCheck;   RDATA stFDCWData[1000]; }
  RDATA { char szName[32]; char szData[32]; double nMax; double nMin; }  # 80 bytes
The offset of stMainWData is auto-detected (4 = packed, 8 = MSVC natural
alignment) by looking for the "VERSION" name that ALPHA writes at slot 20.

NOTE (LYS 2026-07-30): this MUST open an EXISTING mapping only.
mmap.mmap(-1, len, name) on Windows silently CREATES a fresh zero-filled
mapping when the name does not exist, which previously made a wrong mapping
name look like "ALPHA is running but published nothing". We now go through
OpenFileMapping() so a missing/misnamed mapping fails loudly instead.
"""
import ctypes
import struct
import sys
from ctypes import wintypes

# Must match VTM_SHARE_MEM_STRING_FDC in HMI\Source\Include\FDCSheardDefine.h.
# (Older builds used "KMAC_Shared_Memory_For_FDC_V2" - pass it as argv[3] to
#  inspect a legacy FDC/ALPHA pair.)
MAP_NAME = "HBS_Shared_Memory_For_FDC_V2"

RDATA_SIZE = 80
N_SLOTS = 1000

SLOT_FROM = int(sys.argv[1]) if len(sys.argv) > 1 else 400
SLOT_TO = int(sys.argv[2]) if len(sys.argv) > 2 else 440
if len(sys.argv) > 3:
    MAP_NAME = sys.argv[3]

FILE_MAP_READ = 0x0004

_k32 = ctypes.WinDLL("kernel32", use_last_error=True)
_k32.OpenFileMappingW.argtypes = [wintypes.DWORD, wintypes.BOOL, wintypes.LPCWSTR]
_k32.OpenFileMappingW.restype = wintypes.HANDLE
_k32.MapViewOfFile.argtypes = [wintypes.HANDLE, wintypes.DWORD,
                               wintypes.DWORD, wintypes.DWORD, ctypes.c_size_t]
_k32.MapViewOfFile.restype = ctypes.c_void_p
_k32.UnmapViewOfFile.argtypes = [ctypes.c_void_p]
_k32.CloseHandle.argtypes = [wintypes.HANDLE]


def open_existing():
    """Open the mapping; never create one. Returns (addr, handle, size)."""
    handle = _k32.OpenFileMappingW(FILE_MAP_READ, False, MAP_NAME)
    if not handle:
        err = ctypes.get_last_error()
        print(f"cannot open mapping '{MAP_NAME}' (WinError {err})")
        if err == 2:
            print("  -> the mapping does not exist.")
            print("     Is ALPHA.exe running? Does the name match")
            print("     VTM_SHARE_MEM_STRING_FDC in FDCSheardDefine.h?")
        sys.exit(1)

    size = 8 + N_SLOTS * RDATA_SIZE          # stMainWData region is enough
    addr = _k32.MapViewOfFile(handle, FILE_MAP_READ, 0, 0, size)
    if not addr:
        err = ctypes.get_last_error()
        print(f"MapViewOfFile failed (WinError {err})")
        _k32.CloseHandle(handle)
        sys.exit(1)
    return addr, handle, size


def read(addr, off, length):
    return ctypes.string_at(addr + off, length)


def detect_base(addr) -> int:
    for base in (8, 4):   # 8 = int + 4 pad (MSVC natural), 4 = packed
        off = base + 20 * RDATA_SIZE
        if read(addr, off, 32).split(b"\0")[0] == b"VERSION":
            return base
    print("warning: slot20 'VERSION' not found - defaulting to base 8")
    print("         (UpdateSensor() may not have run yet - it is driven by")
    print("          CDlgSubOperate's 1s timer, created in OnInitDialog)")
    return 8


def main():
    addr, handle, _ = open_existing()
    try:
        base = detect_base(addr)
        count = struct.unpack("<i", read(addr, 0, 4))[0]
        print(f"mapping '{MAP_NAME}' opened (existing)")
        print(f"base offset={base}, nMainDataCount={count} "
              f"(ELP slots sit outside this count by design)")
        print(f"--- stMainWData[{SLOT_FROM}..{SLOT_TO - 1}] ---")

        shown = 0
        for i in range(SLOT_FROM, min(SLOT_TO, N_SLOTS)):
            off = base + i * RDATA_SIZE
            name = read(addr, off, 32).split(b"\0")[0].decode("cp949", "replace")
            data = read(addr, off + 32, 32).split(b"\0")[0].decode("ascii", "replace")
            if name:
                print(f"[{i:4d}] {name:<20} = {data}")
                shown += 1
        if shown == 0:
            print("(no named slots in range)")
    finally:
        _k32.UnmapViewOfFile(addr)
        _k32.CloseHandle(handle)


if __name__ == "__main__":
    main()
