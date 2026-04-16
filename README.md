# BCH(15,7) Decoder RTL Design and Logic Synthesis
Berlekamp–Massey Algorithm 기반 BCH(15,7) 디코더의 RTL 설계 및 논리합성 프로젝트

## Overview
This project implements a BCH(15,7) decoder using Verilog RTL and evaluates it through C reference modeling, RTL simulation, and logic synthesis.

The decoder targets up to 2-bit error correction using syndrome calculation, Berlekamp–Massey algorithm, Chien search, and error correction.

## Motivation
BCH codes are widely used in error-prone digital systems such as NAND flash, SSD controllers, DRAM ECC, optical storage, wireless communication, and satellite communication.[file:1]

As semiconductor scaling increases memory unreliability and noise-sensitive environments raise BER, stronger ECC hardware becomes essential.[file:1]

## BCH(15,7) Background
- Code length: 15
- Message length: 7
- Parity bits: 8
- Error correction capability: up to 2-bit errors
- Field: GF(2^4)

The project uses the primitive polynomial \(p(x)=x^4+x+1\) and the generator polynomial \(g(x)=x^8+x^7+x^6+x^4+1\).[file:1]

## Decoding Flow
1. Syndrome calculation: \(S_i = r(\alpha^i)\), for \(i=1,2,3,4\)
2. Berlekamp–Massey algorithm: generates the error locator polynomial
3. Chien search: finds error locations
4. Error correction: flips detected error bits and recovers the original 7-bit message

## Hardware Architecture
The top-level decoder is composed of:
- Syndrome Calculator
- BM Engine
- Chien Search Unit
- Correction Unit
- FSM Controller

The design follows a sequential architecture controlled by FSM.[file:1]

## Verification Plan
The verification strategy is based on a C reference model first, then RTL comparison.

### Flow
- Random 7-bit input generation
- BCH encoding in C
- Error injection: 0-bit, 1-bit, or 2-bit errors
- BCH decoding in C reference model
- Same test applied to RTL
- Result comparison

### Tools
- Visual Studio for C reference model
- Synopsys VCS for RTL simulation
- Synopsys Design Compiler for synthesis

## Expected Results
- Successful 2-bit error correction
- Matching outputs between C model and RTL
- Functional RTL implementation in Verilog
- Synthesis results including latency and area analysis

## Project Schedule
| Week | Task |
|---|---|
| 1–2 | BCH theory and GF operations |
| 3–4 | C-based decoder model |
| 5–7 | RTL design for Syndrome / BM / Chien |
| 8 | Integration and testbench verification |
| 9 | Synthesis with Design Compiler |
| 10 | Timing analysis and final report |

## Directory Structure
```bash
.
├── README.md
├── docs/
│   ├── proposal.pdf
│   └── figures/
├── rtl/
│   ├── top.v
│   ├── syndrome.v
│   ├── bm_engine.v
│   ├── chien_search.v
│   └── correction_unit.v
├── tb/
│   └── tb_top.v
├── c_model/
│   ├── encoder.c
│   └── decoder.c
└── scripts/
    └── run_vcs.sh
```

## How to Use
### RTL simulation
```bash
make sim
```

### Synthesis
```bash
make synth
```

### Run C reference model
```bash
cd c_model
make run
```

## Results
- To be updated after RTL completion
- Synthesis area and timing results will be added later
- Waveform screenshots and test vectors will be documented here

## License
To be added
