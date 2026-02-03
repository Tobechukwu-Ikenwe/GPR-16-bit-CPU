# 16-bit GPR CPU Emulator

A fully functional, educational 16-bit RISC-style CPU emulator in C++ with 8 general-purpose registers, 64KB memory, and an explicit Fetch–Decode–Execute (FDE) cycle.

## What This Program Does

When you run the emulator:

1. **Load** – It reads an assembly file (e.g. `addition.asm`), assembles it into 16‑bit machine code, and loads that code into memory starting at address 0.
2. **Input** – It prompts you for two operands (A and B). These are written to memory at addresses 0x100 and 0x101.
3. **Execute** – The CPU runs your program instruction by instruction. For each instruction it performs a **Fetch** (read word at PC), **Decode** (extract opcode and operands), and **Execute** (update registers/memory).
4. **Trace** – After every instruction it prints the current PC, all 8 registers, flags (Z/C/N), and a short description of what just ran.
5. **Halt** – When the program executes `HALT`, execution stops and the emulator prints the final result (typically in R0 and at address 0x102).

**Example run:** You enter `2` and `3`. The `addition.asm` program loads them from 0x100 and 0x101, adds them with `ADD R0, R1`, stores the result at 0x102, and halts. Output shows `R0: 5`, `Result at 0x102: 5`.

## Architecture

- **Registers:** R0–R7 (16-bit GPRs), PC (Program Counter), FLAGS (Zero, Carry, Negative).
- **Memory:** 64KB addressable as 16-bit words (65536 words).
- **Bus:** Simple read/write abstraction between CPU and memory.

## Instruction Set (16-bit encoding)

| Opcode | Mnemonic | Encoding | Description |
|--------|----------|----------|-------------|
| 0 | HALT | - | Stop execution |
| 1 | MOVI Rd, imm9 | Rd, 9-bit imm | Rd = immediate (0–511) |
| 2 | MOV Rd, Rs | Rd, Rs | Rd = Rs |
| 3 | LOAD Rd, (Rs) | Rd, Rs | Rd = memory[Rs] |
| 4 | STORE Rd, (Rs) | Rd, Rs | memory[Rs] = Rd |
| 5 | ADD Rd, Rs | Rd, Rs | Rd = Rd + Rs |
| 6 | SUB Rd, Rs | Rd, Rs | Rd = Rd - Rs |
| 7 | AND Rd, Rs | Rd, Rs | Rd = Rd & Rs |
| 8 | OR Rd, Rs | Rd, Rs | Rd = Rd \| Rs |
| 9 | XOR Rd, Rs | Rd, Rs | Rd = Rd ^ Rs |
| 10 | NOT Rd, Rs | Rd, Rs | Rd = ~Rs |
| 11 | SHL Rd | Rd, Rs | Rd = Rd << 1 |
| 12 | SHR Rd | Rd, Rs | Rd = Rd >> 1 (logical) |
| 13 | JMP Rs | Rs | PC = Rs |
| 14 | JZ Rs | Rs | If Zero flag: PC = Rs |
| 15 | NOP | - | No operation |

**Instruction format:** `[15:12]` opcode, `[11:9]` Rd, `[8:6]` Rs, `[5:0]` unused (or imm low bits for MOVI: `[8:0]` = 9-bit immediate).

## Assembly

Programs are written in `.asm` files. Supported syntax:

- **Instructions:** `MOVI R0, 5`, `LOAD R0, (R6)`, `STORE R0, (R2)`, `ADD R0, R1`, `SUB`, `AND`, `OR`, `XOR`, `NOT`, `SHL`, `SHR`, `JMP`, `JZ`, `HALT`, `NOP`
- **Labels:** `loop:` (for JMP/JZ targets)
- **Directives:** `.ORG 0`, `.WORD addr value` (store value at address)
- **Comments:** `; rest of line`

## Build

- **CMake:** `mkdir build && cd build && cmake .. && cmake --build .`
- **Manual:**  
  `clang++ -std=c++17 -o gpr_emulator main.cpp gpr_cpu.cpp assembler.cpp`  
  or  
  `g++ -std=c++17 -o gpr_emulator main.cpp gpr_cpu.cpp assembler.cpp`  
  or  
  `cl /EHsc /std:c++17 /Fe:gpr_emulator main.cpp gpr_cpu.cpp assembler.cpp`

## Run

```text
./gpr_emulator [program.asm]
```

**Example programs:**
- `addition.asm` – Adds operands at 0x100 and 0x101, stores result at 0x102
- `subtraction.asm` – Subtracts B from A, stores result at 0x102

If no file is given, runs `addition.asm`. You are prompted for operand A and B; trace mode is on by default.

## Trace / Debugger

With tracing enabled, after each FDE cycle the emulator prints:

- Program counter (PC)
- Current instruction (raw 16-bit word)
- All eight GPRs (R0–R7)
- Flags (Z, C, N)
- A short line describing the executed operation

This lets you follow exactly how each instruction changes state.

## File Layout

- `gpr_cpu.h` / `gpr_cpu.cpp` – CPU state, Bus, FDE cycle, instruction execution.
- `assembler.h` / `assembler.cpp` – Assembler for `.asm` files.
- `main.cpp` – Loads `.asm`, assembles into memory, runs CPU.
- `addition.asm` – Add program (A + B → 0x102).
- `subtraction.asm` – Subtract program (A - B → 0x102).

## Bitwise Decoding (for beginners)

Instructions are decoded with shifts and masks:

- **Opcode (4 bits):** `(inst >> 12) & 0xF` — shift bits 15–12 down, keep 4 bits.
- **Rd (3 bits):** `(inst >> 9) & 0x7` — shift bits 11–9 down, keep 3 bits.
- **Rs (3 bits):** `(inst >> 6) & 0x7` — shift bits 8–6 down, keep 3 bits.
- **MOVI immediate (9 bits):** `inst & 0x1FF` — keep the lowest 9 bits.

So: **right shift** brings a field to the LSBs; **AND with a mask** keeps only that field’s bits.
