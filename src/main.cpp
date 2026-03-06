/**
 * main.c - RC Receiver Microcode for ATmega328P
 *
 * Description:
 *   This firmware reads PWM signals from a 3-channel RC receiver 
 *   (steering, throttle, teleop) using pin change interrupts.
 *   Each channel’s pulse width is measured in microseconds and 
 *   sent over Serial as a packed binary packet to a host controller.
 *
 * Hardware:
 *   MCU: ATmega328P (Arduino-compatible)
 *   Pins:
 *     - PB0 / D8 : Steering channel (CH0)
 *     - PB1 / D9 : Throttle channel (CH1)
 *     - PB2 / D10: Teleop channel (CH2)
 *
 * Software Details:
 *   - Uses pin change interrupts (PCINT0_vect) to measure pulse widths
 *     accurately without blocking the CPU.
 *   - Pulse widths are stored in `receiver[3]` as uint16_t values (µs)
 *   - ControlPacket struct (packed) sent over Serial every 20 ms:
 *       struct ControlPacket {
 *           uint8_t  header;    // 0xAA
 *           uint16_t steer;     // CH0
 *           uint16_t throttle;  // CH1
 *           uint16_t teleop;    // CH2
 *       }
 *
 * Usage:
 *   - Upload to ATmega328P
 *   - Connect RC receiver to pins D8–D10
 *   - Read binary packets from Serial at 115200 baud
 *
 * Notes:
 *   - Assumes RC pulse widths ~1000–2000 µs
 *   - Modify CH constants if channel assignment changes
 */
#include <Arduino.h>

#define BAUD_RATE 115200 /* 115,200 is a safe standard baud rate for most computers */

volatile uint32_t timer[3]; /* timer is changed to a 32 bit to match micros() type */
volatile uint16_t receiver[3] = {1500, 1500, 1500}; /* 3 base times from receiver. */
volatile byte lastState = 0;

struct __attribute__((packed)) PACKET {
  uint8_t header = 0XAA; /* Optimal Square Wave Tag */
  uint16_t steer;        /* Mapped to index 0 */
  uint16_t throttle;     /* Mapped to index 1 */
  uint16_t teleop;       /* Mapped to index 2 */
}; 

unsigned long last_send_time = 0;

/* Setup: setup the registers, gates and set lastState. */
void setup() {
  Serial.begin(BAUD_RATE);
  PCICR |= (1 << PCIE0); /* Set pin change control integister to enable for Pins 8-13. */
  PCMSK0 |= (1 << PCINT0) | (1 << PCINT1) | (1 << PCINT2); /* Set pin change mask register 0 to enable specific pins 8, 9, 10 in port B */

  /* redudancy for hardware clarity to take input from Pin 8-10*/
  pinMode(8, INPUT);
  pinMode(9, INPUT);
  pinMode(10, INPUT);

  /* Take current state of PINB and compare it to the first ISR Trigger*/
  lastState = PINB;
}

void loop() {
  unsigned long current_time = millis();
  if (current_time - last_send_time >= 20) {
    last_send_time = current_time;
    PACKET p;
    noInterrupts();
    p.steer    = receiver[0];
    p.throttle = receiver[1];
    p.teleop   = receiver[2];
    interrupts();

    Serial.write((uint8_t*)&p, sizeof(p));
  }
}

ISR(PCINT0_vect) {
  uint32_t now = micros();
  byte current_pins = PINB;
  byte changed = current_pins ^ lastState; /* bit wise state change detection */

  /* Channel 1: Pin 8 mapped onto index 0 */
  if (changed & 0x01) {
    if (current_pins & 0x01)
      timer[0] = now;
    else receiver[0] = now - timer[0];
  }
  /* Channel 4: Pin 9 mapped onto index 0 */
  if (changed & 0x02) {
    if (current_pins & 0x02)
      timer[1] = now;
    else receiver[1] = now - timer[1];
  }
  /* Channel 5: Pin 10 mapped onto index 0 */
  if (changed & 0x04) {
    if (current_pins & 0x04)
      timer[2] = now;
    else receiver[2] = now - timer[2];
  }

  lastState = current_pins;
}