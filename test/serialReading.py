#!/usr/bin/env python3
import serial
import struct
import time
import argparse
import sys

def main():
    parser = argparse.ArgumentParser(description="Phoenix RC Binary Protocol Diagnostic Tool")
    parser.add_argument('port', nargs='?', default='/dev/ttyACM0', help="Serial port (default: /dev/ttyACM0)")
    parser.add_argument('--baud', type=int, default=115200, help="Baud rate (default: 115200)")
    args = parser.parse_args()

    # Trackers for Min/Max values
    # Initialized with high/low bounds to ensure first packet sets actual values
    stats = {
        "steer":    {"min": 3000, "max": 0},
        "throttle": {"min": 3000, "max": 0},
        "teleop":   {"min": 3000, "max": 0}
    }

    HEADER_BYTE = 0xAA

    try:
        ser = serial.Serial(args.port, args.baud, timeout=1)
        print(f"Connected to {args.port}. Monitoring Phoenix stream...")
        
        time.sleep(2)
        ser.reset_input_buffer()

        while True:
            if ser.read(1) == bytes([HEADER_BYTE]):
                payload = ser.read(6)
                
                if len(payload) == 6:
                    steer, throttle, teleop = struct.unpack('<HHH', payload)
                    
                    # Update Statistics
                    for val, key in zip([steer, throttle, teleop], ["steer", "throttle", "teleop"]):
                        if val < stats[key]["min"]: stats[key]["min"] = val
                        if val > stats[key]["max"]: stats[key]["max"] = val

                    # Clean terminal output
                    output = f"ST: {steer:4} | TH: {throttle:4} | SW: {teleop:4}"
                    sys.stdout.write(f"\r{output}")
                    sys.stdout.flush()

    except KeyboardInterrupt:
        # Professional Exit Summary
        print("\n\n--- Phoenix Session Statistics ---")
        print(f"{'Channel':<10} | {'Min (µs)':<10} | {'Max (µs)':<10}")
        print("-" * 35)
        for key in ["steer", "throttle", "teleop"]:
            s = stats[key]
            # Handle case where no data was collected
            min_val = s['min'] if s['min'] != 3000 else 0
            print(f"{key.capitalize():<10} | {min_val:<10} | {s['max']:<10}")
        print("-" * 35)
        print("Exiting.")

    except Exception as e:
        print(f"\nError: {e}")
    finally:
        if 'ser' in locals():
            ser.close()

if __name__ == "__main__":
    main()