/**
 * 16-bit GPR CPU Emulator - Load and run .asm programs
 *
 * Usage: gpr_emulator [program.asm]
 * If no file given, runs program.asm in current directory.
 * Trace is enabled by default.
 */

#include "gpr_cpu.h"
#include "assembler.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>

static void printTraceHeader() {
    std::printf("\n  PC    | R0    R1    R2    R3    R4    R5    R6    R7    | Z C N | Instruction\n");
    std::printf("--------+--------------------------------------------------+-------+----------------\n");
}

int main(int argc, char** argv) {
    const char* asmPath = "addition.asm";
    if (argc >= 2)
        asmPath = argv[1];

    Bus bus;
    GPRCPU cpu(bus);

    AssembleResult ar = assembleFile(asmPath, bus.getMemory(), MEMORY_SIZE);
    if (!ar.ok) {
        std::fprintf(stderr, "Assembly error at line %zu: %s\n", ar.lineNum, ar.error.c_str());
        return 1;
    }

    // Optional: place operands at 0x100 and 0x101 for math programs
    std::printf("Operand A at 0x100 (decimal or 0x...): ");
    std::string sa;
    std::getline(std::cin, sa);
    if (!sa.empty()) {
        uint16_t a = static_cast<uint16_t>(std::stoul(sa, nullptr, 0));
        bus.write(0x100, a);
        std::printf("Operand B at 0x101 (decimal or 0x...): ");
        std::string sb;
        std::getline(std::cin, sb);
        if (!sb.empty()) {
            uint16_t b = static_cast<uint16_t>(std::stoul(sb, nullptr, 0));
            bus.write(0x101, b);
        }
    }

    cpu.trace(true);

    std::printf("\n=== 16-bit GPR CPU Emulator ===\n");
    std::printf("Program: %s\n", asmPath);
    printTraceHeader();

    size_t cycles = 0;
    while (cpu.step())
        cycles++;

    std::printf("\n--- HALTED ---\n");
    std::printf("Total cycles: %zu\n", cycles);
    std::printf("R0: %u (0x%04X)\n", cpu.getState().R[0], cpu.getState().R[0]);
    uint16_t result = bus.read(0x102);
    std::printf("Result at 0x102: %u (0x%04X)\n", result, result);

    return 0;
}
