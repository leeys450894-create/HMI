# -*- coding: utf-8 -*-
"""
VTM_MCC shared-memory reader for MCC verification. (LYS 2026-07-31)

Opens the existing mapping "VTM_MCC_Shared_Memory_KMAC" created by dMCCSpec.dll
(inside ALPHA.exe) and dumps:
  - header: nEQType, uMCCUploadTime, bMCCTransfer
  - EQ info: EquipID/Module/Step/Lot/Glass/Slot/Recipe IDs
  - I-Log: axes with bUse=TRUE (pos/speed/rms), nCntILog, dData[0..14]
  - event queue: m_front / m_rear and the last few queued events

Run while ALPHA.exe is running:
  python mcc_sharemem_reader.py            # summary
  python mcc_sharemem_reader.py queue 10   # also dump last 10 queue entries

Layout mirrors HMI\Source\Include\SheardDefineMCC.h compiled as x64 Unicode
(TCHAR = wchar_t = 2 bytes, doubles 8-aligned). Opens EXISTING mapping only
(never creates one) - same rationale as fdc_sharemem_reader.py.
"""
import ctypes
import struct
import sys
from ctypes import wintypes

MAP_NAME = "VTM_MCC_Shared_Memory_KMAC"

# ---- struct layout (x64, TCHAR=2) -------------------------------------------
SZ_MOTION = 64                    # 5*BOOL(20)+pad4 + 5*double(40)
N_AXES = 100
SZ_EQINFO = (4 * 260 + 5 * 32) * 2          # 2400
OFF_EQINFO = 12                             # nEQType,uMCCUploadTime,bMCCTransfer
OFF_ILOG = ((OFF_EQINFO + SZ_EQINFO + 7) // 8) * 8   # align 8 -> 2416
SZ_ILOG = N_AXES * SZ_MOTION + 8 + 100 * 8            # 7208
OFF_FRONT = OFF_ILOG + SZ_ILOG                        # 9624
OFF_REAR = OFF_FRONT + 4
OFF_EVT = OFF_REAR + 4                                # stMCCDataEQ
SZ_EVT = 4 + 64 * 2 + 4 + 4 + 4 + 4                   # 148
OFF_QUEUE = OFF_EVT + SZ_EVT
MAX_QUEUE = 10000
TOTAL = OFF_QUEUE + MAX_QUEUE * SZ_EVT + 12

FILE_MAP_READ = 0x0004
_k32 = ctypes.WinDLL("kernel32", use_last_error=True)
_k32.OpenFileMappingW.argtypes = [wintypes.DWORD, wintypes.BOOL, wintypes.LPCWSTR]
_k32.OpenFileMappingW.restype = wintypes.HANDLE
_k32.MapViewOfFile.argtypes = [wintypes.HANDLE, wintypes.DWORD,
                               wintypes.DWORD, wintypes.DWORD, ctypes.c_size_t]
_k32.MapViewOfFile.restype = ctypes.c_void_p


def wstr(buf):
    return buf.decode("utf-16-le", "replace").split("\x00")[0]


def read(addr, off, length):
    return ctypes.string_at(addr + off, length)


def dump_event(b, label):
    flag, = struct.unpack_from("<i", b, 0)
    t = wstr(b[4:4 + 128])
    spq, step, fromto, stoe = struct.unpack_from("<iiii", b, 132)
    kind = {0: "A", 1: "S", 2: "E", 3: "W"}.get(flag, str(flag))
    se = "START" if stoe else "END"
    print(f"  {label}: [{kind}] no={spq} step={step} fromto={fromto} {se} time={t}")


def main():
    h = _k32.OpenFileMappingW(FILE_MAP_READ, False, MAP_NAME)
    if not h:
        err = ctypes.get_last_error()
        print(f"cannot open mapping '{MAP_NAME}' (WinError {err})")
        if err == 2:
            print("  -> mapping does not exist. Is ALPHA.exe running with [MCC] USE=1?")
        sys.exit(1)
    addr = _k32.MapViewOfFile(h, FILE_MAP_READ, 0, 0, TOTAL)
    if not addr:
        print(f"MapViewOfFile failed (WinError {ctypes.get_last_error()})")
        sys.exit(1)

    eqtype, upload, transfer = struct.unpack("<IIi", read(addr, 0, 12))
    print(f"mapping opened. nEQType={eqtype} uMCCUploadTime={upload} bMCCTransfer={transfer}")

    info = read(addr, OFF_EQINFO, SZ_EQINFO)
    names = ["EquipID", "EquipTo", "EquipFrom", "ModuleID"]
    off = 0
    for n in names:
        print(f"  {n:9s}= '{wstr(info[off:off + 520])}'")
        off += 520
    for n in ["StepID", "LotID", "GlassID", "SlotID", "RecipeID"]:
        print(f"  {n:9s}= '{wstr(info[off:off + 64])}'")
        off += 64

    print("--- I-Log motors (bUse=TRUE) ---")
    used = 0
    for i in range(N_AXES):
        b = read(addr, OFF_ILOG + i * SZ_MOTION, SZ_MOTION)
        buse, = struct.unpack_from("<i", b, 0)
        if buse:
            sspd, spd, spos, pos, rms = struct.unpack_from("<5d", b, 24)
            print(f"  axis[{i:2d}] set_spd={sspd:10.3f} set_pos={spos:10.3f} "
                  f"spd={spd:10.3f} pos={pos:10.3f} rms={rms:8.3f}")
            used += 1
    ncnt, = struct.unpack("<i", read(addr, OFF_ILOG + N_AXES * SZ_MOTION, 4))
    ddata = struct.unpack("<15d", read(addr, OFF_ILOG + N_AXES * SZ_MOTION + 8, 15 * 8))
    print(f"  used axes={used} (MCCNAME.csv motor rows must be {used*5})")
    print(f"  nCntILog={ncnt} (expected 15)")
    labels = ["TEMP_PC_RACK", "TEMP_EB_RACK_1", "TEMP_EB_RACK_2",
              "N01_T_PW", "N01_I_PW", "N01_R_VOL", "N01_T_VOL", "N01_R_CUR", "N01_T_CUR",
              "N02_T_PW", "N02_I_PW", "N02_R_VOL", "N02_T_VOL", "N02_R_CUR", "N02_T_CUR"]
    for lab, v in zip(labels, ddata):
        print(f"  dData {lab:15s}= {v:.4f}")

    front, rear = struct.unpack("<ii", read(addr, OFF_FRONT, 8))
    print(f"--- event queue: front={front} rear={rear} pending={(rear - front) % MAX_QUEUE} ---")
    dump_event(read(addr, OFF_EVT, SZ_EVT), "current slot")

    if len(sys.argv) > 1 and sys.argv[1] == "queue":
        n = int(sys.argv[2]) if len(sys.argv) > 2 else 5
        for k in range(n):
            idx = (rear - 1 - k) % MAX_QUEUE
            dump_event(read(addr, OFF_QUEUE + idx * SZ_EVT, SZ_EVT), f"queue[{idx}]")


if __name__ == "__main__":
    main()
