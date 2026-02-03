/**
 * 16-bit GPR CPU Emulator - Implementation
 */

#include "gpr_cpu.h"
#include <cstring>
#include <cstdio>

// =============================================================================
// BUS
// =============================================================================

Bus::Bus() {
    memory = new uint16_t[MEMORY_SIZE];
    std::memset(memory, 0, MEMORY_SIZE * sizeof(uint16_t));
}

Bus::~Bus() {
    delete[] memory;
}

uint16_t Bus::read(uint16_t address) const {
    // address is 16-bit so 0..65535; cast to size_t for comparison with MEMORY_SIZE
    if (static_cast<size_t>(address) < MEMORY_SIZE)
        return memory[address];
    return 0;
}

void Bus::write(uint16_t address, uint16_t value) {
    if (static_cast<size_t>(address) < MEMORY_SIZE)
        memory[address] = value;
}

// =============================================================================
// DECODE HELPERS (Bitwise operations for instruction decoding)
// =============================================================================
// In C++, we use:
//   - Right shift (>>) to move a bit field to the least significant bits.
//   - Bitwise AND (&) with a mask to keep only those bits (mask = (1<<n)-1 for n bits).

uint8_t GPRCPU::decodeOpcode(uint16_t inst) {
    // Opcode is in bits 15-12. Shift right by 12 so bits 15-12 become 3-0.
    // Mask with 0xF (binary 1111) to keep only 4 bits.
    return static_cast<uint8_t>((inst >> 12) & 0xFu);
}

uint8_t GPRCPU::decodeRd(uint16_t inst) {
    // Rd is in bits 11-9. Shift right by 9, mask with 0x7 (111) for 3 bits.
    return static_cast<uint8_t>((inst >> 9) & 0x7u);
}

uint8_t GPRCPU::decodeRs(uint16_t inst) {
    // Rs is in bits 8-6. Shift right by 6, mask with 0x7.
    return static_cast<uint8_t>((inst >> 6) & 0x7u);
}

uint16_t GPRCPU::decodeImm9(uint16_t inst) {
    // 9-bit immediate is in bits 8-0. Mask with 0x1FF (9 ones in binary).
    return inst & 0x1FFu;
}

// =============================================================================
// FLAG UPDATES
// =============================================================================

void GPRCPU::setResultFlags(uint16_t result) {
    state.FLAGS &= ~(FLAG_ZERO | FLAG_CARRY | FLAG_NEGATIVE); // Clear all first
    if (result == 0)
        state.FLAGS |= FLAG_ZERO;
    if (result & 0x8000u)  // Bit 15 set = negative in 16-bit signed view
        state.FLAGS |= FLAG_NEGATIVE;
}

void GPRCPU::setAddFlags(uint16_t a, uint16_t b, uint16_t result) {
    state.FLAGS &= ~(FLAG_ZERO | FLAG_CARRY | FLAG_NEGATIVE);
    if (result == 0)
        state.FLAGS |= FLAG_ZERO;
    if (result & 0x8000u)
        state.FLAGS |= FLAG_NEGATIVE;
    // Carry: overflow from bit 15 (sum of two 16-bit values produced 17-bit result)
    if ((a + b) > 0xFFFFu)
        state.FLAGS |= FLAG_CARRY;
}

void GPRCPU::setSubFlags(uint16_t a, uint16_t b, uint16_t result) {
    state.FLAGS &= ~(FLAG_ZERO | FLAG_CARRY | FLAG_NEGATIVE);
    if (result == 0)
        state.FLAGS |= FLAG_ZERO;
    if (result & 0x8000u)
        state.FLAGS |= FLAG_NEGATIVE;
    // Carry here means "no borrow": a >= b. So carry set when a >= b.
    if (a >= b)
        state.FLAGS |= FLAG_CARRY;
}

// =============================================================================
// CPU CONSTRUCTION & RESET
// =============================================================================

GPRCPU::GPRCPU(Bus& bus) : bus(bus), tracing(false) {
    reset();
}

void GPRCPU::reset() {
    std::memset(state.R, 0, sizeof(state.R));
    state.PC = 0;
    state.FLAGS = 0;
    state.halted = false;
}

// =============================================================================
// FETCH-DECODE-EXECUTE (one step)
// =============================================================================

bool GPRCPU::step() {
    if (state.halted)
        return false;

    // --- FETCH: Read instruction at PC from memory via bus ---
    uint16_t instruction = bus.read(state.PC);

    if (tracing) {
        std::printf("\n--- Cycle @ PC=0x%04X ---\n", state.PC);
        std::printf("  Instruction: 0x%04X\n", instruction);
        std::printf("  R0=%04X R1=%04X R2=%04X R3=%04X R4=%04X R5=%04X R6=%04X R7=%04X\n",
                    state.R[0], state.R[1], state.R[2], state.R[3],
                    state.R[4], state.R[5], state.R[6], state.R[7]);
        std::printf("  FLAGS: Z=%d C=%d N=%d\n",
                    (state.FLAGS & FLAG_ZERO) ? 1 : 0,
                    (state.FLAGS & FLAG_CARRY) ? 1 : 0,
                    (state.FLAGS & FLAG_NEGATIVE) ? 1 : 0);
    }

    // --- DECODE: Advance PC to next instruction (most instructions are 1 word) ---
    state.PC += 1;

    // --- EXECUTE: Perform the operation ---
    execute(instruction);

    return !state.halted;
}

void GPRCPU::execute(uint16_t instruction) {
    uint8_t op = decodeOpcode(instruction);
    uint8_t rd = decodeRd(instruction);
    uint8_t rs = decodeRs(instruction);
    uint16_t imm9 = decodeImm9(instruction);

    switch (static_cast<Opcode>(op)) {
        case Opcode::HALT:
            state.halted = true;
            if (tracing) std::printf("  [EXEC] HALT\n");
            break;

        case Opcode::MOVI: {
            // Rd = 9-bit immediate (zero-extended to 16 bits)
            state.R[rd] = imm9;
            setResultFlags(state.R[rd]);
            if (tracing) std::printf("  [EXEC] MOVI R%u, %u\n", rd, imm9);
            break;
        }

        case Opcode::MOV: {
            state.R[rd] = state.R[rs];
            setResultFlags(state.R[rd]);
            if (tracing) std::printf("  [EXEC] MOV R%u, R%u\n", rd, rs);
            break;
        }

        case Opcode::LOAD: {
            uint16_t addr = state.R[rs];
            state.R[rd] = bus.read(addr);
            setResultFlags(state.R[rd]);
            if (tracing) std::printf("  [EXEC] LOAD R%u, (R%u)  ; R%u = mem[0x%04X] = 0x%04X\n", rd, rs, rd, addr, state.R[rd]);
            break;
        }

        case Opcode::STORE: {
            uint16_t addr = state.R[rs];
            bus.write(addr, state.R[rd]);
            if (tracing) std::printf("  [EXEC] STORE R%u, (R%u)  ; mem[0x%04X] = 0x%04X\n", rd, rs, addr, state.R[rd]);
            break;
        }

        case Opcode::ADD: {
            uint16_t a = state.R[rd], b = state.R[rs];
            uint16_t result = a + b;
            state.R[rd] = result;
            setAddFlags(a, b, result);
            if (tracing) std::printf("  [EXEC] ADD R%u, R%u  ; R%u = 0x%04X + 0x%04X = 0x%04X\n", rd, rs, rd, a, b, result);
            break;
        }

        case Opcode::SUB: {
            uint16_t a = state.R[rd], b = state.R[rs];
            uint16_t result = a - b;
            state.R[rd] = result;
            setSubFlags(a, b, result);
            if (tracing) std::printf("  [EXEC] SUB R%u, R%u  ; R%u = 0x%04X - 0x%04X = 0x%04X\n", rd, rs, rd, a, b, result);
            break;
        }

        case Opcode::AND: {
            state.R[rd] = state.R[rd] & state.R[rs];
            setResultFlags(state.R[rd]);
            if (tracing) std::printf("  [EXEC] AND R%u, R%u\n", rd, rs);
            break;
        }

        case Opcode::OR: {
            state.R[rd] = state.R[rd] | state.R[rs];
            setResultFlags(state.R[rd]);
            if (tracing) std::printf("  [EXEC] OR R%u, R%u\n", rd, rs);
            break;
        }

        case Opcode::XOR: {
            state.R[rd] = state.R[rd] ^ state.R[rs];
            setResultFlags(state.R[rd]);
            if (tracing) std::printf("  [EXEC] XOR R%u, R%u\n", rd, rs);
            break;
        }

        case Opcode::NOT: {
            state.R[rd] = ~state.R[rs];
            setResultFlags(state.R[rd]);
            if (tracing) std::printf("  [EXEC] NOT R%u, R%u  ; R%u = ~R%u\n", rd, rs, rd, rs);
            break;
        }

        case Opcode::SHL: {
            uint16_t val = state.R[rd];
            state.R[rd] = val << 1;
            state.FLAGS &= ~(FLAG_ZERO | FLAG_CARRY | FLAG_NEGATIVE);
            if (state.R[rd] == 0) state.FLAGS |= FLAG_ZERO;
            if (state.R[rd] & 0x8000u) state.FLAGS |= FLAG_NEGATIVE;
            if (val & 0x8000u) state.FLAGS |= FLAG_CARRY; // bit 15 was set, so it carried out
            if (tracing) std::printf("  [EXEC] SHL R%u  ; R%u = 0x%04X << 1 = 0x%04X\n", rd, rd, val, state.R[rd]);
            break;
        }

        case Opcode::SHR: {
            uint16_t val = state.R[rd];
            state.R[rd] = val >> 1;
            state.FLAGS &= ~(FLAG_ZERO | FLAG_CARRY | FLAG_NEGATIVE);
            if (state.R[rd] == 0) state.FLAGS |= FLAG_ZERO;
            if (state.R[rd] & 0x8000u) state.FLAGS |= FLAG_NEGATIVE;
            if (val & 1u) state.FLAGS |= FLAG_CARRY; // bit 0 was set, carried out
            if (tracing) std::printf("  [EXEC] SHR R%u  ; R%u = 0x%04X >> 1 = 0x%04X\n", rd, rd, val, state.R[rd]);
            break;
        }

        case Opcode::JMP: {
            state.PC = state.R[rs];
            if (tracing) std::printf("  [EXEC] JMP R%u  ; PC = 0x%04X\n", rs, state.PC);
            break;
        }

        case Opcode::JZ: {
            if (state.FLAGS & FLAG_ZERO) {
                state.PC = state.R[rs];
                if (tracing) std::printf("  [EXEC] JZ R%u  ; Z=1, PC = 0x%04X\n", rs, state.PC);
            } else {
                if (tracing) std::printf("  [EXEC] JZ R%u  ; Z=0, no jump\n", rs);
            }
            break;
        }

        case Opcode::NOP:
        default:
            if (tracing) std::printf("  [EXEC] NOP\n");
            break;
    }
}

// =============================================================================
// RUN (until HALT)
// =============================================================================

size_t GPRCPU::run() {
    size_t cycles = 0;
    while (step())
        ++cycles;
    return cycles;
}
