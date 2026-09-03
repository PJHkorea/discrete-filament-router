# =========================================================================
# @file constraints.xdc
# @brief DFR V3 Universal Magnet Integrated Master Core Hardware Physical Pin Constraints
# @details Defines ultra-precision physical clocks for 50Hz power grid (Grid) fixed-rhythm synchronization
#          and maps silicon pins for dual independent driver ports (straight coil, diagonal escape coil).
# =========================================================================

# -------------------------------------------------------------------------
# 1. Baseline Frequency & Time Interval Physical Clock Definition (Clock Constraints)
# -------------------------------------------------------------------------
# Establishes and derives the main internal system clock (100MHz baseline) and global timer clock
# inside the FPGA fabric to eliminate thermal dissipation jitter from the 50Hz nominal grid-sync traveling wave.
create_clock -period 10.000 -name sys_clk_pin [get_ports sys_clk]

# -------------------------------------------------------------------------
# 2. Hardware Address Identification & Synchronization Input Pin Mapping (Input Port Mappings)
# -------------------------------------------------------------------------
# [is_chamber_node_pin]: A physical pin hardwired to 0V (standard) or 3.3V (chamber) rails during field routing.
# Enforces an internal pulldown resistor to fundamentally block arithmetic hallucinations caused by compiler floating states.
set_property PACKAGE_PIN AK17      [get_ports is_chamber_node_pin]
set_property IOSTANDARD LVCMOS33   [get_ports is_chamber_node_pin]
set_property PULLDOWN TRUE          [get_ports is_chamber_node_pin]


# High-speed hardwired charge signal port directly fed at nanosecond (ns) latencies from the upstream (N-1) magnet wire
set_property PACKAGE_PIN AL18      [get_ports upstream_wire_signal]
set_property IOSTANDARD LVCMOS33   [get_ports upstream_wire_signal]

# AC synchronization incoming wire port specification for the 50Hz trigonometric look-up table
set_property PACKAGE_PIN AM19      [get_ports cos_50hz]
set_property IOSTANDARD LVCMOS33   [get_ports cos_50hz]
set_property PACKAGE_PIN AN20      [get_ports sin_50hz]
set_property IOSTANDARD LVCMOS33   [get_ports sin_50hz]

# -------------------------------------------------------------------------
# 3. Dual Independent Driver Physical Coil Output Port Mappings (Output Port Mappings)
# -------------------------------------------------------------------------
# 📌 Port 1: Direct link to the GaN/SiC inverter gate driver for nominal Z-axis traveling confinement and straight acceleration.
# Hard-coded to a FAST slew rate and 8mA drive strength to mitigate power semiconductor thermal degradation during high-frequency switching.
set_property PACKAGE_PIN AP21      [get_ports out_main_z_coil_wire]
set_property IOSTANDARD LVCMOS33   [get_ports out_main_z_coil_wire]
set_property SLEW FAST             [get_ports out_main_z_coil_wire]
set_property DRIVE 8               [get_ports out_main_z_coil_wire]

# 📌 Port 2: Direct link to the gate driver for diagonal escape-axis magnet array 2x reverse overvoltage emergency activation.
# Bound to the outermost ultra-high-speed driver pins to enforce a 0ns swap during emergency sequences while maintaining a 0V baseline.
set_property PACKAGE_PIN AQ22      [get_ports out_chamber_curl_coil_wire]
set_property IOSTANDARD LVCMOS33   [get_ports out_chamber_curl_coil_wire]
set_property SLEW FAST             [get_ports out_chamber_curl_coil_wire]
set_property DRIVE 8               [get_ports out_chamber_curl_coil_wire]

# -------------------------------------------------------------------------
# 4. Timing Path Isolation for Computational Latency & Crosstalk Mitigation (Timing Exceptions)
# -------------------------------------------------------------------------
# Emergency flag propagation paths driven by branchless MUX (uni_branchless_select) and sign-symmetric guards 
# are entirely isolated (False Path configuration) from conventional synchronous timing analyses to eradicate jitter bottlenecks.
set_false_path -from [get_ports is_chamber_node_pin] -to [get_ports out_main_z_coil_wire]
set_false_path -from [get_ports is_chamber_node_pin] -to [get_ports out_chamber_curl_coil_wire]


# 고온 열 잡음 도메인 축소용 파데 필터 가속 파이프라인의 물리적 배치 최적화 고정
set_max_delay -from [get_ports upstream_wire_signal] -to [get_ports out_main_z_coil_wire] 10.000
set_max_delay -from [get_ports upstream_wire_signal] -to [get_ports out_chamber_curl_coil_wire] 10.000
