#pragma once

#define OP_NOP 0x00

// 8bit Load Commands
// ld r,r
#define OP_LD_B_B 0x40
#define OP_LD_B_C 0x41
#define OP_LD_B_D 0x42
#define OP_LD_B_E 0x43
#define OP_LD_B_H 0x44
#define OP_LD_B_L 0x45
#define OP_LD_B_A 0x47

#define OP_LD_C_B 0x48
#define OP_LD_C_C 0x49
#define OP_LD_C_D 0x4A
#define OP_LD_C_E 0x4B
#define OP_LD_C_H 0x4C
#define OP_LD_C_L 0x4D
#define OP_LD_C_A 0x4F

#define OP_LD_D_B 0x50
#define OP_LD_D_C 0x51
#define OP_LD_D_D 0x52
#define OP_LD_D_E 0x53
#define OP_LD_D_H 0x54
#define OP_LD_D_L 0x55
#define OP_LD_D_A 0x57

#define OP_LD_E_B 0x58
#define OP_LD_E_C 0x59
#define OP_LD_E_D 0x5A
#define OP_LD_E_E 0x5B
#define OP_LD_E_H 0x5C
#define OP_LD_E_L 0x5D
#define OP_LD_E_A 0x5F

#define OP_LD_H_B 0x60
#define OP_LD_H_C 0x61
#define OP_LD_H_D 0x62
#define OP_LD_H_E 0x63
#define OP_LD_H_H 0x64
#define OP_LD_H_L 0x65
#define OP_LD_H_A 0x67

#define OP_LD_L_B 0x68
#define OP_LD_L_C 0x69
#define OP_LD_L_D 0x6A
#define OP_LD_L_E 0x6B
#define OP_LD_L_H 0x6C
#define OP_LD_L_L 0x6D
#define OP_LD_L_A 0x6F

#define OP_LD_A_B 0x78
#define OP_LD_A_C 0x79
#define OP_LD_A_D 0x7A
#define OP_LD_A_E 0x7B
#define OP_LD_A_H 0x7C
#define OP_LD_A_L 0x7D
#define OP_LD_A_A 0x7F
//ld r,n

//ld r,[HL]
#define OP_LD_B_ptrHL 0x46
#define OP_LD_C_ptrHL 0x4E
#define OP_LD_D_ptrHL 0x56
#define OP_LD_E_ptrHL 0x4E
#define OP_LD_H_ptrHL 0x66
#define OP_LD_L_ptrHL 0x6E
#define OP_LD_A_ptrHL 0x7E

// 16bit Load Commands
// ld rr,nn
#define OP_LD_BC_NN 0x01
#define OP_LD_DE_NN 0x11
#define OP_LD_HL_NN 0x21
#define OP_LD_SP_NN 0x31
// ld SP, HL
#define OP_LD_SP_HL 0xF9

// Jump Commands
#define OP_JP_NN 0xC3
#define OP_JP_HL 0xE9