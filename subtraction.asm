; Subtract: result = A - B
; Operands at 0x100, 0x101; result at 0x102

.ORG 0

    MOVI R6, 0x100
    LOAD R0, (R6)
    MOVI R7, 0x101
    LOAD R1, (R7)

    SUB R0, R1           ; R0 = R0 - R1

    MOVI R2, 0x102
    STORE R0, (R2)

    HALT
