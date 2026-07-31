# -*- coding: utf-8 -*-
"""
ELP ElecBox simulator (doc 2026-07-28) for CCommTcpIpObjectPGMon verification.

Acts as the ELP Server on port 5001. HMI(PG_MON, Client) connects to it.
Sends S10F01 status frames periodically and S10F20 alarm frames on keypress,
then prints the S20F01/S20F20 ACK received from HMI.

Usage:
  python elp_elecbox_sim.py [port]
  - Set Network.ini PARA_10: IP=127.0.0.1, PORT=5001, MODE=0, OUT_OF_SERVICE=0
  - Run this first, then start ALPHA.exe (or reconnect from Config > Module page)

Commands (stdin):
  s          : send S10F01 for box1+box2 once
  a 0 13     : send S10F20 with codes 0, 13 (max 10)
  q          : quit
Auto: S10F01 every 10 s.
"""
import socket
import sys
import threading
import time

STX = b"\x02"
ETX = b"\x03"
PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 5001


def frame(payload: str) -> bytes:
    return STX + payload.encode("ascii") + ETX


def s10f01(box: int) -> bytes:
    # Index(2) temp(4) smoke(4) volt(10) harmV(10) curr(10) harmC(10) leak(10) instP(10) accumP(10) = 80
    body = (
        f"{box:02d}"
        + f"{234 + box:04d}"      # 23.4 / 23.5 C
        + f"{5:04d}"              # 5 %
        + f"{220123:010d}"        # 220.123 V
        + f"{1234:010d}"          # 1.234 V
        + f"{15678:010d}"         # 15.678 A
        + f"{2345:010d}"          # 2.345 mA
        + f"{123:010d}"           # 0.123 mA
        + f"{3456:010d}"          # 3.456 kW
        + f"{12345:010d}"         # 12345 kWh
    )
    assert len(body) == 80, len(body)
    return frame(f"1001{len(body):04d}" + body)


def s10f20(codes) -> bytes:
    codes = list(codes)[:10]
    body = "".join(f"{c:05d}" for c in codes)
    return frame(f"1020{len(body):04d}" + body)


def reader(conn):
    while True:
        try:
            data = conn.recv(4096)
        except OSError:
            break
        if not data:
            print("[SIM] HMI disconnected")
            break
        print(f"[SIM] RECV from HMI: {data!r}")


def main():
    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.bind(("0.0.0.0", PORT))
    srv.listen(1)
    print(f"[SIM] ELP ElecBox server listening on :{PORT} ... waiting for HMI")

    conn, addr = srv.accept()
    print(f"[SIM] HMI connected from {addr}")
    threading.Thread(target=reader, args=(conn,), daemon=True).start()

    def periodic():
        while True:
            time.sleep(10)
            try:
                for box in (1, 2):
                    conn.sendall(s10f01(box))
                print("[SIM] S10F01 box1+box2 sent (periodic)")
            except OSError:
                return

    threading.Thread(target=periodic, daemon=True).start()

    while True:
        line = input().strip()
        if line == "q":
            break
        if line == "s":
            for box in (1, 2):
                conn.sendall(s10f01(box))
            print("[SIM] S10F01 box1+box2 sent")
        elif line.startswith("a"):
            try:
                codes = [int(x) for x in line.split()[1:]]
            except ValueError:
                print("usage: a <code> <code> ...   e.g. a 0 13 200")
                continue
            conn.sendall(s10f20(codes))
            print(f"[SIM] S10F20 sent: {codes}")

    conn.close()
    srv.close()


if __name__ == "__main__":
    main()
