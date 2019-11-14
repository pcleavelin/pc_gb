// GB.h - Main structure for the GameBoy

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <memory.h>

#ifdef WIN32
#include <SDL2/SDL.h>
#elif __APPLE__
#include <SDL.h>
#endif

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

    // Interrupt Master Enable flag
    bool IME;

    // Main Memory
    uint8_t mem[0x8000];

    // Cartridge Memory
    uint8_t *cart;
    uint32_t cartSize;

    // Boot ROM
    uint8_t *bootRom;
    uint32_t bootRomSize;
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
        -1080,
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
    else if (addr <= 0xFF && gb->mem[0xFF50 - 0x8000] == 0)
    {
        return gb->bootRom[addr % 0xFF];
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
        if (addr == 0xFF44)
        {
            gb->mem[addr - 0x8000] = 0;
        }
        else if (addr == 0xFF50)
        {
            printf("Touching DMG rom flag\n");
            gb->mem[addr - 0x8000] = val;
        }
        else
        {
            gb->mem[addr - 0x8000] = val;
        }
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
    printf("PC: 0x%02x\n", gb->regs[REG_PC]);
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
                uint8_t tileIndex = ReadMem(gb, bgBase + i + j * 32);

                uint16_t rowIndex = tileBase + (ty * 2);
                if (tileBase == 0x8800)
                {
                    rowIndex = 0x9000;
                    if ((tileIndex & 0x80) > 0)
                    {
                        rowIndex -= ((~tileIndex) + 1) * 16;
                    }
                    else
                    {
                        rowIndex += tileIndex * 16;
                    }
                }
                else
                {
                    rowIndex += tileIndex * 16;
                }

                // tileBase + tileIndex + tileY
                uint8_t row1 = ReadMem(gb, rowIndex);
                uint8_t row2 = ReadMem(gb, rowIndex + 1);

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

bool CheckInterrupt(GB *gb, uint8_t mask)
{
    uint8_t ienable = ReadMem(gb, 0xFFFF);
    uint8_t iflag = ReadMem(gb, 0xFF0F);

    if (gb->IME && (ienable & mask) > 0 && (iflag & mask) > 0)
    {
        WriteMem(gb, 0xFF0F, iflag & ~mask);

        return true;
    }

    return false;
}

void Push16(GB *gb, uint16_t val)
{
    gb->regs[REG_SP] -= 2;
    WriteMem(gb, gb->regs[REG_SP] + 1, (val & 0xFF00) >> 8);
    WriteMem(gb, gb->regs[REG_SP] + 2, val & 0xFF);
}

void CallInterrupt(GB *gb, uint8_t vector)
{
    printf("Calling interrupt: $%01X", vector);

    Push16(gb, gb->regs[REG_PC]);
    gb->regs[REG_PC] = vector;
}

uint16_t Pop16(GB *gb)
{
    gb->regs[REG_SP] += 2;
    uint16_t val = ReadMem(gb, gb->regs[REG_SP]);
    val |= ReadMem(gb, gb->regs[REG_SP] - 1) << 8;

    return val;
}

bool DoCBInstruction(GB *gb)
{
    const uint8_t instrPC = gb->regs[REG_PC];

    uint8_t opcode = FetchByte(gb);

    switch (opcode)
    {
    //------------------------------Rotate/Shift Commands---------------------------------
    case OP_CB_RL_B:
    case OP_CB_RL_C:
    case OP_CB_RL_D:
    case OP_CB_RL_E:
    case OP_CB_RL_H:
    case OP_CB_RL_L:
    case OP_CB_RL_A:
    {
        uint8_t reg = opcode & 0b111;
        uint8_t val = Get8Reg(gb, reg);

        val <<= 1;
        val |= gb->regs[REG_AF] & 0x8;

        SetFlag(gb, FLAG_Z, val == 0);
        SetFlag(gb, FLAG_C, (Get8Reg(gb, reg) & 0x80) > 0);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, RL %s\n", instrPC, GetReg8Name(reg));
#endif
    }
    break;

    case OP_CB_SWAP_B:
    case OP_CB_SWAP_C:
    case OP_CB_SWAP_D:
    case OP_CB_SWAP_E:
    case OP_CB_SWAP_H:
    case OP_CB_SWAP_L:
    case OP_CB_SWAP_A:
    {
        uint8_t reg = opcode & 0b111;
        uint8_t val = Get8Reg(gb, reg);
        uint8_t lo = val & 0xF;

        val = (val >> 4) | (lo << 4);

        Set8Reg(gb, reg, val);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, SWAP %s\n", instrPC, GetReg8Name(reg));
#endif
    }
    break;

    // --------------------Single Bit Operation Commands-------------------------------
    case OP_CB_BIT_n_B:
    case OP_CB_BIT_n_C:
    case OP_CB_BIT_n_D:
    case OP_CB_BIT_n_E:
    case OP_CB_BIT_n_H:
    case OP_CB_BIT_n_L:
    case OP_CB_BIT_n_A:
    {
        uint8_t bit = opcode >> 4;
        uint8_t reg = Get8Reg(gb, opcode & 0b111);

        SetFlag(gb, FLAG_Z, (reg & (1 << bit)) == 0);
        SetFlag(gb, FLAG_N, 0);
        SetFlag(gb, FLAG_H, 1);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, BIT %01X,%s\n", instrPC, bit, GetReg8Name(opcode & 0b111));
#endif
    }
    break;

    default:
        DumpCPURegisters(gb);
        printf("Unknown CB instruction: 0x%01X\n", opcode);
        return false;
    }

    return true;
}

bool DoInstruction(GB *gb)
{
    const uint16_t instrPC = gb->regs[REG_PC];

    if (instrPC == 0xfe && gb->mem[0xFF50 - 0x8000] == 0)
    {
        printf("Booted Up!\n");
    }

    // Fetch opcode
    uint8_t opcode = FetchByte(gb);

    if (opcode == 0xCB)
    {
        return DoCBInstruction(gb);
    }

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
        printf("PC: 0x%02X, LD %s,%s\n", instrPC, GetReg8Name(regDst), GetReg8Name(regSrc));
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
        printf("PC: 0x%02X, LD %s,$%01X\n", instrPC, GetReg8Name(regDst), val);
#endif
    }
    break;

    case OP_LD_ptrDE_A:
    {
#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, LD ($%02X),A - (DE)\n", instrPC, gb->regs[REG_DE]);
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

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, LD A, (FF00+$%01X) - A is now 0x%01X!\n", instrPC, offset, Get8Reg(gb, REG_A));
#endif
    }
    break;

    case OP_LD_IOn_A:
    {
        // TODO: write to io port
        uint8_t offset = FetchByte(gb);
        if (offset < 0x7F)
            WriteMem(gb, 0xFF00 + offset, Get8Reg(gb, REG_A));
#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, LD (FF00+$%01X),A - now $%01X!\n", instrPC, offset, ReadMem(gb, 0xFF00 + offset));
#endif
    }
    break;

    case OP_LD_IOC_A:
    {
        // TODO: write to io port
        uint8_t offset = Get8Reg(gb, REG_C);
        if (offset < 0x7F)
            WriteMem(gb, 0xFF00 + offset, Get8Reg(gb, REG_A));
#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, LD (FF00+$%01X),A - now $%01X!\n", instrPC, offset, ReadMem(gb, 0xFF00 + offset));
#endif
    }
    break;

    case OP_LDI_A_ptrHL:
    {

        uint8_t val = ReadMem(gb, gb->regs[REG_HL]);

        Set8Reg(gb, REG_A, val);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, LDI A,($%02X) - (HL)\n", instrPC, gb->regs[REG_HL]);
#endif
        gb->regs[REG_HL] += 1;
    }
    break;

    case OP_LDI_ptrHL_A:
    {
        uint8_t val = Get8Reg(gb, REG_A);
        WriteMem(gb, gb->regs[REG_HL], val);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, LDI ($%02X), A - (HL)\n", instrPC, gb->regs[REG_HL]);
#endif
        gb->regs[REG_HL] += 1;
    }
    break;

    case OP_LDD_ptrHL_A:
    {
        uint8_t val = Get8Reg(gb, REG_A);
        WriteMem(gb, gb->regs[REG_HL], val);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, LDD ($%02X), A - (HL)\n", instrPC, gb->regs[REG_HL]);
#endif
        gb->regs[REG_HL] -= 1;
    }
    break;

    case OP_LD_ptrHL_B:
    case OP_LD_ptrHL_C:
    case OP_LD_ptrHL_D:
    case OP_LD_ptrHL_E:
    case OP_LD_ptrHL_H:
    case OP_LD_ptrHL_L:
    case OP_LD_ptrHL_A:
    {
        uint8_t reg = opcode & 0b111;
        uint8_t val = Get8Reg(gb, reg);
        WriteMem(gb, gb->regs[REG_HL], val);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, LD ($%02X), %s - (HL)\n", instrPC, gb->regs[REG_HL], GetReg8Name(reg));
#endif
    }
    break;

    case OP_LD_ptrHL_n:
    {
        uint8_t val = FetchByte(gb);
        WriteMem(gb, gb->regs[REG_HL], val);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, LD ($%02X), $%01X - (HL)\n", instrPC, gb->regs[REG_HL], val);
#endif
    }
    break;

    case OP_LD_A_ptrBC:
    {
        uint8_t val = ReadMem(gb, gb->regs[REG_BC]);

        Set8Reg(gb, REG_A, val);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, LD A,($%02X) - (BC)\n", instrPC, gb->regs[REG_BC]);
#endif
    }
    break;

    case OP_LD_A_ptrDE:
    {
        uint8_t val = ReadMem(gb, gb->regs[REG_DE]);

        Set8Reg(gb, REG_A, val);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, LD A,($%02X) - (DE)\n", instrPC, gb->regs[REG_DE]);
#endif
    }
    break;

    case OP_LD_A_ptrnn:
    {
        uint8_t addr = FetchWord(gb);
        uint8_t val = ReadMem(gb, addr);

        Set8Reg(gb, REG_A, val);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, LD A,($%02X)\n", instrPC, addr);
#endif
    }
    break;

    case OP_LD_ptrBC_A:
    {
        uint8_t val = Get8Reg(gb, REG_A);
        WriteMem(gb, gb->regs[REG_BC], val);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, LD ($%02X), A - (BC)\n", instrPC, gb->regs[REG_BC]);
#endif
    }
    break;

    case OP_LD_ptrnn_A:
    {
        uint16_t addr = FetchWord(gb);

        WriteMem(gb, addr, Get8Reg(gb, REG_A));

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, LD ($%02X),A\n", instrPC, addr);
#endif
    }
    break;

    // 8bit Arthmetic/logical Commands
    case OP_ADD_A_ptrHL:
    {
        uint8_t val = ReadMem(gb, gb->regs[REG_HL]);
        uint8_t newVal = Get8Reg(gb, REG_A) + val;

        Set8Reg(gb, REG_A, newVal);

        SetFlag(gb, FLAG_Z, newVal == 0);
        SetFlag(gb, FLAG_C, newVal < Get8Reg(gb, REG_A));
        SetFlag(gb, FLAG_N, 0);
        SetFlag(gb, FLAG_H, (newVal & 0xF) < (Get8Reg(gb, REG_A) & 0xF));

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, ADD ($%02X) - (HL) A=$%01X\n", instrPC, gb->regs[REG_HL], Get8Reg(gb, REG_A));
#endif
    }
    break;

    case OP_SUB_B:
    case OP_SUB_C:
    case OP_SUB_D:
    case OP_SUB_E:
    case OP_SUB_H:
    case OP_SUB_L:
    case OP_SUB_A:
    {
        uint8_t reg = opcode & 0b111;
        uint8_t val = Get8Reg(gb, REG_A) - Get8Reg(gb, reg);

        SetFlag(gb, FLAG_Z, val == 0);
        SetFlag(gb, FLAG_C, Get8Reg(gb, REG_A) < Get8Reg(gb, reg));
        SetFlag(gb, FLAG_N, 1);
        SetFlag(gb, FLAG_H, 0);

        Set8Reg(gb, REG_A, val);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, SUB %s\n", instrPC, GetReg8Name(reg));
#endif
    }
    break;

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

        SetFlag(gb, FLAG_Z, val == 0);
        SetFlag(gb, FLAG_C, 0);
        SetFlag(gb, FLAG_N, 0);
        SetFlag(gb, FLAG_H, 1);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, AND %s\n", instrPC, GetReg8Name(reg));
#endif
    }
    break;

    case OP_AND_A_nn:
    {
        uint8_t val = FetchByte(gb);
        uint8_t newVal = Get8Reg(gb, REG_A) & val;
        Set8Reg(gb, REG_A, newVal);

        SetFlag(gb, FLAG_Z, newVal == 0);
        SetFlag(gb, FLAG_C, 0);
        SetFlag(gb, FLAG_N, 0);
        SetFlag(gb, FLAG_H, 1);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, AND $%01X\n", instrPC, val);
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

        SetFlag(gb, FLAG_Z, val == 0);
        SetFlag(gb, FLAG_C, 0);
        SetFlag(gb, FLAG_N, 0);
        SetFlag(gb, FLAG_H, 0);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, XOR %s\n", instrPC, GetReg8Name(reg));
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

        SetFlag(gb, FLAG_Z, val == 0);
        SetFlag(gb, FLAG_C, 0);
        SetFlag(gb, FLAG_N, 0);
        SetFlag(gb, FLAG_H, 0);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, OR %s\n", instrPC, GetReg8Name(reg));
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
        SetFlag(gb, FLAG_H, (newVal & 0xF) > (Get8Reg(gb, REG_A) & 0xF));

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, CP $%01X - A=$%01X\n", instrPC, val, Get8Reg(gb, REG_A));
#endif
    }
    break;

    case OP_CP_ptrHL:
    {
        uint8_t val = ReadMem(gb, gb->regs[REG_HL]);
        uint8_t newVal = Get8Reg(gb, REG_A) - val;

        SetFlag(gb, FLAG_Z, newVal == 0);
        SetFlag(gb, FLAG_C, newVal > Get8Reg(gb, REG_A));
        SetFlag(gb, FLAG_N, 1);
        SetFlag(gb, FLAG_H, (newVal & 0xF) < (Get8Reg(gb, REG_A) & 0xF));

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, CP ($%01X) - (HL) A=$%01X\n", instrPC, val, Get8Reg(gb, REG_A));
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
        printf("PC: 0x%02X, INC %s\n", instrPC, GetReg8Name(reg));
#endif
    }
    break;

    case OP_PUSH_BC:
    case OP_PUSH_DE:
    case OP_PUSH_HL:
    case OP_PUSH_AF:
    {
        uint8_t reg = (opcode >> 4) & 0b11;
        Push16(gb, gb->regs[reg]);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, PUSH %s\n", instrPC, GetRegName(reg));
        printf("Pushed $%02X\n", gb->regs[reg]);
#endif
    }
    break;

    case OP_POP_BC:
    case OP_POP_DE:
    case OP_POP_HL:
    case OP_POP_AF:
    {
        uint8_t reg = (opcode >> 4) & 0b11;
        gb->regs[reg] = Pop16(gb);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, POP %s\n", instrPC, GetRegName(reg));
        printf("Popped $%02X\n", gb->regs[reg]);
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
        printf("PC: 0x%02X, DEC %s\n", instrPC, GetReg8Name(reg));
#endif
    }
    break;

    case OP_CPL:
    {
        uint8_t val = Get8Reg(gb, REG_A) ^ 0xFF;
        Set8Reg(gb, REG_A, val);

        SetFlag(gb, FLAG_Z, 0);
        SetFlag(gb, FLAG_C, 0);
        SetFlag(gb, FLAG_N, 1);
        SetFlag(gb, FLAG_H, 1);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, CPL\n", instrPC);
#endif
    }
    break;

    //--------------------------16bit Load Commands--------------------------------------
    case OP_LD_BC_nn:
    case OP_LD_DE_nn:
    case OP_LD_HL_nn:
    case OP_LD_SP_nn:
    {

        uint8_t reg = (opcode & 0xF0) >> 4;
        uint16_t val = FetchWord(gb);

        gb->regs[reg] = val;

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, LD %s,$%02X\n", instrPC, GetRegName(reg), val);
#endif
    }
    break;

    case OP_INC_BC:
    case OP_INC_DE:
    case OP_INC_HL:
    case OP_INC_SP:
    {
        uint8_t reg = (opcode >> 4);

        gb->regs[reg] += 1;

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, INC %s\n", instrPC, GetRegName(reg));
#endif
    }
    break;

    case OP_DEC_BC:
    case OP_DEC_DE:
    case OP_DEC_HL:
    case OP_DEC_SP:
    {
        uint8_t reg = (opcode >> 4);

        gb->regs[reg] -= 1;

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, DEC %s\n", instrPC, GetRegName(reg));
#endif
    }
    break;

    //------------------------------Rotate/Shift Commands---------------------------------
    case OP_RLA:
    {
        uint8_t val = Get8Reg(gb, REG_A);

        val <<= 1;
        val |= gb->regs[REG_AF] & 0x8;

        SetFlag(gb, FLAG_C, (Get8Reg(gb, REG_A) & 0x80) > 0);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, RLA\n", instrPC);
#endif
    }
    break;

    // CPU Control Commands
    case OP_DI:
    {
        gb->IME = false;
#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, DI!\n", instrPC);
#endif
    }
    break;

    case OP_EI:
    {
        gb->IME = true;
#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, EI!\n", instrPC);
#endif
    }
    break;

    // Jump Commands
    case OP_JP_NN:
    {
        uint16_t addr = FetchWord(gb);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, JP $%02X\n", instrPC, addr);
#endif

        gb->regs[REG_PC] = addr;
    }
    break;

    case OP_JP_NZ_nn:
    case OP_JP_Z_nn:
    case OP_JP_NC_nn:
    case OP_JP_C_nn:
    {
        uint8_t addr = FetchWord(gb);
        uint8_t flag = (opcode >> 3) & 0b11;

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, JP %s,$%02X\n", instrPC, GetFlagName(flag), addr);
#endif
        if (CheckFlag(gb, flag))
        {
            gb->regs[REG_PC] = addr;
        }
        else
        {
            // printf("didn't jump\n");
        }
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
        printf("PC: 0x%02X, JR %s,PC+$%01X\n", instrPC, GetFlagName(flag), offset);
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
        printf("PC: 0x%02X, JR PC+$%01X\n", instrPC, (uint8_t)offset);
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
        Push16(gb, gb->regs[REG_PC]);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, CALL $%02X - (return to $%02X)\n", instrPC, addr, gb->regs[REG_PC]);
#endif
        gb->regs[REG_PC] = addr;
    }
    break;

    case OP_CALL_NZ_nn:
    case OP_CALL_Z_nn:
    case OP_CALL_NC_nn:
    case OP_CALL_C_nn:
    {
        uint8_t flag = opcode >> 3 & 0b111;
        uint16_t addr = FetchWord(gb);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, CALL %s,$%02X\n", instrPC, GetFlagName(flag), addr);
#endif

        if (CheckFlag(gb, flag))
        {
            gb->regs[REG_SP] -= 2;
            WriteMem(gb, gb->regs[REG_SP], gb->regs[REG_PC]);

            gb->regs[REG_PC] = addr;
        }
    }
    break;

    case OP_RET:
    {
#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, RET\n", instrPC);
#endif

        gb->regs[REG_PC] = Pop16(gb);
    }
    break;

    case OP_RET_NZ_nn:
    case OP_RET_Z_nn:
    case OP_RET_NC_nn:
    case OP_RET_C_nn:
    {
        uint8_t flag = opcode >> 3 & 0b111;

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, RET %s\n", instrPC, GetFlagName(flag));
#endif

        if (CheckFlag(gb, flag))
        {
            gb->regs[REG_PC] = Pop16(gb);
        }
    }
    break;

    case OP_RETI:
    {
#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, RETI\n", instrPC);
#endif

        gb->regs[REG_PC] = Pop16(gb);
        gb->IME = true;
    }
    break;

    case OP_RST_00:
    case OP_RST_08:
    case OP_RST_10:
    case OP_RST_18:
    case OP_RST_20:
    case OP_RST_28:
    case OP_RST_30:
    {
        uint8_t val = (FetchByte(gb) >> 3) & 0b111;
        Push16(gb, gb->regs[REG_PC]);

#ifdef _DBG_INSTR_
        printf("PC: 0x%02X, RST $%01X - (return to $%02X)\n", instrPC, val, gb->regs[REG_PC]);
#endif
        gb->regs[REG_PC] = val;
    }
    break;

    default:
    {
        DumpCPURegisters(gb);
        printf("PC: $%02X: Unknown instruction: 0x%01X\n", instrPC, opcode);
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

    uint8_t bootstrapROMSize = 0;
    gb->bootRom = LoadRom("../tests/carts/DMG_ROM.bin", &gb->bootRomSize);
    if (gb->bootRom == NULL)
    {
        return;
    }

    // gb->cart = LoadRom("../tests/carts/cpu_instrs/individual/06-ld r,r.gb", &gb->cartSize);
    gb->cart = LoadRom("../tests/carts/Tetris (JUE) (V1.1) [!].gb", &gb->cartSize);
    // gb->cart = LoadRom("../tests/carts/Dr. Mario (JU) (V1.1).gb", &gb->cartSize);
    // gb->cart = LoadRom("../tests/carts/Cadillac II (J).gb", &gb->cartSize);
    // gb->cart = LoadRom("../tests/carts/Legend of Zelda, The - Link's Awakening (U) (V1.2) [!].gb", &gb->cartSize);
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
    gb->regs[REG_PC] = 0x0;
    gb->IME = false;

    memset(gb->mem, 0x00, sizeof(uint8_t) * 0x8000);

    WriteMem(gb, 0xFF05, 0x00);
    WriteMem(gb, 0xFF06, 0x00);
    WriteMem(gb, 0xFF07, 0x00);
    WriteMem(gb, 0xFF10, 0x80);
    WriteMem(gb, 0xFF11, 0xBF);
    WriteMem(gb, 0xFF12, 0xF3);
    WriteMem(gb, 0xFF14, 0xBF);
    WriteMem(gb, 0xFF16, 0x3F);
    WriteMem(gb, 0xFF17, 0x00);
    WriteMem(gb, 0xFF19, 0xBF);
    WriteMem(gb, 0xFF1A, 0x7F);
    WriteMem(gb, 0xFF1B, 0xFF);
    WriteMem(gb, 0xFF1C, 0x9F);
    WriteMem(gb, 0xFF1E, 0xBF);
    WriteMem(gb, 0xFF20, 0xFF);
    WriteMem(gb, 0xFF21, 0x00);
    WriteMem(gb, 0xFF22, 0x00);
    WriteMem(gb, 0xFF23, 0xBF);
    WriteMem(gb, 0xFF24, 0x77);
    WriteMem(gb, 0xFF25, 0xF3);
    WriteMem(gb, 0xFF26, 0xF1);
    WriteMem(gb, 0xFF40, 0x91);
    WriteMem(gb, 0xFF42, 0x00);
    WriteMem(gb, 0xFF43, 0x00);
    WriteMem(gb, 0xFF45, 0x00);
    WriteMem(gb, 0xFF47, 0xFC);
    WriteMem(gb, 0xFF48, 0xFF);
    WriteMem(gb, 0xFF49, 0xFF);
    WriteMem(gb, 0xFF4A, 0x00);
    WriteMem(gb, 0xFF4B, 0x00);
    WriteMem(gb, 0xFFFF, 0x00);

    SimpleRender(gb, gb->ctx);

    uint64_t count = 0;

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

        // Check for interrupt requests
        if (CheckInterrupt(gb, VBLANK_MASK))
        {
            CallInterrupt(gb, 0x40);
        }
        else if (CheckInterrupt(gb, LCD_STAT_MASK))
        {
            CallInterrupt(gb, 0x48);
        }
        else if (CheckInterrupt(gb, TIMER_MASK))
        {
            CallInterrupt(gb, 0x50);
        }
        else if (CheckInterrupt(gb, SERIAL_MASK))
        {
            CallInterrupt(gb, 0x58);
        }
        else if (CheckInterrupt(gb, JOYPAD_MASK))
        {
            CallInterrupt(gb, 0x60);
        }

        if (!DoInstruction(gb))
        {
            running = false;
        }

        uint8_t LY = ReadMem(gb, 0xFF44);
        // if ((count % 8) == 0)
        {
            // SimpleRender(gb, gb->ctx);
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

        if ((count % 1024 * 8) == 0)
        {
            SimpleRender(gb, gb->ctx);
        }

        gb->mem[0xFF44 - 0x8000] = LY;
        count += 1;
    }

    SimpleRender(gb, gb->ctx);
    DumpCPURegisters(gb);
}