/**
 * Simple assembler for 16-bit GPR CPU.
 * Parses .asm files and produces machine code loaded into memory.
 */

#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

/** Result of assembly: success + optional error message */
struct AssembleResult {
    bool ok;
    std::string error;
    size_t lineNum;
};

/**
 * Assemble source code into memory.
 * Returns AssembleResult; on success, instructions/data are written to mem.
 */
AssembleResult assemble(const std::string& source, uint16_t* mem, size_t memSize);

/** Load and assemble a .asm file. */
AssembleResult assembleFile(const char* path, uint16_t* mem, size_t memSize);

#endif // ASSEMBLER_H
