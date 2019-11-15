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

// Rarely used flags
#define FLAG_N 0x04
#define FLAG_H 0x05

#define VBLANK_MASK 0x1
#define LCD_STAT_MASK 0x2
#define TIMER_MASK 0x4
#define SERIAL_MASK 0x8
#define JOYPAD_MASK 0x10

#define OP_NOP 0x00

//--------------------------------8bit Load Commands--------------------------------
// ld (nn), SP
#define OP_LD_ptrnn_SP 0x08

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
// ld r,n
#define OP_LD_B_n (REG_B << 3 | 6)
#define OP_LD_C_n (REG_C << 3 | 6)
#define OP_LD_D_n (REG_D << 3 | 6)
#define OP_LD_E_n (REG_E << 3 | 6)
#define OP_LD_H_n (REG_H << 3 | 6)
#define OP_LD_L_n (REG_L << 3 | 6)
#define OP_LD_A_n (REG_A << 3 | 6)

// ld (DE), A
#define OP_LD_ptrDE_A 0x12

// ld A,(FF00+n)
#define OP_LD_A_IOn 0xF0

// ld (FF00+n),A
#define OP_LD_IOn_A 0xE0

// ld (FF00+C),A
#define OP_LD_IOC_A 0xE2

// ldi A, (HL)
#define OP_LDI_A_ptrHL 0x2A

// ldi (HL), A
#define OP_LDI_ptrHL_A 0x22

// ldd (HL), A
#define OP_LDD_ptrHL_A 0x32

// ld r,(HL)
#define OP_LD_B_ptrHL (REG_B << 3 | 0x46)
#define OP_LD_C_ptrHL (REG_C << 3 | 0x46)
#define OP_LD_D_ptrHL (REG_D << 3 | 0x46)
#define OP_LD_E_ptrHL (REG_E << 3 | 0x46)
#define OP_LD_H_ptrHL (REG_H << 3 | 0x46)
#define OP_LD_L_ptrHL (REG_L << 3 | 0x46)
#define OP_LD_A_ptrHL (REG_A << 3 | 0x46)

// ld (HL),r
#define OP_LD_ptrHL_B (0x70 | REG_B)
#define OP_LD_ptrHL_C (0x70 | REG_C)
#define OP_LD_ptrHL_D (0x70 | REG_D)
#define OP_LD_ptrHL_E (0x70 | REG_E)
#define OP_LD_ptrHL_H (0x70 | REG_H)
#define OP_LD_ptrHL_L (0x70 | REG_L)
#define OP_LD_ptrHL_A (0x70 | REG_A)

// ld (HL),n
#define OP_LD_ptrHL_n 0x36

// ld A,(BC)
#define OP_LD_A_ptrBC 0x0A

// ld A,(DE)
#define OP_LD_A_ptrDE 0x1A

// ld A,(nn)
#define OP_LD_A_ptrnn 0xFA

// ld (BC),A
#define OP_LD_ptrBC_A 0x02

// ld (nn), A
#define OP_LD_ptrnn_A 0xEA

//--------------------------------8bit Arthmetic/logical Commands---------------------
// add r
#define OP_ADD_B (0x80 | REG_B)
#define OP_ADD_C (0x80 | REG_C)
#define OP_ADD_D (0x80 | REG_D)
#define OP_ADD_E (0x80 | REG_E)
#define OP_ADD_H (0x80 | REG_H)
#define OP_ADD_L (0x80 | REG_L)
#define OP_ADD_A (0x80 | REG_A)

// add A,n
#define OP_ADD_A_n 0xC6

// add A,(HL)
#define OP_ADD_A_ptrHL 0x86

// adc r
#define OP_ADC_B (0x88 | REG_B)
#define OP_ADC_C (0x88 | REG_C)
#define OP_ADC_D (0x88 | REG_D)
#define OP_ADC_E (0x88 | REG_E)
#define OP_ADC_H (0x88 | REG_H)
#define OP_ADC_L (0x88 | REG_L)
#define OP_ADC_A (0x88 | REG_A)

// adc A,(HL)
#define OP_ADC_A_ptrHL 0x8E

// sub r
#define OP_SUB_B (0x90 | REG_B)
#define OP_SUB_C (0x90 | REG_C)
#define OP_SUB_D (0x90 | REG_D)
#define OP_SUB_E (0x90 | REG_E)
#define OP_SUB_H (0x90 | REG_H)
#define OP_SUB_L (0x90 | REG_L)
#define OP_SUB_A (0x90 | REG_A)

// sub A,n
#define OP_SUB_A_n 0xD6

// sbc A,r
#define OP_SBC_B (0x98 | REG_B)
#define OP_SBC_C (0x98 | REG_C)
#define OP_SBC_D (0x98 | REG_D)
#define OP_SBC_E (0x98 | REG_E)
#define OP_SBC_H (0x98 | REG_H)
#define OP_SBC_L (0x98 | REG_L)
#define OP_SBC_A (0x98 | REG_A)

// sbc A,(HL)
#define OP_SBC_ptrHL 0x9E

// and r
#define OP_AND_B (0xA0 | REG_B)
#define OP_AND_C (0xA0 | REG_C)
#define OP_AND_D (0xA0 | REG_D)
#define OP_AND_E (0xA0 | REG_E)
#define OP_AND_H (0xA0 | REG_H)
#define OP_AND_L (0xA0 | REG_L)
#define OP_AND_A (0xA0 | REG_A)

// and n
#define OP_AND_A_nn 0xE6

// xor r
#define OP_XOR_B (0xA8 | REG_B)
#define OP_XOR_C (0xA8 | REG_C)
#define OP_XOR_D (0xA8 | REG_D)
#define OP_XOR_E (0xA8 | REG_E)
#define OP_XOR_H (0xA8 | REG_H)
#define OP_XOR_L (0xA8 | REG_L)
#define OP_XOR_A (0xA8 | REG_A)

// xor (HL)
#define OP_XOR_ptrHL 0xAE

// or r
#define OP_OR_B (0xB0 | REG_B)
#define OP_OR_C (0xB0 | REG_C)
#define OP_OR_D (0xB0 | REG_D)
#define OP_OR_E (0xB0 | REG_E)
#define OP_OR_H (0xB0 | REG_H)
#define OP_OR_L (0xB0 | REG_L)
#define OP_OR_A (0xB0 | REG_A)

// or n
#define OP_OR_n 0xF6

// or A, (HL)
#define OP_OR_ptrHL 0xB6

// cp r
#define OP_CP_B (0xB8 | REG_B)
#define OP_CP_C (0xB8 | REG_C)
#define OP_CP_D (0xB8 | REG_D)
#define OP_CP_E (0xB8 | REG_E)
#define OP_CP_H (0xB8 | REG_H)
#define OP_CP_L (0xB8 | REG_L)
#define OP_CP_A (0xB8 | REG_A)

// cp n
#define OP_CP_n 0xFE

// cp (HL)
#define OP_CP_ptrHL 0xBE

// inc r
#define OP_INC_B (REG_B << 3 | 4)
#define OP_INC_C (REG_C << 3 | 4)
#define OP_INC_D (REG_D << 3 | 4)
#define OP_INC_E (REG_E << 3 | 4)
#define OP_INC_H (REG_H << 3 | 4)
#define OP_INC_L (REG_L << 3 | 4)
#define OP_INC_A (REG_A << 3 | 4)

// inc (HL)
#define OP_INC_ptrHL 0x34

// dec r
#define OP_DEC_B (REG_B << 3 | 5)
#define OP_DEC_C (REG_C << 3 | 5)
#define OP_DEC_D (REG_D << 3 | 5)
#define OP_DEC_E (REG_E << 3 | 5)
#define OP_DEC_H (REG_H << 3 | 5)
#define OP_DEC_L (REG_L << 3 | 5)
#define OP_DEC_A (REG_A << 3 | 5)

// dec (HL)
#define OP_DEC_ptrHL 0x35

// daa
#define OP_DAA 0x27

// cpl
#define OP_CPL 0x2F

//--------------------------------16bit Load Commands--------------------------------
// ld rr,nn
#define OP_LD_BC_nn 0x01
#define OP_LD_DE_nn 0x11
#define OP_LD_HL_nn 0x21
#define OP_LD_SP_nn 0x31
// ld SP, HL
#define OP_LD_SP_HL 0xF9

// push rr
#define OP_PUSH_BC (REG_BC << 4 | 0xC5)
#define OP_PUSH_DE (REG_DE << 4 | 0xC5)
#define OP_PUSH_HL (REG_HL << 4 | 0xC5)
#define OP_PUSH_AF (REG_AF << 4 | 0xC5)

// pop rr
#define OP_POP_BC (REG_BC << 4 | 0xC1)
#define OP_POP_DE (REG_DE << 4 | 0xC1)
#define OP_POP_HL (REG_HL << 4 | 0xC1)
#define OP_POP_AF (REG_AF << 4 | 0xC1)

//--------------------------------16bit Arithmetic Commands----------------------------
// add HL,rr
#define OP_ADD_HL_BC (REG_BC << 4 | 0x9)
#define OP_ADD_HL_DE (REG_DE << 4 | 0x9)
#define OP_ADD_HL_HL (REG_HL << 4 | 0x9)
#define OP_ADD_HL_SP (REG_SP << 4 | 0x9)

// inc rr
#define OP_INC_BC (REG_BC << 4 | 0x3)
#define OP_INC_DE (REG_DE << 4 | 0x3)
#define OP_INC_HL (REG_HL << 4 | 0x3)
#define OP_INC_SP (REG_SP << 4 | 0x3)

// dec rr
#define OP_DEC_BC (REG_BC << 4 | 0xB)
#define OP_DEC_DE (REG_DE << 4 | 0xB)
#define OP_DEC_HL (REG_HL << 4 | 0xB)
#define OP_DEC_SP (REG_SP << 4 | 0xB)

// add SP,dd
#define OP_ADD_SP_dd 0xE8

//------------------------------Rotate/Shift Commands---------------------------------
// rlca
#define OP_RLCA 0x07

// rla
#define OP_RLA 0x17

// rra
#define OP_RRA 0x1F

// rl r
#define OP_CB_RL_B (REG_B | 0x10)
#define OP_CB_RL_C (REG_C | 0x10)
#define OP_CB_RL_D (REG_D | 0x10)
#define OP_CB_RL_E (REG_E | 0x10)
#define OP_CB_RL_H (REG_H | 0x10)
#define OP_CB_RL_L (REG_L | 0x10)
#define OP_CB_RL_A (REG_A | 0x10)

// rr r
#define OP_CB_RR_B (REG_B | 0x18)
#define OP_CB_RR_C (REG_C | 0x18)
#define OP_CB_RR_D (REG_D | 0x18)
#define OP_CB_RR_E (REG_E | 0x18)
#define OP_CB_RR_H (REG_H | 0x18)
#define OP_CB_RR_L (REG_L | 0x18)
#define OP_CB_RR_A (REG_A | 0x18)

// rr (HL)
#define OP_CB_RR_ptrHL 0x1E

// sla r
#define OP_CB_SLA_B (REG_B | 0x20)
#define OP_CB_SLA_C (REG_C | 0x20)
#define OP_CB_SLA_D (REG_D | 0x20)
#define OP_CB_SLA_E (REG_E | 0x20)
#define OP_CB_SLA_H (REG_H | 0x20)
#define OP_CB_SLA_L (REG_L | 0x20)
#define OP_CB_SLA_A (REG_A | 0x20)

// swap r
#define OP_CB_SWAP_B (REG_B | 0x30)
#define OP_CB_SWAP_C (REG_C | 0x30)
#define OP_CB_SWAP_D (REG_D | 0x30)
#define OP_CB_SWAP_E (REG_E | 0x30)
#define OP_CB_SWAP_H (REG_H | 0x30)
#define OP_CB_SWAP_L (REG_L | 0x30)
#define OP_CB_SWAP_A (REG_A | 0x30)

// srl r
#define OP_CB_SRL_B (REG_B | 0x38)
#define OP_CB_SRL_C (REG_C | 0x38)
#define OP_CB_SRL_D (REG_D | 0x38)
#define OP_CB_SRL_E (REG_E | 0x38)
#define OP_CB_SRL_H (REG_H | 0x38)
#define OP_CB_SRL_L (REG_L | 0x38)
#define OP_CB_SRL_A (REG_A | 0x38)

//------------------------------Single Bit Operation Commands-------------------------
// bit n,r
#define OP_CB_BIT_0_B (REG_B | 0x40)
#define OP_CB_BIT_1_B (REG_B | 0x48)
#define OP_CB_BIT_2_B (REG_B | 0x50)
#define OP_CB_BIT_3_B (REG_B | 0x58)
#define OP_CB_BIT_4_B (REG_B | 0x60)
#define OP_CB_BIT_5_B (REG_B | 0x68)
#define OP_CB_BIT_6_B (REG_B | 0x70)
#define OP_CB_BIT_7_B (REG_B | 0x78)
#define OP_CB_BIT_0_C (REG_C | 0x40)
#define OP_CB_BIT_1_C (REG_C | 0x48)
#define OP_CB_BIT_2_C (REG_C | 0x50)
#define OP_CB_BIT_3_C (REG_C | 0x58)
#define OP_CB_BIT_4_C (REG_C | 0x60)
#define OP_CB_BIT_5_C (REG_C | 0x68)
#define OP_CB_BIT_6_C (REG_C | 0x70)
#define OP_CB_BIT_7_C (REG_C | 0x78)
#define OP_CB_BIT_0_D (REG_D | 0x40)
#define OP_CB_BIT_1_D (REG_D | 0x48)
#define OP_CB_BIT_2_D (REG_D | 0x50)
#define OP_CB_BIT_3_D (REG_D | 0x58)
#define OP_CB_BIT_4_D (REG_D | 0x60)
#define OP_CB_BIT_5_D (REG_D | 0x68)
#define OP_CB_BIT_6_D (REG_D | 0x70)
#define OP_CB_BIT_7_D (REG_D | 0x78)
#define OP_CB_BIT_0_E (REG_E | 0x40)
#define OP_CB_BIT_1_E (REG_E | 0x48)
#define OP_CB_BIT_2_E (REG_E | 0x50)
#define OP_CB_BIT_3_E (REG_E | 0x58)
#define OP_CB_BIT_4_E (REG_E | 0x60)
#define OP_CB_BIT_5_E (REG_E | 0x68)
#define OP_CB_BIT_6_E (REG_E | 0x70)
#define OP_CB_BIT_7_E (REG_E | 0x78)
#define OP_CB_BIT_0_H (REG_H | 0x40)
#define OP_CB_BIT_1_H (REG_H | 0x48)
#define OP_CB_BIT_2_H (REG_H | 0x50)
#define OP_CB_BIT_3_H (REG_H | 0x58)
#define OP_CB_BIT_4_H (REG_H | 0x60)
#define OP_CB_BIT_5_H (REG_H | 0x68)
#define OP_CB_BIT_6_H (REG_H | 0x70)
#define OP_CB_BIT_7_H (REG_H | 0x78)
#define OP_CB_BIT_0_L (REG_L | 0x40)
#define OP_CB_BIT_1_L (REG_L | 0x48)
#define OP_CB_BIT_2_L (REG_L | 0x50)
#define OP_CB_BIT_3_L (REG_L | 0x58)
#define OP_CB_BIT_4_L (REG_L | 0x60)
#define OP_CB_BIT_5_L (REG_L | 0x68)
#define OP_CB_BIT_6_L (REG_L | 0x70)
#define OP_CB_BIT_7_L (REG_L | 0x78)
#define OP_CB_BIT_0_A (REG_A | 0x40)
#define OP_CB_BIT_1_A (REG_A | 0x48)
#define OP_CB_BIT_2_A (REG_A | 0x50)
#define OP_CB_BIT_3_A (REG_A | 0x58)
#define OP_CB_BIT_4_A (REG_A | 0x60)
#define OP_CB_BIT_5_A (REG_A | 0x68)
#define OP_CB_BIT_6_A (REG_A | 0x70)
#define OP_CB_BIT_7_A (REG_A | 0x78)

// bit n,r
#define OP_CB_RES_0_B (REG_B | (0x40 + 0x40))
#define OP_CB_RES_1_B (REG_B | (0x48 + 0x40))
#define OP_CB_RES_2_B (REG_B | (0x50 + 0x40))
#define OP_CB_RES_3_B (REG_B | (0x58 + 0x40))
#define OP_CB_RES_4_B (REG_B | (0x60 + 0x40))
#define OP_CB_RES_5_B (REG_B | (0x68 + 0x40))
#define OP_CB_RES_6_B (REG_B | (0x70 + 0x40))
#define OP_CB_RES_7_B (REG_B | (0x78 + 0x40))
#define OP_CB_RES_0_C (REG_C | (0x40 + 0x40))
#define OP_CB_RES_1_C (REG_C | (0x48 + 0x40))
#define OP_CB_RES_2_C (REG_C | (0x50 + 0x40))
#define OP_CB_RES_3_C (REG_C | (0x58 + 0x40))
#define OP_CB_RES_4_C (REG_C | (0x60 + 0x40))
#define OP_CB_RES_5_C (REG_C | (0x68 + 0x40))
#define OP_CB_RES_6_C (REG_C | (0x70 + 0x40))
#define OP_CB_RES_7_C (REG_C | (0x78 + 0x40))
#define OP_CB_RES_0_D (REG_D | (0x40 + 0x40))
#define OP_CB_RES_1_D (REG_D | (0x48 + 0x40))
#define OP_CB_RES_2_D (REG_D | (0x50 + 0x40))
#define OP_CB_RES_3_D (REG_D | (0x58 + 0x40))
#define OP_CB_RES_4_D (REG_D | (0x60 + 0x40))
#define OP_CB_RES_5_D (REG_D | (0x68 + 0x40))
#define OP_CB_RES_6_D (REG_D | (0x70 + 0x40))
#define OP_CB_RES_7_D (REG_D | (0x78 + 0x40))
#define OP_CB_RES_0_E (REG_E | (0x40 + 0x40))
#define OP_CB_RES_1_E (REG_E | (0x48 + 0x40))
#define OP_CB_RES_2_E (REG_E | (0x50 + 0x40))
#define OP_CB_RES_3_E (REG_E | (0x58 + 0x40))
#define OP_CB_RES_4_E (REG_E | (0x60 + 0x40))
#define OP_CB_RES_5_E (REG_E | (0x68 + 0x40))
#define OP_CB_RES_6_E (REG_E | (0x70 + 0x40))
#define OP_CB_RES_7_E (REG_E | (0x78 + 0x40))
#define OP_CB_RES_0_H (REG_H | (0x40 + 0x40))
#define OP_CB_RES_1_H (REG_H | (0x48 + 0x40))
#define OP_CB_RES_2_H (REG_H | (0x50 + 0x40))
#define OP_CB_RES_3_H (REG_H | (0x58 + 0x40))
#define OP_CB_RES_4_H (REG_H | (0x60 + 0x40))
#define OP_CB_RES_5_H (REG_H | (0x68 + 0x40))
#define OP_CB_RES_6_H (REG_H | (0x70 + 0x40))
#define OP_CB_RES_7_H (REG_H | (0x78 + 0x40))
#define OP_CB_RES_0_L (REG_L | (0x40 + 0x40))
#define OP_CB_RES_1_L (REG_L | (0x48 + 0x40))
#define OP_CB_RES_2_L (REG_L | (0x50 + 0x40))
#define OP_CB_RES_3_L (REG_L | (0x58 + 0x40))
#define OP_CB_RES_4_L (REG_L | (0x60 + 0x40))
#define OP_CB_RES_5_L (REG_L | (0x68 + 0x40))
#define OP_CB_RES_6_L (REG_L | (0x70 + 0x40))
#define OP_CB_RES_7_L (REG_L | (0x78 + 0x40))
#define OP_CB_RES_0_A (REG_A | (0x40 + 0x40))
#define OP_CB_RES_1_A (REG_A | (0x48 + 0x40))
#define OP_CB_RES_2_A (REG_A | (0x50 + 0x40))
#define OP_CB_RES_3_A (REG_A | (0x58 + 0x40))
#define OP_CB_RES_4_A (REG_A | (0x60 + 0x40))
#define OP_CB_RES_5_A (REG_A | (0x68 + 0x40))
#define OP_CB_RES_6_A (REG_A | (0x70 + 0x40))
#define OP_CB_RES_7_A (REG_A | (0x78 + 0x40))

//--------------------------------CPU Control Commands--------------------------------
#define OP_CCF 0x3F
#define OP_SCF 0x37
#define OP_HALT 0x76
#define OP_STOP 0x10
#define OP_DI 0xF3
#define OP_EI 0xFB

// Jump Commands
// jp nn
#define OP_JP_NN 0xC3

// jp HL
#define OP_JP_HL 0xE9

// jp f,nn
#define OP_JP_NZ_nn (FLAG_NZ << 3 | 0xC2)
#define OP_JP_Z_nn (FLAG_Z << 3 | 0xC2)
#define OP_JP_NC_nn (FLAG_NC << 3 | 0xC2)
#define OP_JP_C_nn (FLAG_C << 3 | 0xC2)

// jr f,PC+dd
#define OP_JR_NZ_dd (FLAG_NZ << 3 | 0x20)
#define OP_JR_Z_dd (FLAG_Z << 3 | 0x20)
#define OP_JR_NC_dd (FLAG_NC << 3 | 0x20)
#define OP_JR_C_dd (FLAG_C << 3 | 0x20)

// jr PC+dd
#define OP_JR_dd 0x18

// call nn
#define OP_CALL_nn 0xCD

// call f,nn
#define OP_CALL_NZ_nn (FLAG_NZ << 3 | 0xC4)
#define OP_CALL_Z_nn (FLAG_Z << 3 | 0xC4)
#define OP_CALL_NC_nn (FLAG_NC << 3 | 0xC4)
#define OP_CALL_C_nn (FLAG_C << 3 | 0xC4)

// ret
#define OP_RET 0xC9

// ret f
#define OP_RET_NZ_nn (FLAG_NZ << 3 | 0xC0)
#define OP_RET_Z_nn (FLAG_Z << 3 | 0xC0)
#define OP_RET_NC_nn (FLAG_NC << 3 | 0xC0)
#define OP_RET_C_nn (FLAG_C << 3 | 0xC0)

// reti
#define OP_RETI 0xD9

// rst n
#define OP_RST_00 (0 << 3 | 0xC7)
#define OP_RST_08 (1 << 3 | 0xC7)
#define OP_RST_10 (2 << 3 | 0xC7)
#define OP_RST_18 (3 << 3 | 0xC7)
#define OP_RST_20 (4 << 3 | 0xC7)
#define OP_RST_28 (5 << 3 | 0xC7)
#define OP_RST_30 (6 << 3 | 0xC7)
#define OP_RST_38 (7 << 3 | 0xC7)