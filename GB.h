// GB.h - Main structure for the GameBoy

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <memory.h>
#include <SDL2/SDL.h>

#include "GBOpcodes.h"

// #define _DBG_INSTR_

#define GB_VID_WIDTH 160
#define GB_VID_HEIGHT 144

#define RENDER_SCALE 4

#define CART_ENTRYPOINT 0x100
#define CART_LOGO 0x104
#define CART_LOGO_END 0x133
#define CART_TITLE 0x134
#define CART_TITLE_END 0x143
#define CART_CART_TYPE 0x147
#define CART_ROMSIZE 0x148
#define CART_RAMSIZE 0x149
#define CART_DEST_CODE 0x14A
#define CART_OLD_LICENSEE_CODE 0x14B
#define CART_MASK_ROM_VERSION 0x14C
#define CART_HEADER_CHECKSUM 0x14D
#define CART_GLOBAL_CHECKSUM 0x14E
#define CART_GLOBAL_CHECKSUM_END 0x14F

// 0xFF40 - LCD Control Register
// Bit 7 - LCD Power           (0=Off, 1=On)
// Bit 6 - Window Tile Map     (0=9800h-9BFFh, 1=9C00h-9FFFh)
// Bit 5 - Window Enable       (0=Disabled, 1=Enabled)
// Bit 4 - BG & Window Tileset (0=8800h-97FFh, 1=8000h-8FFFh)
// Bit 3 - BG Tile Map         (0=9800h-9BFFh, 1=9C00h-9FFFh)
// Bit 2 - Sprite Size         (0=8×8, 1=8×16)
// Bit 1 - Sprites Enabled     (0=Disabled, 1=Enabled)
// Bit 0 - BG Enabled (in DMG) (0=Disabled, 1=Enabled)
typedef struct RenderContextstruct
{
    SDL_Window *window;
    SDL_Renderer *renderer;

    SDL_Texture *backbufferTexture;

    uint32_t *pixels;
    int pitch;
} RenderContext;

// CPU Opcode information (Found on Page 65)
// Memory Info (Found on Page 8)
// RAM: 8K = uint8_t[0x8000]

typedef struct GBstruct
{
    RenderContext *ctx;

    // Info From http://problemkaputt.de/pandocs.htm#cpuregistersandflags

    // BC[0], DE[1], HL[2], SP[3], PC[4], padding[5,6], AF[7]
    // Stored as an array for easier instruction handling
    uint16_t regs[8];

    // Flags
    // Bit 7 - Zero
    // Bit 6 - Add/Sub-Flag (BCD)
    // Bit 5 - Half Carry Flag (BCD)
    // Bit 4 - Carry
    // Bit 3->0 - Unused (always zero)

    // Main Memory
    uint8_t mem[0x8000];

    // Cartridge Memory
    uint8_t *cart;
    uint32_t cartSize;
} GB;

void DestroyGBRenderContext(RenderContext *ctx)
{
    if (ctx != NULL)
    {
        printf("Destroying Rendering Context\n");
        if (ctx->renderer != NULL)
        {
            SDL_DestroyRenderer(ctx->renderer);
            ctx->renderer = NULL;
            ctx->pixels = NULL;
        }
        if (ctx->window != NULL)
        {
            SDL_DestroyWindow(ctx->window);
            ctx->window = NULL;
        }
        if (ctx->pixels != NULL)
        {
            free(ctx->pixels);
            ctx->pixels = NULL;
        }

        free(ctx);
        SDL_Quit();
    }
}

void DestroyGB(GB *gb)
{
    printf("Destroying GB\n");
    if (gb != NULL)
    {
        DestroyGBRenderContext(gb->ctx);
        gb->ctx = NULL;

        if (gb->cart != NULL)
        {
            free(gb->cart);
        }

        free(gb);
    }
}

RenderContext *CreateRenderContext()
{
    RenderContext *ctx = malloc(sizeof(RenderContext));

    ctx->window = SDL_CreateWindow(
        "pc_gb",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        GB_VID_WIDTH * RENDER_SCALE,
        GB_VID_HEIGHT * RENDER_SCALE,
        SDL_WINDOW_SHOWN);

    if (ctx->window == NULL)
    {
        DestroyGBRenderContext(ctx);
        return NULL;
    }

    ctx->renderer = SDL_CreateRenderer(ctx->window, -1, SDL_RENDERER_ACCELERATED);
    if (ctx->renderer == NULL)
    {
        DestroyGBRenderContext(ctx);
        return NULL;
    }

    ctx->backbufferTexture = SDL_CreateTexture(
        ctx->renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        GB_VID_WIDTH * RENDER_SCALE,
        GB_VID_HEIGHT * RENDER_SCALE);
    if (ctx->backbufferTexture == NULL)
    {
        DestroyGBRenderContext(ctx);
        return NULL;
    }

    ctx->pixels = malloc(sizeof(uint32_t) * GB_VID_WIDTH * GB_VID_HEIGHT * RENDER_SCALE * RENDER_SCALE);
    if (ctx->pixels == NULL)
    {
        DestroyGBRenderContext(ctx);
        return NULL;
    }
    ctx->pitch = sizeof(uint32_t) * GB_VID_WIDTH * RENDER_SCALE;

    printf("Created Rendering Context\n");
    return ctx;
}

GB *CreateGB(const char *rom)
{
    RenderContext *ctx = CreateRenderContext();
    if (ctx == NULL)
    {
        printf("Failed to create Rendering Context\n");
        return NULL;
    }

    GB *gb = malloc(sizeof(GB));
    gb->ctx = ctx;

    return gb;
}

uint8_t ReadMem(GB *gb, uint16_t addr)
{
    if (addr >= 0x8000)
    {
        return gb->mem[addr - 0x8000];
    }
    else
    {
        return gb->cart[addr];
    }

    return 0;
}

void WriteMem(GB *gb, uint16_t addr, uint8_t val)
{
    if (addr >= 0x8000)
    {
        gb->mem[addr - 0x8000] = val;
    }
}

uint8_t *LoadRom(const char *filename, uint32_t *romSize)
{
    FILE *f = fopen(filename, "rb");
    if (f == NULL)
    {
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    (*romSize) = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t *rom = malloc(sizeof(uint8_t) * (*romSize));

    fread(rom, (*romSize), 1, f);

    fclose(f);

    return rom;
}

void DumpCPURegisters(GB *gb)
{
    printf("CPU Registers\n");
    printf("\tA: 0x%01x\n", gb->regs[REG_AF]);
    printf("\tBC: 0x%02x\n", gb->regs[REG_BC]);
    printf("\tDE: 0x%02x\n", gb->regs[REG_DE]);
    printf("\tHL: 0x%02x\n", gb->regs[REG_HL]);
    printf("\tSP: 0x%02x\n", gb->regs[REG_SP]);
    printf("PC: 0x%02x\n", gb->regs[REG_PC] - 1);
}

void DumpRomInfo(GB *gb)
{
    printf("Rom Info\n");
    printf("\tTitle: %s\n", &gb->cart[CART_TITLE]);
    printf("\tCart Type: 0x%01X\n", gb->cart[CART_CART_TYPE]);
    printf("\tROM Size: 0x%01X\n", gb->cart[CART_ROMSIZE]);
    printf("\tRAM Size: 0x%01X\n", gb->cart[CART_RAMSIZE]);
}

void SimpleRender(GB *gb, RenderContext *ctx)
{
    SDL_LockTexture(
        ctx->backbufferTexture,
        NULL,
        (void **)(&ctx->pixels),
        &ctx->pitch);

    // Get Color shades
    uint32_t colors[4] = {
        0xFFFFFFFF,
        0x7E7E7EFF,
        0x3F3F3FFF,
        0xFF,
    };

    uint32_t color0 = colors[(ReadMem(gb, 0xFF47)) & 0b11];
    uint32_t color1 = colors[(ReadMem(gb, 0xFF47) >> 2) & 0b11];
    uint32_t color2 = colors[(ReadMem(gb, 0xFF47) >> 4) & 0b11];
    uint32_t color3 = colors[(ReadMem(gb, 0xFF47) >> 6) & 0b11];

    // Render Each Tile
    uint8_t lcdControl = ReadMem(gb, 0xFF40);

    uint16_t tileBase = (lcdControl & 0x10) > 0 ? 0x8000 : 0x8800;
    uint16_t bgBase = (lcdControl & 0x8) > 0 ? 0x9C00 : 0x9800;
    for (int i = 0; i < 32; ++i)
    {
        for (int j = 0; j < 32; ++j)
        {
            // TODO: use LCD register to get tilemap location
            // Get each row of the tile
            for (int ty = 0; ty < 8; ++ty)
            {
                uint16_t tileIndex = ReadMem(gb, bgBase + i + j * 32);

                // tileBase + tileIndex + tileY
                uint8_t row1 = ReadMem(gb, tileBase + tileIndex * 16 + (ty * 2));
                uint8_t row2 = ReadMem(gb, tileBase + tileIndex * 16 + (ty * 2) + 1);

                for (int tx = 0; tx < 8; ++tx)
                {
                    uint8_t color = (row1 >> (7 - tx)) & 1;
                    color |= ((row2 >> (7 - tx)) & 1) << 1;

                    int px = (i * 8 + tx) * RENDER_SCALE;
                    int py = (j * 8 + ty) * RENDER_SCALE;

                    if (px >= GB_VID_WIDTH * RENDER_SCALE || py >= GB_VID_HEIGHT * RENDER_SCALE)
                        continue;

                    // 5*3 = 15
                    // (5*3)*2 = 15*2 = 30
                    // (5*2) * (3*2) = 10 * 6 = 60
                    // (5*3)*2*2 = (5*3)*4 = 15 * 4 = 60

                    for (int scalex = 0; scalex < RENDER_SCALE; ++scalex)
                    {
                        for (int scaley = 0; scaley < RENDER_SCALE; ++scaley)
                        {
                            if (color == 0)
                            {
                                ctx->pixels[px + scalex + (py + scaley) * GB_VID_WIDTH * RENDER_SCALE] = color0;
                            }
                            else if (color == 1)
                            {
                                ctx->pixels[px + scalex + (py + scaley) * GB_VID_WIDTH * RENDER_SCALE] = color1;
                            }
                            else if (color == 2)
                            {
                                ctx->pixels[px + scalex + (py + scaley) * GB_VID_WIDTH * RENDER_SCALE] = color2;
                            }
                            else if (color == 3)
                            {
                                ctx->pixels[px + scalex + (py + scaley) * GB_VID_WIDTH * RENDER_SCALE] = color3;
                            }
                        }
                    }
                }
            }
        }
    }

    SDL_UnlockTexture(ctx->backbufferTexture);

    SDL_RenderCopy(ctx->renderer, ctx->backbufferTexture, NULL, NULL);
    SDL_RenderPresent(ctx->renderer);
}

uint8_t FetchByte(GB *gb)
{
    gb->regs[REG_PC] += 1;
    return ReadMem(gb, gb->regs[REG_PC] - 1);
}

uint16_t FetchWord(GB *gb)
{
    return FetchByte(gb) | (FetchByte(gb) << 8);
}

void Set8Reg(GB *gb, uint8_t reg, uint8_t val)
{
    size_t index = reg == REG_A ? 7 : reg / 2;
    uint16_t bigVal = val;
    uint16_t mask = 0xFF00;
    if (reg % 2 == 0 || reg == REG_A)
    {
        bigVal <<= 8;
        mask >>= 8;
    }

    gb->regs[index] &= mask;
    gb->regs[index] |= bigVal;
}

uint8_t Get8Reg(GB *gb, uint8_t reg)
{
    size_t index = reg == REG_A ? 7 : reg / 2;
    uint16_t val = gb->regs[index];
    if ((reg % 2) == 0 || reg == REG_A)
    {
        val >>= 8;
    }

    return val & 0xFF;
}

char *GetReg8Name(uint8_t reg)
{
    switch (reg)
    {
    case REG_B:
        return "B";

    case REG_C:
        return "C";

    case REG_D:
        return "D";

    case REG_E:
        return "E";

    case REG_H:
        return "H";

    case REG_L:
        return "L";

    case REG_A:
        return "A";
    default:
        return "??";
    }
}

char *GetRegName(uint8_t reg)
{
    switch (reg)
    {
    case REG_BC:
        return "BC";

    case REG_DE:
        return "DE";

    case REG_HL:
        return "HL";

    case REG_SP:
        return "SP";

    case REG_AF:
        return "A(F)";

    default:
        return "??";
    }
}

char *GetFlagName(uint8_t flag)
{
    switch (flag)
    {
    case FLAG_NZ:
        return "NZ";

    case FLAG_Z:
        return "Z";

    case FLAG_NC:
        return "NC";

    case FLAG_C:
        return "C";

    default:
        return "??";
    }
}

void SetFlag(GB *gb, uint8_t flag, uint8_t val)
{
    uint8_t mask = 1;

    switch (flag)
    {
    case FLAG_Z:
    {
        mask <<= 7;
    }
    break;

    case FLAG_C:
    {
        mask <<= 4;
    }
    break;

    case FLAG_N:
    {
        mask <<= 6;
    }
    break;

    case FLAG_H:
    {
        mask <<= 5;
    }
    break;

    default:
        printf("Trying to set invalid flag. This shouldn't be called!\n");
        return;
    }

    if (val > 0)
    {
        gb->regs[REG_AF] |= mask;
    }
    else
    {
        gb->regs[REG_AF] &= ~mask;
    }
}

bool CheckFlag(GB *gb, uint8_t flag)
{
    switch (flag)
    {
    case FLAG_NZ:
    {
        return (gb->regs[REG_AF] & 0x80) == 0;
    }
    break;

    case FLAG_Z:
    {
        return (gb->regs[REG_AF] & 0x80) > 0;
    }
    break;

    case FLAG_NC:
    {
        return (gb->regs[REG_AF] & 0x8) == 0;
    }
    break;

    case FLAG_C:
    {
        return (gb->regs[REG_AF] & 0x8) >= 0;
    }
    break;

    default:
        printf("Checking flag that doesn't not exist. This should not have been called!\n");
        return false;
    }
}

bool DoInstruction(GB *gb)
{
    // Fetch opcode
    uint8_t opcode = FetchByte(gb);

    switch (opcode)
    {
    case OP_NOP:
    {
    }
    break;

    // 8bit Load Commands
    case OP_LD_B_B:
    case OP_LD_B_C:
    case OP_LD_B_D:
    case OP_LD_B_E:
    case OP_LD_B_H:
    case OP_LD_B_L:
    case OP_LD_B_A:
    case OP_LD_C_B:
    case OP_LD_C_C:
    case OP_LD_C_D:
    case OP_LD_C_E:
    case OP_LD_C_H:
    case OP_LD_C_L:
    case OP_LD_C_A:
    case OP_LD_D_B:
    case OP_LD_D_C:
    case OP_LD_D_D:
    case OP_LD_D_E:
    case OP_LD_D_H:
    case OP_LD_D_L:
    case OP_LD_D_A:
    case OP_LD_E_B:
    case OP_LD_E_C:
    case OP_LD_E_D:
    case OP_LD_E_E:
    case OP_LD_E_H:
    case OP_LD_E_L:
    case OP_LD_E_A:
    case OP_LD_H_B:
    case OP_LD_H_C:
    case OP_LD_H_D:
    case OP_LD_H_E:
    case OP_LD_H_H:
    case OP_LD_H_L:
    case OP_LD_H_A:
    case OP_LD_L_B:
    case OP_LD_L_C:
    case OP_LD_L_D:
    case OP_LD_L_E:
    case OP_LD_L_H:
    case OP_LD_L_L:
    case OP_LD_L_A:
    case OP_LD_A_B:
    case OP_LD_A_C:
    case OP_LD_A_D:
    case OP_LD_A_E:
    case OP_LD_A_H:
    case OP_LD_A_L:
    case OP_LD_A_A:
    {

        uint8_t regDst = (opcode >> 3) & 0b111;
        uint8_t regSrc = opcode & 0b111;

        Set8Reg(gb, regDst, Get8Reg(gb, regSrc));

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, LD %s,%s\n", gb->regs[REG_PC] - 1, GetReg8Name(regDst), GetReg8Name(regSrc));
#endif
    }
    break;

    case OP_LD_B_n:
    case OP_LD_C_n:
    case OP_LD_D_n:
    case OP_LD_E_n:
    case OP_LD_H_n:
    case OP_LD_L_n:
    case OP_LD_A_n:
    {

        uint8_t regDst = (opcode >> 3) & 0b111;
        uint8_t val = FetchByte(gb);

        Set8Reg(gb, regDst, val);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, LD %s,$%01X\n", gb->regs[REG_PC] - 1, GetReg8Name(regDst), val);
#endif
    }
    break;

    case OP_LD_ptrDE_A:
    {
#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, LD ($%02X),A - (DE)\n", gb->regs[REG_PC] - 1, gb->regs[REG_DE]);
#endif

        WriteMem(gb, gb->regs[REG_DE], Get8Reg(gb, REG_A));
    }
    break;

    case OP_LD_A_IOn:
    {
        // TODO: read from io port
        uint8_t offset = FetchByte(gb);
        if (offset < 0x7F)
            Set8Reg(gb, REG_A, ReadMem(gb, 0xFF00 + offset));

#ifndef _DBG_INSTR_
            // printf("PC: 0x%02X, LD A, (FF00+$%01X) - A is now 0x%01X!\n", gb->regs[REG_PC] - 1, offset, Get8Reg(gb, REG_A));
#endif
    }
    break;

    case OP_LD_IOn_A:
    {
        // TODO: write to io port
        uint8_t offset = FetchByte(gb);
        if (offset < 0x7F)
            WriteMem(gb, 0xFF00 + offset, Get8Reg(gb, REG_A));
#ifndef _DBG_INSTR_
        printf("PC: 0x%02X, LD (FF00+$%01X),A - now $%01X!\n", gb->regs[REG_PC] - 1, offset, ReadMem(gb, 0xFF00 + offset));
#endif
    }
    break;

    case OP_LD_IOC_A:
    {
        // TODO: write to io port
        uint8_t offset = Get8Reg(gb, REG_C);
        if (offset < 0x7F)
            WriteMem(gb, 0xFF00 + offset, Get8Reg(gb, REG_A));
#ifndef _DBG_INSTR_
        printf("PC: 0x%02X, LD (FF00+$%01X),A - now $%01X!\n", gb->regs[REG_PC] - 1, offset, ReadMem(gb, 0xFF00 + offset));
#endif
    }
    break;

    case OP_LDI_A_ptrHL:
    {

        uint8_t val = ReadMem(gb, gb->regs[REG_HL]);
        gb->regs[REG_HL] += 1;

        Set8Reg(gb, REG_A, val);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, LDI A,($%02X) - (HL)\n", gb->regs[REG_PC] - 1, gb->regs[REG_HL]);
#endif
    }
    break;

    case OP_LDI_ptrHL_A:
    {
        uint8_t val = Get8Reg(gb, REG_A);
        WriteMem(gb, gb->regs[REG_HL], val);
        gb->regs[REG_HL] -= 1;

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, LDD ($%02X), A - (HL)\n", gb->regs[REG_PC] - 1, gb->regs[REG_HL]);
#endif
    }
    break;

    case OP_LD_ptrHL_n:
    {
        uint8_t val = FetchByte(gb);
        WriteMem(gb, gb->regs[REG_HL], val);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, LD ($%02X), $%01X - (HL)\n", gb->regs[REG_PC] - 1, gb->regs[REG_HL], val);
#endif
    }
    break;

    case OP_LD_ptrnn_A:
    {
        uint16_t addr = FetchWord(gb);

        WriteMem(gb, addr, Get8Reg(gb, REG_A));

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, LD ($%02X),A\n", gb->regs[REG_PC] - 1, addr);
#endif
    }
    break;

    // 8bit Arthmetic/logical Commands
    case OP_AND_B:
    case OP_AND_C:
    case OP_AND_D:
    case OP_AND_E:
    case OP_AND_H:
    case OP_AND_L:
    case OP_AND_A:
    {
        uint8_t reg = opcode & 0b111;
        uint8_t val = Get8Reg(gb, REG_A) & Get8Reg(gb, reg);
        Set8Reg(gb, REG_A, val);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, AND %s\n", gb->regs[REG_PC] - 1, Get8Reg(gb, reg));
#endif
    }
    break;

    case OP_XOR_B:
    case OP_XOR_C:
    case OP_XOR_D:
    case OP_XOR_E:
    case OP_XOR_H:
    case OP_XOR_L:
    case OP_XOR_A:
    {
        uint8_t reg = opcode & 0b111;
        uint8_t val = Get8Reg(gb, REG_A) ^ Get8Reg(gb, reg);
        Set8Reg(gb, REG_A, val);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, XOR %s\n", gb->regs[REG_PC] - 1, GetReg8Name(reg));
#endif
    }
    break;

    case OP_OR_B:
    case OP_OR_C:
    case OP_OR_D:
    case OP_OR_E:
    case OP_OR_H:
    case OP_OR_L:
    case OP_OR_A:
    {
        uint8_t reg = opcode & 0b111;
        uint8_t val = Get8Reg(gb, REG_A) | Get8Reg(gb, reg);
        Set8Reg(gb, REG_A, val);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, OR %s\n", gb->regs[REG_PC] - 1, GetReg8Name(reg));
#endif
    }
    break;

    case OP_CP_n:
    {
        uint8_t val = FetchByte(gb);
        uint8_t newVal = Get8Reg(gb, REG_A) - val;

        SetFlag(gb, FLAG_Z, newVal == 0);
        SetFlag(gb, FLAG_C, newVal > Get8Reg(gb, REG_A));
        SetFlag(gb, FLAG_N, 1);
        SetFlag(gb, FLAG_H, (newVal & 0xF) < (Get8Reg(gb, REG_A) & 0xF));

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, cp $%01X\n", gb->regs[REG_PC] - 1, val);
#endif
    }
    break;

    case OP_INC_B:
    case OP_INC_C:
    case OP_INC_D:
    case OP_INC_E:
    case OP_INC_H:
    case OP_INC_L:
    case OP_INC_A:
    {

        uint8_t reg = (opcode >> 3) & 0b111;
        uint8_t val = Get8Reg(gb, reg) + 1;

        SetFlag(gb, FLAG_Z, val == 0);
        SetFlag(gb, FLAG_N, 0);
        SetFlag(gb, FLAG_H, (val & 0xF) < (Get8Reg(gb, reg) & 0xF));

        Set8Reg(gb, reg, val);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, INC %s\n", gb->regs[REG_PC] - 1, GetReg8Name(reg));
#endif
    }
    break;

    case OP_DEC_B:
    case OP_DEC_C:
    case OP_DEC_D:
    case OP_DEC_E:
    case OP_DEC_H:
    case OP_DEC_L:
    case OP_DEC_A:
    {

        uint8_t reg = (opcode >> 3) & 0b111;
        uint8_t val = Get8Reg(gb, reg) - 1;

        Set8Reg(gb, reg, val);

        SetFlag(gb, FLAG_Z, val == 0);
        SetFlag(gb, FLAG_N, 1);
        SetFlag(gb, FLAG_H, (val & 0xF) > (Get8Reg(gb, reg) & 0xF));

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, DEC %s\n", gb->regs[REG_PC] - 1, GetReg8Name(reg));
#endif
    }
    break;

    // 16bit Load Commands
    case OP_LD_BC_NN:
    case OP_LD_DE_NN:
    case OP_LD_HL_NN:
    case OP_LD_SP_NN:
    {

        uint8_t reg = (opcode & 0xF0) >> 4;
        uint16_t val = FetchWord(gb);

        gb->regs[reg] = val;

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, LD %s,$%02X\n", gb->regs[REG_PC] - 1, GetRegName(reg), val);
#endif
    }
    break;

    case OP_DEC_BC:
    case OP_DEC_DE:
    case OP_DEC_HL:
    case OP_DEC_SP:
    {
        uint8_t reg = (opcode >> 4);
        uint8_t val = Get8Reg(gb, reg) - 1;

        Set8Reg(gb, reg, val);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, DEC %s\n", gb->regs[REG_PC] - 1, GetRegName(reg));
#endif
    }
    break;

    // CPU Control Commands
    case OP_DI:
    {
        // TODO: disable interrupts
#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, DI - NOT IMPLEMENTED!\n", gb->regs[REG_PC] - 1);
#endif
    }
    break;

    // Jump Commands
    case OP_JP_NN:
    {
        uint16_t addr = FetchWord(gb);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, JP $%02X\n", gb->regs[REG_PC] - 1, addr);
#endif

        gb->regs[REG_PC] = addr;
    }
    break;

    case OP_JR_NZ_dd:
    case OP_JR_Z_dd:
    case OP_JR_NC_dd:
    case OP_JR_C_dd:
    {

        uint8_t offset = FetchByte(gb);
        uint8_t flag = (opcode >> 3) & 0b11;

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, JR %s,PC+$%01X\n", gb->regs[REG_PC] - 1, GetFlagName(flag), (uint8_t)offset);
#endif
        if (CheckFlag(gb, flag))
        {
            if ((offset & 0x80) > 0)
            {
                offset = ~offset;
                offset += 1;

                gb->regs[REG_PC] -= offset;
            }
            else
            {
                gb->regs[REG_PC] += offset;
            }
        }
        else
        {
            // printf("didn't jump\n");
        }
    }
    break;

    case OP_JR_dd:
    {
        uint8_t offset = FetchByte(gb);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, JR PC+$%01X\n", gb->regs[REG_PC] - 1, (uint8_t)offset);
#endif
        if ((offset & 0x80) > 0)
        {
            offset = ~offset;
            offset += 1;

            gb->regs[REG_PC] -= offset;
        }
        else
        {
            gb->regs[REG_PC] += offset;
        }
    }
    break;

    case OP_CALL_nn:
    {
        uint16_t addr = FetchWord(gb);
        gb->regs[REG_SP] -= 2;
        WriteMem(gb, gb->regs[REG_SP], gb->regs[REG_PC]);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, CALL $%02X\n", gb->regs[REG_PC] - 1, addr);
#endif
        gb->regs[REG_PC] = addr;
    }
    break;

    case OP_RET:
    {
#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, RET\n", gb->regs[REG_PC] - 1);
#endif

        gb->regs[REG_PC] = ReadMem(gb, gb->regs[REG_SP]);
        gb->regs[REG_SP] += 2;
    }
    break;

    default:
    {
        DumpCPURegisters(gb);
        printf("Unknown instruction: 0x%01X\n", opcode);
        return false;
    }
    }

    return true;
}

void StartGB(GB *gb)
{
    if (gb == NULL)
    {
        return;
    }

    printf("GB Starting...\n");

    // gb->cart = LoadRom("../tests/carts/cpu_instrs/individual/06-ld r,r.gb", &gb->cartSize);
    gb->cart = LoadRom("../tests/carts/Tetris (JUE) (V1.1) [!].gb", &gb->cartSize);
    if (gb->cart == NULL)
    {
        return;
    }

    DumpRomInfo(gb);

    // Info from https://realboyemulator.files.wordpress.com/2013/01/gbcpuman.pdf
    // Power Up Sequence (Found on Page 18)
    // BC = $0013
    // DE = $00D8
    // HL = $014D
    // Stack Pointer = $FFFE

    gb->regs[REG_BC] = 0x0013;
    gb->regs[REG_DE] = 0x00D8;
    gb->regs[REG_HL] = 0x014D;
    gb->regs[REG_SP] = 0xFFFE;
    gb->regs[REG_AF] = 0x0000;

    // TODO: Run boot up procedure instead of jumping to cartridge immediately
    gb->regs[REG_PC] = 0x0100;

    memset(gb->mem, 0xEA, sizeof(uint8_t) * 0x8000);

    gb->mem[0x1C00] = 0;

    gb->mem[0] = 0x7C;
    gb->mem[1] = 0x7C;
    gb->mem[2] = 0x00;
    gb->mem[3] = 0xC6;
    gb->mem[4] = 0xC6;
    gb->mem[5] = 0x00;
    gb->mem[6] = 0x00;
    gb->mem[7] = 0xFE;
    gb->mem[8] = 0xC6;
    gb->mem[9] = 0xC6;
    gb->mem[0xa] = 0x00;
    gb->mem[0xb] = 0xC6;
    gb->mem[0xc] = 0xC6;
    gb->mem[0xd] = 0x00;
    gb->mem[0xe] = 0x00;
    gb->mem[0xf] = 0x00;

    SimpleRender(gb, gb->ctx);

    uint64_t count = 0;
    uint8_t LY = 0;

    bool running = true;
    while (running)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                running = false;
            }
        }

        WriteMem(gb, 0xFF44, LY);

        if (!DoInstruction(gb))
        {
            running = false;
        }

        if ((count % 1024 * 2) == 0)
        {
            SimpleRender(gb, gb->ctx);
            LY += 1;

            if (LY == 144)
            {
                uint8_t val = ReadMem(gb, 0xFF0F);
                val |= 1;
                WriteMem(gb, 0xFF0F, val);
            }
            if (LY > 153)
            {
                LY = 0;
            }
        }

        count += 1;
    }

    SimpleRender(gb, gb->ctx);
    DumpCPURegisters(gb);
}