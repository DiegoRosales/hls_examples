#!/usr/bin/env python3
"""
Cycle through common baud rates on a serial port and score each one by how
much printable ASCII it produces. Useful for figuring out the correct baud
rate when a terminal shows gibberish.

Usage:
    python3 detect_baudrate.py /dev/ttyUSB0
    python3 detect_baudrate.py /dev/ttyUSB0 --timeout 2 --bytes 256
    python3 detect_baudrate.py /dev/ttyUSB0 --baud 115200
"""

import argparse
import string
import sys
import time

try:
    import serial
except ImportError:
    sys.exit("pyserial is required: pip install pyserial")

BAUD_RATES = [1200, 2400, 4800, 9600, 14400, 19200, 38400, 57600, 115200,
              230400, 460800, 921600]

PRINTABLE = set(string.printable.encode())


def score(data: bytes) -> float:
    """Return fraction of bytes that are printable ASCII (0.0–1.0)."""
    if not data:
        return 0.0
    return sum(1 for b in data if b in PRINTABLE) / len(data)


def sample(port: str, baud: int, read_bytes: int, timeout: float) -> bytes:
    try:
        with serial.Serial(port, baud, timeout=timeout,
                           bytesize=8, parity='N', stopbits=1) as s:
            s.reset_input_buffer()
            # Small pause so the board has time to send something
            time.sleep(0.3)
            return s.read(read_bytes)
    except serial.SerialException as e:
        print(f"  [error] {e}")
        return b""


def main():
    parser = argparse.ArgumentParser(description="Detect serial baud rate")
    parser.add_argument("port", help="Serial device (e.g. /dev/ttyUSB0)")
    parser.add_argument("--timeout", type=float, default=2.0,
                        help="Read timeout per baud rate in seconds (default: 2)")
    parser.add_argument("--bytes", type=int, default=128,
                        help="Bytes to read per attempt (default: 128)")
    parser.add_argument("--baud", type=int, default=None,
                        help="Capture at a single baud rate and print raw output")
    args = parser.parse_args()

    if args.baud is not None:
        print(f"Listening on {args.port} at {args.baud} baud — press Ctrl+C to stop.\n")
        try:
            with serial.Serial(args.port, args.baud, timeout=0.1,
                               bytesize=8, parity='N', stopbits=1) as s:
                s.reset_input_buffer()
                while True:
                    data = s.read(256)
                    if data:
                        sys.stdout.write(data.decode("ascii", errors="replace"))
                        sys.stdout.flush()
        except serial.SerialException as e:
            sys.exit(f"Serial error: {e}")
        except KeyboardInterrupt:
            print("\nStopped.")
        return

    results = []

    print(f"Sampling {args.port} at each baud rate "
          f"({args.timeout}s / {args.bytes} bytes per attempt)\n")
    print(f"{'Baud':>10}  {'Bytes':>6}  {'Score':>6}  Preview")
    print("-" * 60)

    for baud in BAUD_RATES:
        data = sample(args.port, baud, args.bytes, args.timeout)
        s = score(data)
        preview = data[:40].decode("ascii", errors="replace").replace("\n", "↵").replace("\r", "")
        print(f"{baud:>10}  {len(data):>6}  {s:>5.1%}  {preview!r}")
        results.append((s, len(data), baud, data))

    results.sort(reverse=True)
    best_score, best_len, best_baud, best_data = results[0]

    print("\n--- Best candidate ---")
    if best_score == 0.0 and best_len == 0:
        print("No data received at any baud rate.")
        print("Check: correct port? board powered? TX/RX wired correctly?")
    elif best_score < 0.3:
        print(f"Baud {best_baud} had the highest score ({best_score:.1%}) "
              f"but still looks like noise — the board may not be sending data yet.")
    else:
        print(f"Baud rate: {best_baud}  (score {best_score:.1%})")
        print(f"Sample output:\n{best_data.decode('ascii', errors='replace')}")


if __name__ == "__main__":
    main()
