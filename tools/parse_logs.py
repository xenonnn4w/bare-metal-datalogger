#!/usr/bin/env python3
"""Firmware test-automation tool.

Runs the logger's `dump` command, parses the UART output, verifies every
record's CRC status, and prints a pass/fail report. This is the "HDD Tools
and Automation / firmware verification" piece: exactly what a test harness
does against a real device over a serial link.

Usage: parse_logs.py [path-to-datalogger-binary]   (default ./datalogger)
Exit code: 0 if all records pass, 1 if any CRC failed or no records found.
"""
import re
import subprocess
import sys

REC_RE = re.compile(
    r"REC seq=(\d+) tick=(\d+) temp=(-?\d+\.\d+) press=(\d+) crc=(OK|FAIL)"
)


def main() -> int:
    binary = sys.argv[1] if len(sys.argv) > 1 else "./datalogger"

    # Pull the log back over the "UART" (the program's stdout).
    out = subprocess.run([binary, "dump"], capture_output=True, text=True).stdout

    records, failures = [], []
    for line in out.splitlines():
        m = REC_RE.search(line)
        if not m:
            continue
        seq, tick, temp, press, crc = m.groups()
        rec = {
            "seq": int(seq),
            "tick": int(tick),
            "temp": float(temp),
            "press": int(press),
            "crc_ok": crc == "OK",
        }
        records.append(rec)
        if not rec["crc_ok"]:
            failures.append(rec)

    print("=" * 44)
    print("  FIRMWARE VERIFICATION REPORT")
    print("=" * 44)
    print(f"  records read : {len(records)}")
    print(f"  crc passed   : {len(records) - len(failures)}")
    print(f"  crc failed   : {len(failures)}")

    if records:
        temps = [r["temp"] for r in records]
        # Simple anomaly check: flag samples outside an expected band.
        anomalies = [r for r in records if not (20.0 <= r["temp"] <= 35.0)]
        print(f"  temp min/max : {min(temps):.2f} / {max(temps):.2f} C")
        print(f"  anomalies    : {len(anomalies)} (temp outside 20-35 C)")

    # Check sequence numbers are contiguous (no dropped/duplicated records).
    gaps = [
        (records[i - 1]["seq"], records[i]["seq"])
        for i in range(1, len(records))
        if records[i]["seq"] != records[i - 1]["seq"] + 1
    ]
    print(f"  seq gaps     : {len(gaps)}")

    if failures:
        print("-" * 44)
        for r in failures:
            print(f"  !! seq {r['seq']} CRC FAIL")

    ok = bool(records) and not failures
    print("-" * 44)
    print("  RESULT:", "PASS" if ok else "FAIL")
    print("=" * 44)
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
