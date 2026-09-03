# Discrete Filament Router (DFR) - Phase-Shift Offset Matrix Specification for Power Semiconductors
## (Phase-Shift Resolution Matrix Specification for GaN/SiC Fabric Platform)

## 1. Deployment Overview
This document defines the physical phase offset constraint specifications for directly injecting the **50Hz traveling wave (Traveling Wave)** defined in `Normal_Operation_Specs.md` and `unified_magnet_master_core.h` into 16 decentralized electromagnet inverter gate drivers via a hardwired (Hardwired) methodology.

- **Baseline System Clock (sys_clk):** 100 MHz ($\Delta t = 10.000\text{ ns}$ deterministic fixed clock)
- **Target Traveling Wave Frequency:** 50.0 Hz (1-cycle intrinsic time window = $20.0\text{ ms}$)
- **Synchronization Topology:** Each magnet node statically binds its localized fixed timer counter offset value assigned to its specific `Sector ID` into local FPGA/ASIC fabric constants. Subsequently, it utilizes exclusively 1:1 neighbor relay communication to construct a flawless clockwise (CW) electromagnetic transport field across the global loop.

## 2. Fixed Phase-Shift Offset Matrix Table by Magnet Sector
These counter parameters are engineered to map the critical pathways between the master synchronization and physical clock pins (`AP21`, `AQ22`) defined in `constraints.xdc` and the internal accelerator pipeline registers directly into the Input/Output Blocks (IOBs). This design minimizes routing delay and fully satisfies architectural timing closure (Timing Closure) requirements.


| Sector ID | Role | Spatial Angle (°) | Phase Offset (rad) | Phase Offset (°) | FPGA Timer Register Offset (100MHz sys_clk) |
|:---|:---|:---|:---|:---|:---|
| Sector 00 | GENERAL (AP21) | 0.00° | 0.0000 π_rad | 0.00° | 0 Counts |
| Sector 01 | GENERAL (AP21) | 22.50° | 0.1250 π | 22.50° | 125,000 Counts |
| Sector 02 | GENERAL (AP21) | 45.00° | 0.2500 π | 45.00° | 250,000 Counts |
| Sector 03 | GENERAL (AP21) | 67.50° | 0.3750 π | 67.50° | 375,000 Counts |
| Sector 04 | GENERAL (AP21) | 90.00° | 0.5000 π | 90.00° | 500,000 Counts |
| Sector 05 | GENERAL (AP21) | 112.50° | 0.6250 π | 112.50° | 625,000 Counts |
| Sector 06 | GENERAL (AP21) | 135.00° | 0.7500 π | 135.00° | 750,000 Counts |
| Sector 07 | GENERAL (AP21) | 157.50° | 0.8750 π | 157.50° | 875,000 Counts |
| Sector 08 | GENERAL (AP21) | 180.00° | 1.0000 π | 180.00° | 1,000,000 Counts |
| Sector 09 | GENERAL (AP21) | 202.50° | 1.1250 π | 202.50° | 1,125,000 Counts |
| Sector 10 | GENERAL (AP21) | 225.00° | 1.2500 π | 225.00° | 1,250,000 Counts |
| Sector 11 | GENERAL (AP21) | 247.50° | 1.3750 π | 247.50° | 1,375,000 Counts |
| Sector 12 | GENERAL (AP21) | 270.00° | 1.5000 π | 270.00° | 1,500,000 Counts |
| Sector 13 | GENERAL (AP21) | 292.50° | 1.6250 π | 292.50° | 1,625,000 Counts |
| Sector 14 | GENERAL (AP21) | 315.00° | 1.7500 π | 315.00° | 1,750,000 Counts |
| Sector 15 | CHAMBER (AQ22) | 337.50° | 1.8750 π | 337.50° | 1,875,000 Counts |

## 3. Deployment Guardrails & Field Engineering Instructions
1. **[Compile-Time Constant Locking]:** When building the firmware for the GaN/SiC power inverter control MCUs of each respective sector, declare the specified `Timer Register Offset` value as an immutable static constant (Constant) to physically lock and hardcode it at compile time.
2. **[Sector 15 Node Special Interlock Protection]:** Sector 15 operates in a passive mode during nominal conditions, maintaining its magnetic power output at `0.0`. However, upon receiving an upstream node disconnection interrupt, it must **instantly inject an inverse vector (`-sin_50hz * 2.0`) into the diagonal escape-axis physical pin (`AQ22`) within 10 nanoseconds (sub-10ns)**. To guarantee this response, configure a hardware-independent memory barrier that forces the phase timer to constantly execute its 50Hz synchronization count (2,000,000 cycles) in the background without interruption.
3. **[Clockwise Phase Alignment Verification]:** This matrix adheres to a **clockwise (CW) sign matrix multiplication** that advances the phase forward along the time axis. Verify and confirm that as the sector ID increments, the underlying counter thresholds and radian phase angles systematically advance toward the future time horizon to complete flawless traveling-wave synchronization.
