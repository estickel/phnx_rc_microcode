# Phoenix RC Microcode: High-Performance PWM Acquisition

## Technical Overview
The **Phoenix RC Microcode** is a low-latency firmware solution for the ATmega328P microcontroller. It is designed to facilitate the high-frequency acquisition of 3-channel RC receiver PWM signals, providing an optimized data bridge for autonomous navigation stacks. 

By utilizing direct register access and a custom binary framing protocol, the system achieves significantly higher throughput and lower jitter compared to standard Arduino library implementations.



## Key Architectures

### Interrupt Service Routine (ISR)
The firmware utilizes **Pin Change Interrupts (PCINT)** on Port B. Pulse timing is managed via **Bitwise Difference Mapping**, where the XOR operator identifies toggled pins in a single clock cycle. This approach minimizes CPU cycles spent in the ISR, ensuring maximum timing precision for the 1000–2000µs pulses.

### Binary Framing Protocol
To optimize serial bandwidth and reduce parsing overhead on the host computer, data is transmitted as a packed 7-byte binary structure.

| Byte Offset | Field | Type | Description |
| :--- | :--- | :--- | :--- |
| 0x00 | Header | uint8_t | Constant synchronization byte: `0xAA` (10101010) |
| 0x01 | Steer | uint16_t | Channel 1 pulse width in microseconds (Little-Endian) |
| 0x03 | Throttle | uint16_t | Channel 2 pulse width in microseconds (Little-Endian) |
| 0x05 | Teleop | uint16_t | Channel 3 pulse width in microseconds (Little-Endian) |

## Hardware Specifications
* **Target MCU:** ATmega328P (16MHz)
* **Baud Rate:** 115200 bps
* **Transmission Frequency:** 50Hz (20ms period)
* **I/O Mapping:**
    * **Pin 8 (PCINT0):** Steering Input
    * **Pin 9 (PCINT1):** Throttle Input
    * **Pin 10 (PCINT2):** Teleop/Auxiliary Switch



## Repository Structure
* `src/main.cpp`: Primary firmware containing the ISR logic and non-blocking loop.
* `test/serialReading.py`: Python-based diagnostic tool utilizing `pyserial` and `struct` for packet verification.
* `platformio.ini`: Configuration for the PlatformIO build system.

## Build and Deployment
The project is managed via the PlatformIO ecosystem.

### 1. Compilation and Upload
Execute the following command to build the firmware and flash the target microcontroller:
```bash
pio run --target upload