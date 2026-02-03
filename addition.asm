; 16-bit GPR CPU - Math operation program
; Loads operands from 0x0100 and 0x0101, performs operation, stores result at 0x0102
; Operands must be placed in memory before run (via .WORD or by loader)

.ORG 0

    MOVI R6, 0x100       ; R6 = address of operand A (256)
    LOAD R0, (R6)        ; R0 = mem[R6]
    MOVI R7, 0x101       ; R7 = address of operand B (257)
    LOAD R1, (R7)        ; R1 = mem[R7]

    ADD R0, R1           ; R0 = R0 + R1

    MOVI R2, 0x102       ; R2 = result address (258)
    STORE R0, (R2)       ; mem[R2] = R0

    HALT

; Data section: operands and result (loaded by emulator or .WORD)
; .WORD 0x100 2         ; operand A = 2
; .WORD 0x101 3         ; operand B = 3
; Result will be at 0x102
