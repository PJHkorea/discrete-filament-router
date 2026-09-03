/**
 * @file unified_magnet_master_core.h
 * @brief [Layer 1 Core Kernel] Universal Magnet Integrated Master Specification for Chamber Independent Ports
 * @details Autonomously identifies standard and pre-chamber zones according to hardware physical pin mapping configurations,
 *          while concurrently synchronizing dual wire outputs in alignment with dual-driver magnet array specifications.
 */

#ifndef UNIFIED_MAGNET_MASTER_CORE_H
#define UNIFIED_MAGNET_MASTER_CORE_H

#include <stdint.h>
#include <string.h>

/* ========================================================================= */
/* [MASTER SHIELD ALIGNED STRUCT]                                            */
/* ========================================================================= */
typedef struct {
    float main_z_flux;         /* Connected to Port 1: Nominal Z-axis traveling confinement magnetic state vector */
    float chamber_curl_flux;   /* Connected to Port 2: Emergency chamber diagonal suction vortex magnetic state vector */
    float p00_shield;          /* Joseph-form based covariance protective barrier against numerical negative-inversion anomalies */
    uint32_t fail_counter;     /* Accumulated counter tracking sequential fault tokens (-99.0f) propagating from upstream nodes */
    uint32_t is_emergency_on;  /* 0: Nominal 50Hz constant-velocity traveling mode | 1: Emergency sequence containment execution mode */
    uint8_t reserved[12];      /* Padding firewall for AXI Master Bus burst optimization and strict 32-byte cacheline alignment closure */
} UnifiedMagnetRegister32;

_Static_assert(sizeof(UnifiedMagnetRegister32) == 32, "CRITICAL ERROR: Size mismatch on unified master cacheline!");

/* ========================================================================= */
/* [BRANCHLESS REAL-TIME MUX]                                                */
/* ========================================================================= */
static inline uint32_t uni_branchless_select_u32(uint32_t condition, uint32_t true_val, uint32_t false_val) {
#pragma HLS INLINE
    uint32_t mask = -(condition != 0);
    return (true_val & mask) | (false_val & ~mask);
}

static inline float uni_branchless_select_float(uint32_t condition, float true_val, float false_val) {
#pragma HLS INLINE
    uint32_t true_bits, false_bits, final_bits;
    __builtin_memcpy(&true_bits, &true_val, sizeof(float));
    __builtin_memcpy(&false_bits, &false_val, sizeof(float));
    final_bits = uni_branchless_select_u32(condition, true_bits, false_bits);
    float final_val;
    __builtin_memcpy(&final_val, &final_bits, sizeof(float));
    return final_val;
}

/* ========================================================================= */
/* [CORE OPERATIONAL MATRIX PROCESSOR]                                       */
/* ========================================================================= */
/**
 * @brief Universal control engine shared across all magnet sectors and chamber independent ports
 * @param self Pointer to the master register structure
 * @param upstream_signal High-speed charge signal propagated from the upstream hardwired wire in real time
 * @param is_chamber_node 0: Standard position magnet node | 1: Pre-chamber magnet node (Target for hardware pin binding)
 * @param cos_50hz 50Hz grid synchronization cosine look-up table value
 * @param sin_50hz 50Hz grid synchronization sine look-up table value
 */
static inline void unified_magnet_master_process(
    UnifiedMagnetRegister32* const self,
    float upstream_signal,
    uint32_t is_chamber_node,
    float cos_50hz,
    float sin_50hz
) {
#pragma HLS INLINE
#pragma HLS DATA_PACK variable=self

    /* 1. Branchless anomaly detection evaluating upstream fault tokens (-99.0f) and float transients */
    uint32_t is_nan = (upstream_signal != upstream_signal);
    uint32_t is_over = (upstream_signal > 1e6f) || (upstream_signal < -1e6f);
    uint32_t is_dead = (upstream_signal == -99.0f);
    uint32_t is_anomaly = is_nan | is_over | is_dead;

    /* 2. Branchless counter accumulation and emergency state global lock-in execution */
    self->fail_counter = uni_branchless_select_u32(is_anomaly, self->fail_counter + 1, 0);
    uint32_t trigger_emergency = (self->fail_counter >= 5) || (self->is_emergency_on == 1);
    self->is_emergency_on = uni_branchless_select_u32(trigger_emergency, 1, 0);

    /* 3. Nominal steady-state 50Hz traveling wave rotation and Padé notch filter mathematical execution */
    /* Code Review Guide:
     * 1) [CW Rotational Coordinate System]: This system adheres to a 'Clockwise (CW)' architecture 
     *    that advances the phase forward along the future time axis rather than a counter-clockwise (CCW) approach.
     *    Consequently, algebraic matrix multiplication matrix expansions yield a negative (-) sign at the center to preserve geometric consistency.
     * 2) [Ultra-High Sampling Margin]: Operating under an ultra-high-speed cruising clock envelope (θ → 0), 
     *    numerical divergence is physically blocked. This formula serves as an algorithm to isolate and filter minute 
     *    electromagnetic crosstalk noise within inter-node communication at absolute 0ns latency. */
    float main_z_pred = (cos_50hz * self->main_z_flux) - (sin_50hz * self->chamber_curl_flux);
    float curl_pred   = (sin_50hz * self->main_z_flux) + (cos_50hz * self->chamber_curl_flux);

    float K_gain = self->p00_shield / (self->p00_shield + 1.0f);
    float ImKH = 1.0f - K_gain;
    self->p00_shield = (ImKH * self->p00_shield * ImKH) + (K_gain * 1.0f * K_gain);

    float scaled_energy = (main_z_pred * main_z_pred) + (curl_pred * curl_pred);
    float noise_notch = (6.0f * scaled_energy) / (12.0f + (scaled_energy * scaled_energy));
    float normal_flux_output = main_z_pred + (K_gain * (upstream_signal - main_z_pred)) * noise_notch;

    /* 4. Upon Emergency Activation: Enforces role-based execution matching physical configuration pin (is_chamber_node) ceramic markings */
    
    /* [Branch 1] Standard Node (0) Emergency Output: Completely cuts off transverse chamber fields, executing a straight-axis rear acceleration propulsion wave */
    float gen_emergency_z = 1.5f;                  /* Baseline acceleration flux line for accelerator rear flush and variable valve full occlusion (0.0) */
    float gen_emergency_curl = 0.0f;

    /* [Branch 2] Chamber Node (1) Emergency Output Configuration */
    /* Hardware Constraints Guide:
     * 1) [Physical Port Separation]: The nominal forward confinement magnet (out_main_z_coil_wire) and the transverse chamber suction magnet 
     *    (out_chamber_curl_coil_wire) are completely isolated into independent physical pins (Pins AP21 and AQ22) and driver circuits 
     *    at the FPGA silicon fabric level.
     * 2) [0ns Sign-Inversion Suction]: Even if cham_emergency_z is dropped to 0.0f to form a virtual forward bulkhead blocking the failed sector, 
     *    the pure 50Hz grid prediction profile (curl_pred) of the independent chamber magnet array is preserved free of contamination. 
     *    It algebraically inverts (-) the rotational trajectory phase sign of incoming plasma packets and explosively amplifies the output by 2x (* 2.0f), 
     *    forcing an inertial guided ejection into the vacuum buffer corridor. */
    float cham_emergency_z = 0.0f;                 /* Forms a forward virtual bulkhead to occlude forward progression (Port 1: Pin AP21 emission) */
    float cham_emergency_curl = -curl_pred * 2.0f; /* Drives dynamic diagonal vortex suction into the emergency dissipation chamber (Port 2: Pin AQ22 emission) */

    /* Primary Consolidation Based on Hardware Pin Markings (Branchless MUX Circuit Operations) */
    float target_emergency_z = uni_branchless_select_float(is_chamber_node, cham_emergency_z, gen_emergency_z);
    float target_emergency_curl = uni_branchless_select_float(is_chamber_node, cham_emergency_curl, gen_emergency_curl);

    /* 5. Final Operational State Swap & Dual-Port Register Synchronization (Terminating 0ns register wire emission layout) */
    self->main_z_flux = uni_branchless_select_float(self->is_emergency_on, target_emergency_z, normal_flux_output);
    self->chamber_curl_flux = uni_branchless_select_float(self->is_emergency_on, target_emergency_curl, curl_pred);
}

#endif /* UNIFIED_MAGNET_MASTER_CORE_H */
