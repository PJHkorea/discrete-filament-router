# =========================================================================
# DISCRETE FILAMENT ROUTER (DFR V3) HYBRID INFRASTRUCTURE MASTER MAKEFILE
# =========================================================================

# Compiler standards and optimization barrier configurations
CXX      := g++
CXXFLAGS := -O3 -Wall -std=c++20 -shared -fPIC

# Dynamic include path resolution for pybind11 high-speed data conduits
PYTHON_INCLUDES  := $(shell python3 -m pybind11 --includes)
EXTENSION_SUFFIX := $(shell python3-config --extension-suffix)

# Final target binary extension module specification (Single Source of Truth)
TARGET := c_accelerator_bridge_conduit$(EXTENSION_SUFFIX)
SRC    := src/c_accelerator_bridge_conduit.cpp

.PHONY: all test clean run-sim help

# 1. Baseline Target: Compiles and emits the hardware bridge conduit
all: $(TARGET)
	@echo "➔ 🏰 [C++ Core Bridge] 0ns zero-copy data conduit binary build completed successfully."
	@echo "   | Extension plugin module emitted: $(TARGET)"

$(TARGET): $(SRC)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(PYTHON_INCLUDES) $< -o $@

# 2. Validation Target: Executes automated regression test suites for multiphysics guardrails (TDD)
test: $(TARGET)
	@echo ""
	@echo "➔ 🔬 [CI/CD INFRASTRUCTURE] Activating multiphysics guardrail validation regression test suite..."
	PYTHONPATH=. python3 src/sim/dfr_lithium_jacket_homeostasis_solver.py

# 3. Integration Verification: Global 16-Sector Magnet Spacetimes Integrated Emulation Run
run-sim: $(TARGET)
	@echo ""
	@echo "➔ 🚀 [Digital Twin Engine] Igniting 50Hz traveling-wave synchronization & valve emergency occlusion self-stabilization runtime!"
	PYTHONPATH=. python3 src/sim/magnet_stream_sim.py

# 4. Infrastructure Purge: Comprehensive formatting of compiled register residue
clean:
	rm -f $(TARGET)
	rm -f dfr_homeostasis_sweep.png
	@echo "➔ 🧹 [Safe Purge] Hybrid build pipeline residue formatted successfully. Nominal baseline hardware profile restored."

# 5. Automation Manual & Help Desk
help:
	@echo "====================================================================="
	@echo "🖨️ [DFR V3 INFRASTRUCTURE] Field Engineering Build & Automation Deployment Manual"
	@echo "====================================================================="
	@echo "  • make          : Compiles the C++ PCIe BAR zero-copy bridge conduit (.so output)"
	@echo "  • make test     : Executes multiphysics (Knudsen number, CFL sound speed margins, etc.) guardrail validations"
	@echo "  • make run-sim  : Fires the 4-tier integrated verification full-stack digital twin emulator"
	@echo "  • make clean    : Comprehensive cleanup of compiled binaries and residual graphic plots"
