#pragma once

#define REG_BC 0x0
#define REG_DE 0x1
#define REG_HL 0x2
#define REG_SP 0x3
#define REG_PC 0x4
#define REG_AF 0x7

#define REG_B 0x0
#define REG_C 0x1
#define REG_D 0x2
#define REG_E 0x3
#define REG_H 0x4
#define REG_L 0x5
#define REG_A 0x7

#define FLAG_NZ 0x0
#define FLAG_Z 0x1
#define FLAG_NC 0x2
#define FLAG_C 0x3

#define OP_NOP 0x00

// 8bit Load Commands
// ld r,r
#define OP_LD_B_B (REG_B << 3 | REG_B | 0x40)
#define OP_LD_B_C (REG_B << 3 | REG_C | 0x40)
#define OP_LD_B_D (REG_B << 3 | REG_D | 0x40)
#define OP_LD_B_E (REG_B << 3 | REG_E | 0x40)
#define OP_LD_B_H (REG_B << 3 | REG_H | 0x40)
#define OP_LD_B_L (REG_B << 3 | REG_L | 0x40)
#define OP_LD_B_A (REG_B << 3 | REG_A | 0x40)

#define OP_LD_C_B (REG_C << 3 | REG_B | 0x40)
#define OP_LD_C_C (REG_C << 3 | REG_C | 0x40)
#define OP_LD_C_D (REG_C << 3 | REG_D | 0x40)
#define OP_LD_C_E (REG_C << 3 | REG_E | 0x40)
#define OP_LD_C_H (REG_C << 3 | REG_H | 0x40)
#define OP_LD_C_L (REG_C << 3 | REG_L | 0x40)
#define OP_LD_C_A (REG_C << 3 | REG_A | 0x40)

#define OP_LD_D_B (REG_D << 3 | REG_B | 0x40)
#define OP_LD_D_C (REG_D << 3 | REG_C | 0x40)
#define OP_LD_D_D (REG_D << 3 | REG_D | 0x40)
#define OP_LD_D_E (REG_D << 3 | REG_E | 0x40)
#define OP_LD_D_H (REG_D << 3 | REG_H | 0x40)
#define OP_LD_D_L (REG_D << 3 | REG_L | 0x40)
#define OP_LD_D_A (REG_D << 3 | REG_A | 0x40)

#define OP_LD_E_B (REG_E << 3 | REG_B | 0x40)
#define OP_LD_E_C (REG_E << 3 | REG_C | 0x40)
#define OP_LD_E_D (REG_E << 3 | REG_D | 0x40)
#define OP_LD_E_E (REG_E << 3 | REG_E | 0x40)
#define OP_LD_E_H (REG_E << 3 | REG_H | 0x40)
#define OP_LD_E_L (REG_E << 3 | REG_L | 0x40)
#define OP_LD_E_A (REG_E << 3 | REG_A | 0x40)

#define OP_LD_H_B (REG_H << 3 | REG_B | 0x40)
#define OP_LD_H_C (REG_H << 3 | REG_C | 0x40)
#define OP_LD_H_D (REG_H << 3 | REG_D | 0x40)
#define OP_LD_H_E (REG_H << 3 | REG_E | 0x40)
#define OP_LD_H_H (REG_H << 3 | REG_H | 0x40)
#define OP_LD_H_L (REG_H << 3 | REG_L | 0x40)
#define OP_LD_H_A (REG_H << 3 | REG_A | 0x40)

#define OP_LD_L_B (REG_L << 3 | REG_B | 0x40)
#define OP_LD_L_C (REG_L << 3 | REG_C | 0x40)
#define OP_LD_L_D (REG_L << 3 | REG_D | 0x40)
#define OP_LD_L_E (REG_L << 3 | REG_E | 0x40)
#define OP_LD_L_H (REG_L << 3 | REG_H | 0x40)
#define OP_LD_L_L (REG_L << 3 | REG_L | 0x40)
#define OP_LD_L_A (REG_L << 3 | REG_A | 0x40)

#define OP_LD_A_B (REG_A << 3 | REG_B | 0x40)
#define OP_LD_A_C (REG_A << 3 | REG_C | 0x40)
#define OP_LD_A_D (REG_A << 3 | REG_D | 0x40)
#define OP_LD_A_E (REG_A << 3 | REG_E | 0x40)
#define OP_LD_A_H (REG_A << 3 | REG_H | 0x40)
#define OP_LD_A_L (REG_A << 3 | REG_L | 0x40)
#define OP_LD_A_A (REG_A << 3 | REG_A | 0x40)
//ld r,n
#define OP_LD_B_n (REG_B << 3 | 6)
#define OP_LD_C_n (REG_C << 3 | 6)
#define OP_LD_D_n (REG_D << 3 | 6)
#define OP_LD_E_n (REG_E << 3 | 6)
#define OP_LD_H_n (REG_H << 3 | 6)
#define OP_LD_L_n (REG_L << 3 | 6)
#define OP_LD_A_n (REG_A << 3 | 6)

//ld [DE], A
#define OP_LD_ptrDE_A 0x12

//ldi A, [HL]
#define OP_LDI_A_ptrHL 0x2A

//ld r,[HL]
#define OP_LD_B_ptrHL 0x46
#define OP_LD_C_ptrHL 0x4E
#define OP_LD_D_ptrHL 0x56
#define OP_LD_E_ptrHL 0x4E
#define OP_LD_H_ptrHL 0x66
#define OP_LD_L_ptrHL 0x6E
#define OP_LD_A_ptrHL 0x7E

// 8bit Arthmetic/logical Commands
// inc r
#define OP_INC_B (REG_B << 3 | 4)
#define OP_INC_C (REG_C << 3 | 4)
#define OP_INC_D (REG_D << 3 | 4)
#define OP_INC_E (REG_E << 3 | 4)
#define OP_INC_H (REG_H << 3 | 4)
#define OP_INC_L (REG_L << 3 | 4)
#define OP_INC_A (REG_A << 3 | 4)

// dec r
#define OP_DEC_B (REG_B << 3 | 5)
#define OP_DEC_C (REG_C << 3 | 5)
#define OP_DEC_D (REG_D << 3 | 5)
#define OP_DEC_E (REG_E << 3 | 5)
#define OP_DEC_H (REG_H << 3 | 5)
#define OP_DEC_L (REG_L << 3 | 5)
#define OP_DEC_A (REG_A << 3 | 5)

// 16bit Load Commands
// ld rr,nn
#define OP_LD_BC_NN 0x01
#define OP_LD_DE_NN 0x11
#define OP_LD_HL_NN 0x21
#define OP_LD_SP_NN 0x31
// ld SP, HL
#define OP_LD_SP_HL 0xF9

// Jump Commands
// jp nn
#define OP_JP_NN 0xC3

// jp HL
#define OP_JP_HL 0xE9

// jr f,PC+dd
#define OP_JR_NZ_dd (FLAG_NZ << 3 | 0x20)
#define OP_JR_Z_dd (FLAG_Z << 3 | 0x20)
#define OP_JR_NC_dd (FLAG_NC << 3 | 0x20)
#define OP_JR_C_dd (FLAG_C << 3 | 0x20)