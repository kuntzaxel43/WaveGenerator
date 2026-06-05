# WaveGenerator
This is a little hobby project to gain some knowledge in creating a small system with a microcontroller and an FPGA, and to learn how they can cooperate. The focus is on learning VHDL.

# 🔗 STM32-FPGA Hybrid Signal Generator

This project is a hybrid function generator. A **STM32 Nucleo** (C/C++) acts as the user interface and controller, while an **FPGA** (VHDL) implements a high-precision DDS core (Direct Digital Synthesis) to generate waveforms in real time.

## 🚀 Overview

Unlike purely software-based solutions, FPGA hardware enables signal generation with extremely low jitter and high frequency stability because the logic operates in parallel and is clock-accurate.

### System Architecture

1. **STM32 (Control Plane):** Manages the user interface (rotary encoder, display) and calculates the increment values for the target frequency.
2. **SPI Bus:** The bridge between the two worlds. The STM32 configures the FPGA like a custom chip.
3. **FPGA (Data Plane):** Contains an SPI slave, a phase accumulator, and a sine LUT (Look-Up Table).
4. **R2R DAC:** A discrete resistor network that converts the 8-bit digital values into an analog voltage.

---

## 🛠 Hardware Requirements

- **MCU:** STM32 Nucleo-64 (e.g. F401RE or G474RE)
- **FPGA:** Any development board (e.g. Lattice iCE40, Cyclone IV, or Spartan 7)
- **Peripherals:**
  - 1x I2C OLED display (SSD1306)
  - 1x rotary encoder
  - 16x resistors (8x 10kΩ, 8x 20kΩ) for the 8-bit R2R DAC
- **Connection:** 4-wire SPI (MOSI, MISO, SCK, CS) + GND

---

## 📂 Project Structure

```
├── stm32_controller/     # STM32 CubeIDE project (C/C++)
│   ├── Core/Src/         # Main logic, SPI driver, display menu
│   └── Drivers/          # HAL drivers & OLED library
├── fpga_vhdl/            # FPGA design (VHDL)
│   ├── rtl/              # VHDL source files
│   │   ├── top.vhd       # Top-level entity
│   │   ├── spi_slave.vhd # Communication interface
│   │   └── dds_core.vhd  # Phase accumulator & LUT
│   ├── simulation/       # Testbenches for ModelSim/GHDL
│   └── constraints/      # Pin mapping (.lpf or .xdc)
└── docs/                 # Schematics & mathematical basics
```

---

## 🔌 Hardware Connections (Pin Mapping)

### 1. SPI & Control (Nucleo to FPGA)

| Signal      | Nucleo Pin (Arduino) | Nucleo Pin (Morpho) | Description                    |
|-------------|----------------------|---------------------|--------------------------------|
| **MOSI**    | D11                  | PA7                 | Data from STM32 -> FPGA        |
| **MISO**    | D12                  | PA6                 | Data from FPGA -> STM32        |
| **SCK**     | D13                  | PA5                 | SPI clock                      |
| **CS / NSS**| D10                  | PB6                 | Chip select (active low)       |
| **GND**     | GND                  | GND                 | Common reference ground        |

User interface connections
The following pins connect the display and rotary encoder to the Nucleo.

| Component  | Nucleo Pin | Zio Connector Pin | Description             |
|------------|------------|-------------------|-------------------------|
| OLED SDA   | PB9        | CN7 - Pin 4 (D14) | I2C data line           |
| OLED SCL   | PB8        | CN7 - Pin 2 (D15) | I2C clock line          |
| Encoder A  | PF12       | CN7 - Pin 1       | Encoder phase A         |
| Encoder B  | PF13       | CN7 - Pin 3       | Encoder phase B         |

### 2. R2R DAC (FPGA to resistor network)

| FPGA Pin   | DAC Bit | R2R Resistor                          |
|------------|---------|----------------------------------------|
| IO_P0 (MSB)| Bit 7   | 10kΩ to pin, 20kΩ in series           |
| IO_P1      | Bit 6   | 10kΩ to pin, 20kΩ in series           |
| ...        | ...     | ...                                    |
| IO_P7 (LSB)| Bit 0   | 10kΩ to pin, 20kΩ in series           |

---

## 📉 The R2R Ladder DAC

### Circuit Concept

```
FPGA Pins (8-Bit)
MSB (Bit 7) --[ 10k ]--+----------- Analog Out
                       |
Bit 6 --------[ 10k ]--+--[ 20k ]--+
                       |
...                     |
                       |
LSB (Bit 0) --[ 10k ]--+--[ 20k ]--+
                       |
                       [ 20k ]
                       |
                       GND
```

### Basics

The R2R network uses binary weighting. Each FPGA pin contributes a portion of the output voltage ($V_{cc}/2$, $V_{cc}/4$, ...). By combining only two resistor values ($R$ and $2R$), a precise voltage divider is created for all 256 steps at 8-bit resolution.

---

## ⚙️ How It Works (DDS)

The frequency is controlled by a **phase accumulator**:

$$f_{out} = \frac{\Delta Phase \cdot f_{clk}}{2^N}$$

The STM32 sends the value $\Delta Phase$ via SPI to the FPGA, which then adjusts the readout speed of the sine table.

---

## 📝 Planned Features

- [ ] **Phase 1:** VHDL simulation of the phase accumulator.
- [ ] **Phase 2:** SPI communication (Nucleo -> FPGA).
- [ ] **Phase 3:** Sine table (LUT) and R2R driving.
- [ ] **Phase 4:** STM32 UI (frequency/waveform selection). 

## Bill of Materials
1. Compute & Logic

    1x STM32 Nucleo-64 board (recommended: NUCLEO-F401RE or NUCLEO-G474RE)
    1x FPGA development board (e.g. TinyFPGA BX, Sipeed Tang Nano, or Digilent Basys 3)

2. User Interface & Display

    1x OLED display (SSD1306), 0.96", 128x64 pixels, I2C interface
    1x rotary encoder (e.g. KY-040) for frequency adjustment

3. Discrete Components (R2R DAC)

    Resistors (metal film, 1% tolerance):
        8x 10 kΩ
        9x 20 kΩ
    Optional: 1x operational amplifier (e.g. TL072) as output buffer

4. Wiring & Prototyping

    1x large breadboard
    Jumper cable set (M-M and M-F)
    USB cable matching the boards (Mini-USB / Micro-USB / USB-C)

5. Measurement Equipment

    USB oscilloscope: Affordable modules (e.g. LHT00L or DSO Shell)

6. **Alternative:** Use the STM board ADC -> ADC driver required, but no oscilloscope -> project within a project

   - **6.1 Configuration:** Set a pin (e.g. PA3) as an ADC input.
   - **6.2 Sampling rate:** Decide how often you measure. For a 10 kHz signal from the FPGA, you should sample at least 100 kHz (Nyquist applies, but oversampling gives a nicer trace).
   - **6.3 DMA (Direct Memory Access):** This is the professional approach. The ADC writes the samples directly into a RAM buffer without CPU intervention for every sample.
   - **6.4 Data export:** Send the buffer via UART (USB virtual COM port) to the PC.

## Software Requirements
Firmware (STM32)

    STM32CubeIDE (compiler & debugger)
    STM32CubeMX (for pin configuration, integrated in the IDE)

Hardware Design (FPGA)

    Lattice: Radiant or OSS CAD Suite (for iCE40 boards)
    Xilinx: Vivado ML Edition
    Altera/Intel: Quartus Prime Lite Edition

## System Requirements

1. STM32 Nucleo (Control Plane)
The STM32 acts as the “brain.” It handles everything related to the user and parameter calculation.

    User interface (UI): Read the rotary encoder (interrupt-based) and drive the OLED display via I2C.
    Frequency calculation: Convert the user input (e.g. 440 Hz) into the binary increment word using the floating point unit (FPU).
    Mode management: Track whether sine, square, or sawtooth is active.
    Communication master: Provide the clock and actively send new parameters via SPI to the FPGA.
    System monitoring: Monitor voltages or errors (optional).

2. FPGA (Data Plane)
The FPGA acts as the “muscle.” It performs repetitive but extremely time-critical tasks that would have too much jitter in software.

    SPI slave interface: Constantly listens on the bus for new data from the STM32 and stores it immediately in internal registers.
    Phase accumulator (real-time counter): A 24-bit or 32-bit counter that adds the increment on every clock cycle (e.g. 50 MHz).
    Waveform generation (LUT): The FPGA uses the top bits of the counter as the address for the ROM storing sine values.
    Hardware synchronization: Ensures the 8-bit value for the DAC is present on all 8 pins exactly at the same time (nanosecond accuracy).
    Signal switching: A digital multiplexer that switches between sine LUT, square logic, or sawtooth counter based on the STM32 command.

Why this separation?

Requirement    STM32 solution                 FPGA solution
Response time  Milliseconds (good enough for humans)  Nanoseconds (needed for signals)
Arithmetic     Floating point                Integer
Parallelism    Sequential                    Parallel

## Systemoverview

       USER INPUT                      ANZEIGE
    +--------------+              +---------------+

    | Drehgeber    |              | OLED Display  |
    | (Frequenz)   |              | (SSD1306)     |
    +-------+------+              +-------+-------+

            |                             ^
            | GPIO (Interrupt)            | I2C
            v                             |
    +-------+-----------------------------+-------+

    |                                             |
    |           STM32 F767ZI (NUCLEO)             |
    |               "The Brain"                   |
    |                                             |
    |  - Berechnet Frequenz-Inkrement (M)         |
    |  - Verwaltet Menü & Benutzereingaben        |
    |  - Master der Kommunikation                 |
    |                                             |
    +----------------------+----------------------+

                           |
                           | SPI BUS (Inkrement M & Befehle)
                           v
    +----------------------+----------------------+

    |                                             |
    |                FPGA (VHDL)                  |
    |               "The Muscle"                  |
    |                                             |
    |  - SPI-Slave Empfänger                      |
    |  - Echtzeit-Phasenakkumulator (DDS)         |
    |  - Wellenform Look-Up Table (LUT)           |
    |                                             |
    +----------------------+----------------------+

                           |
                           | 8-Bit Parallel (Daten @ 50MHz+)
                           v
    +----------------------+----------------------+

    |             R2R-Widerstands-DAC             |
    |         (Wandelt Digital -> Analog)         |
    +----------------------+----------------------+
                           |
                           v
                    ANALOGES SIGNAL
                   (Sinus / Rechteck)

1. Input: Der Nutzer ändert die Frequenz am Drehgeber.
2. Logic: Der STM32 berechnet das Tuning-Wort
und schickt es via SPI los
3. Hardware: Das FPGA addiert
in jedem Taktzyklus auf einen Zähler und liest den passenden Amplitudenwert aus dem Speicher.
4. Output: Die 8-Bit-Werte werden zeitgleich an das Widerstandsnetzwerk ausgegeben

