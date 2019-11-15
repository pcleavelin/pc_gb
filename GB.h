// GB.h - Main structure for the GameBoy

#include "GBOpcodes.h"

#include <assert.h>
#include <memory.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <SDL2/SDL.h>
#elif __APPLE__
#include <SDL.h>
#endif

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

#define instr(opcode, val, name) else if (opcode == val)

#define instr_ld_r_r(opcode, base, shift, name)  \
    else if ((opcode & base) == base &&          \
             ((opcode >> shift & 0b111) <= 7 &&  \
              (opcode >> shift & 0b111) != 6) && \
             ((opcode & 0b111) <= 7 && (opcode & 0b111) != 6))

#define instr_bit(opcode, name)                                         \
    else if ((opcode & 0x40) == 0x40 && (opcode >> 3 & 0b111) >= 0x8 && \
             (opcode >> 3 & 0b111) < 0xF &&                             \
             ((opcode & 0b111) <= 7 && (opcode & 0b111) != 6))

#define instr_res_bit(opcode, name)                                    \
    else if ((opcode & 0x0) == 0x80 && (opcode >> 3 & 0b111) >= 0x8 && \
             (opcode >> 3 & 0b111) < 0xF &&                            \
             ((opcode & 0b111) <= 7 && (opcode & 0b111) != 6))

#define instr_rst(opcode, name) \
    else if ((opcode & 0xC7) == 0xC7 && ((opcode & 0b111) <= 7))

#define instr_left_r(opcode, base, shift, name)                            \
    else if ((opcode & base) == base && ((opcode >> shift & 0b111) <= 7 && \
                                         (opcode >> shift & 0b111) != 6))

#define instr_left_f(opcode, base, shift, name) \
    else if ((opcode & base) == base && ((opcode >> shift & 0b11) <= 3))

#define instr_right_r(opcode, base, name) \
    else if ((opcode & base) == base &&   \
             ((opcode & 0b111) <= 7 && (opcode & 0b111) != 6))

#define instr_invalid() else

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
    SDL_Window *  window;
    SDL_Renderer *renderer;

    SDL_Texture *backbufferTexture;

    uint32_t *pixels;
    int       pitch;
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
    bool halted;

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
            ctx->pixels   = NULL;
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
        "pc_gb", -1080, SDL_WINDOWPOS_CENTERED, GB_VID_WIDTH * RENDER_SCALE,
        GB_VID_HEIGHT * RENDER_SCALE, SDL_WINDOW_SHOWN);

    if (ctx->window == NULL)
    {
        DestroyGBRenderContext(ctx);
        return NULL;
    }

    ctx->renderer =
        SDL_CreateRenderer(ctx->window, -1, SDL_RENDERER_ACCELERATED);
    if (ctx->renderer == NULL)
    {
        DestroyGBRenderContext(ctx);
        return NULL;
    }

    ctx->backbufferTexture = SDL_CreateTexture(
        ctx->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
        GB_VID_WIDTH * RENDER_SCALE, GB_VID_HEIGHT * RENDER_SCALE);
    if (ctx->backbufferTexture == NULL)
    {
        DestroyGBRenderContext(ctx);
        return NULL;
    }

    ctx->pixels = malloc(sizeof(uint32_t) * GB_VID_WIDTH * GB_VID_HEIGHT *
                         RENDER_SCALE * RENDER_SCALE);
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

    GB *gb  = malloc(sizeof(GB));
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
        return gb->bootRom[addr % 0x100];
    }
    else if (addr >= 0x4000 && addr <= 0x7FFF)
    {
        uint8_t bank = gb->mem[0x2000] & 0b11111;

        return gb->cart[(addr + (bank - 1) * 0x4000) % gb->cartSize];
    }
    else
    {
        return gb->cart[addr];
    }

    return 0;
}

void WriteMemRomOnly(GB *gb, uint16_t addr, uint8_t val)
{
    if (addr >= 0x8000)
    {
        if (addr == 0xFF44)
        {
            gb->mem[addr - 0x8000] = 0;
        }
        else
        {
            gb->mem[addr - 0x8000] = val;
        }
    }
}

void WriteMemMBC1(GB *gb, uint16_t addr, uint8_t val)
{
    printf("Not Implemented!\n");
    assert(false);
}

void WriteMemMBC1Ram(GB *gb, uint16_t addr, uint8_t val)
{
    printf("Not Implemented!\n");
    assert(false);
}

void WriteMemMBC1RamBat(GB *gb, uint16_t addr, uint8_t val)
{
    printf("Not Implemented!\n");
    assert(false);
}

void WriteMemMBC2(GB *gb, uint16_t addr, uint8_t val)
{
    printf("Not Implemented!\n");
    assert(false);
}

void WriteMemMBC2Bat(GB *gb, uint16_t addr, uint8_t val)
{
    printf("Not Implemented!\n");
    assert(false);
}

void WriteMem(GB *gb, uint16_t addr, uint8_t val)
{
    switch (gb->cart[CART_CART_TYPE])
    {
        case CART_TYPE_ROM_ONLY:
        {
            WriteMemRomOnly(gb, addr, val);
        }
        break;

        case CART_TYPE_MBC1:
        {
            WriteMemMBC1(gb, addr, val);
        }
        break;

        case CART_TYPE_MBC1_RAM:
        {
            WriteMemMBC1Ram(gb, addr, val);
        }
        break;

        case CART_TYPE_MBC1_RAM_BATTERY:
        {
            WriteMemMBC1RamBat(gb, addr, val);
        }
        break;

        case CART_TYPE_MBC2:
        {
            WriteMemMBC2(gb, addr, val);
        }
        break;

        case CART_TYPE_MBC2_BATTERY:
        {
            WriteMemMBC2Bat(gb, addr, val);
        }
        break;

        default:
            return;
    }

    // if (addr >= 0x8000)
    // {
    //     if (addr == 0xFF44)
    //     {
    //         gb->mem[addr - 0x8000] = 0;
    //     }
    //     else
    //     {
    //         gb->mem[addr - 0x8000] = val;
    //     }
    // }
    // else if (addr >= 0x2000 && addr <= 0x3FFF)
    // {
    //     if (val == 0)
    //         val = 1;
    //     gb->mem[0x2000] = val & 0b11111;
    // }
    // else if (addr >= 0x4000 && addr <= 0x5FFF)
    // {
    //     gb->mem[0x2000] = (val & 0b11) << 5;
    // }
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
    SDL_LockTexture(ctx->backbufferTexture, NULL, (void **)(&ctx->pixels),
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
    uint16_t bgBase   = (lcdControl & 0x8) > 0 ? 0x9C00 : 0x9800;
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

                    if (px >= GB_VID_WIDTH * RENDER_SCALE ||
                        py >= GB_VID_HEIGHT * RENDER_SCALE)
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
                                ctx->pixels[px + scalex +
                                            (py + scaley) * GB_VID_WIDTH *
                                                RENDER_SCALE] = color0;
                            }
                            else if (color == 1)
                            {
                                ctx->pixels[px + scalex +
                                            (py + scaley) * GB_VID_WIDTH *
                                                RENDER_SCALE] = color1;
                            }
                            else if (color == 2)
                            {
                                ctx->pixels[px + scalex +
                                            (py + scaley) * GB_VID_WIDTH *
                                                RENDER_SCALE] = color2;
                            }
                            else if (color == 3)
                            {
                                ctx->pixels[px + scalex +
                                            (py + scaley) * GB_VID_WIDTH *
                                                RENDER_SCALE] = color3;
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
    assert(reg != 6 && reg <= 7);

    size_t   index  = reg == REG_A ? 7 : reg / 2;
    uint16_t bigVal = val;
    uint16_t mask   = 0xFF00;
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
    assert(reg != 6 && reg <= 7);

    size_t   index = reg == REG_A ? 7 : reg / 2;
    uint16_t val   = gb->regs[index];
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
            assert(false);
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
            assert(false);
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
            assert(false);
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
            assert(false);
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
            printf("Checking flag that doesn't not exist. This should not have "
                   "been called!\n");
            assert(false);
    }
}

bool CheckInterrupt(GB *gb, uint8_t mask)
{
    uint8_t ienable = ReadMem(gb, 0xFFFF);
    uint8_t iflag   = ReadMem(gb, 0xFF0F);

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
    Push16(gb, gb->regs[REG_PC]);
    gb->regs[REG_PC] = vector;

    gb->halted = false;
}

uint16_t Pop16(GB *gb)
{
    gb->regs[REG_SP] += 2;
    uint16_t val = ReadMem(gb, gb->regs[REG_SP]);
    val |= ReadMem(gb, gb->regs[REG_SP] - 1) << 8;

    return val;
}

bool DoCBInstruction(GB *gb, uint16_t instrPC)
{
    uint8_t opcode = FetchByte(gb);

    if (false)
    {
    }

    instr_right_r(opcode, 0x10, "rl r")
    {
        uint8_t reg = opcode & 0b111;
        uint8_t val = Get8Reg(gb, reg);

        val <<= 1;
        val |= (gb->regs[REG_AF] & 0x8) >> 4;

        SetFlag(gb, FLAG_Z, val == 0);
        SetFlag(gb, FLAG_C, Get8Reg(gb, reg) >> 7);

        Set8Reg(gb, reg, val);
    }

    instr_right_r(opcode, 0x18, "rr r")
    {
        uint8_t reg = opcode & 0b111;
        uint8_t val = Get8Reg(gb, reg);

        val >>= 1;
        val |= (gb->regs[REG_AF] & 0x8) << 3;

        SetFlag(gb, FLAG_Z, val == 0);
        SetFlag(gb, FLAG_C, Get8Reg(gb, reg) & 1);

        Set8Reg(gb, reg, val);
    }

    instr(opcode, 0x1E, "rr (hl)")
    {
        uint8_t val = ReadMem(gb, gb->regs[REG_HL]);

        val >>= 1;
        val |= (gb->regs[REG_AF] & 0x8) << 3;

        SetFlag(gb, FLAG_Z, val == 0);
        SetFlag(gb, FLAG_C, ReadMem(gb, gb->regs[REG_HL]) & 1);

        WriteMem(gb, gb->regs[REG_HL], val);
    }

    instr_right_r(opcode, 0x20, "sla r")
    {
        uint8_t reg = opcode & 0b111;
        uint8_t val = Get8Reg(gb, reg);

        val <<= 1;

        Set8Reg(gb, reg, val);
    }

    instr_right_r(opcode, 0x30, "swap r")
    {
        uint8_t reg = opcode & 0b111;
        uint8_t val = Get8Reg(gb, reg);
        uint8_t lo  = val & 0xF;

        val = (val >> 4) | (lo << 4);

        Set8Reg(gb, reg, val);
    }

    instr_right_r(opcode, 0x38, "srl r")
    {
        uint8_t reg = opcode & 0b111;
        uint8_t val = Get8Reg(gb, reg);

        val >>= 1;

        Set8Reg(gb, reg, val);
    }

    instr_bit(opcode, "bit n,r")
    {
        uint8_t bit = opcode >> 4;
        uint8_t reg = Get8Reg(gb, opcode & 0b111);

        SetFlag(gb, FLAG_Z, (reg & (1 << bit)) == 0);
        SetFlag(gb, FLAG_N, 0);
        SetFlag(gb, FLAG_H, 1);
    }

    instr_res_bit(opcode, "res n,r")
    {
        uint8_t bit = opcode >> 4;
        uint8_t reg = Get8Reg(gb, opcode & 0b111);

        Set8Reg(gb, opcode & 0b111, (reg & ~(1 << bit)));
    }

    instr_invalid()
    {
        DumpCPURegisters(gb);
        printf("PC: $%02X: Unknown CB-prefixed instruction: 0x%01X\n", instrPC,
               opcode);
        return false;
    }

    return true;
}

bool DoInstruction(GB *gb)
{
    const uint16_t instrPC = gb->regs[REG_PC];
    uint8_t        opcode  = FetchByte(gb);

    if (opcode == 0xCB)
    {
        return DoCBInstruction(gb, instrPC);
    }

    if (opcode == 0x00)
    {
    }

    //-------------8 bit Load Commands-------------
    instr(opcode, 0x08, "ld (nn), sp")
    {
        uint8_t addr = FetchWord(gb);

        WriteMem(gb, addr, gb->regs[REG_SP] >> 8);
        WriteMem(gb, addr + 1, gb->regs[REG_SP] & 0xFF);
    }

    instr_ld_r_r(opcode, 0x40, 3, "ld r,r")
    {
        uint8_t regDst = (opcode >> 3) & 0b111;
        uint8_t regSrc = opcode & 0b111;

        Set8Reg(gb, regDst, Get8Reg(gb, regSrc));
    }

    instr_left_r(opcode, 0x6, 3, "ld r,n")
    {
        uint8_t regDst = (opcode >> 3) & 0b111;
        uint8_t val    = FetchByte(gb);

        Set8Reg(gb, regDst, val);
    }

    instr(opcode, 0x12, "ld (de),a")
    {
        WriteMem(gb, gb->regs[REG_DE], Get8Reg(gb, REG_A));
    }

    instr(opcode, 0xF0, "ld a,($FF00+n)")
    {
        uint8_t offset = FetchByte(gb);
        Set8Reg(gb, REG_A, ReadMem(gb, 0xFF00 + offset));
    }

    instr(opcode, 0xE0, "ld ($FF00+n), a")
    {
        uint8_t offset = FetchByte(gb);
        WriteMem(gb, 0xFF00 + offset, Get8Reg(gb, REG_A));
    }

    instr(opcode, 0xE2, "ld ($FF00+c), a")
    {
        uint8_t offset = Get8Reg(gb, REG_C);
        WriteMem(gb, 0xFF00 + offset, Get8Reg(gb, REG_A));
    }

    instr(opcode, 0x2A, "ldi a,(hl)")
    {
        uint8_t val = ReadMem(gb, gb->regs[REG_HL]);

        Set8Reg(gb, REG_A, val);

        gb->regs[REG_HL] += 1;
    }

    instr(opcode, 0x22, "ldi a,(hl)")
    {
        uint8_t val = Get8Reg(gb, REG_A);
        WriteMem(gb, gb->regs[REG_HL], val);

        gb->regs[REG_HL] += 1;
    }

    instr(opcode, 0x32, "ldd a,(hl)")
    {
        uint8_t val = Get8Reg(gb, REG_A);
        WriteMem(gb, gb->regs[REG_HL], val);

        gb->regs[REG_HL] -= 1;
    }

    instr_left_r(opcode, 0x46, 3, "ld a,(hl)")
    {
        uint8_t reg = (opcode >> 3) & 0b111;
        uint8_t val = ReadMem(gb, gb->regs[REG_HL]);

        Set8Reg(gb, reg, val);
    }

    instr_right_r(opcode, 0x70, "ld (hl),r")
    {
        uint8_t reg = opcode & 0b111;
        uint8_t val = Get8Reg(gb, reg);
        WriteMem(gb, gb->regs[REG_HL], val);
    }

    instr(opcode, 0x36, "ld (hl),n")
    {
        uint8_t val = FetchByte(gb);
        WriteMem(gb, gb->regs[REG_HL], val);
    }

    instr(opcode, 0x0A, "ld a,(bc)")
    {
        uint8_t val = ReadMem(gb, gb->regs[REG_BC]);

        Set8Reg(gb, REG_A, val);
    }

    instr(opcode, 0x1A, "ld a,(de)")
    {
        uint8_t val = ReadMem(gb, gb->regs[REG_DE]);

        Set8Reg(gb, REG_A, val);
    }

    instr(opcode, 0xFA, "ld a,(nn)")
    {
        uint8_t addr = FetchWord(gb);
        uint8_t val  = ReadMem(gb, addr);

        Set8Reg(gb, REG_A, val);
    }

    instr(opcode, 0x02, "ld (bc),a")
    {
        uint8_t val = Get8Reg(gb, REG_A);
        WriteMem(gb, gb->regs[REG_BC], val);
    }

    instr(opcode, 0xEA, "ld (nn),a")
    {
        uint16_t addr = FetchWord(gb);

        WriteMem(gb, addr, Get8Reg(gb, REG_A));
    }

    //-------------8 bit Arthimetic/Logical Commands-------------
    instr_right_r(opcode, 0x80, "add a,r")
    {
        uint8_t reg = opcode & 0b111;
        uint8_t val = Get8Reg(gb, REG_A) + Get8Reg(gb, reg);

        SetFlag(gb, FLAG_Z, val == 0);
        SetFlag(gb, FLAG_C, val < Get8Reg(gb, REG_A));
        SetFlag(gb, FLAG_N, 0);
        SetFlag(gb, FLAG_H, (val & 0xF) < (Get8Reg(gb, REG_A) & 0xF));

        Set8Reg(gb, REG_A, val);
    }

    instr(opcode, 0xC6, "add a,n")
    {
        uint8_t val    = FetchByte(gb);
        uint8_t newVal = Get8Reg(gb, REG_A) + val;

        SetFlag(gb, FLAG_Z, newVal == 0);
        SetFlag(gb, FLAG_C, newVal < Get8Reg(gb, REG_A));
        SetFlag(gb, FLAG_N, 0);
        SetFlag(gb, FLAG_H, (newVal & 0xF) < (Get8Reg(gb, REG_A) & 0xF));

        Set8Reg(gb, REG_A, newVal);
    }

    instr_right_r(opcode, 0x88, "adc a,r")
    {
        uint8_t reg = opcode & 0b111;
        uint8_t val = Get8Reg(gb, REG_A) + Get8Reg(gb, reg) +
                      (gb->regs[REG_AF] & 0x8 >> 3);

        SetFlag(gb, FLAG_Z, val == 0);
        SetFlag(gb, FLAG_C, val < Get8Reg(gb, REG_A));
        SetFlag(gb, FLAG_N, 0);
        SetFlag(gb, FLAG_H, (val & 0xF) < (Get8Reg(gb, REG_A) & 0xF));

        Set8Reg(gb, REG_A, val);
    }

    instr(opcode, 0x86, "add a,(hl)")
    {
        uint8_t val    = ReadMem(gb, gb->regs[REG_HL]);
        uint8_t newVal = Get8Reg(gb, REG_A) + val;

        SetFlag(gb, FLAG_Z, newVal == 0);
        SetFlag(gb, FLAG_C, newVal < Get8Reg(gb, REG_A));
        SetFlag(gb, FLAG_N, 0);
        SetFlag(gb, FLAG_H, (newVal & 0xF) < (Get8Reg(gb, REG_A) & 0xF));

        Set8Reg(gb, REG_A, newVal);
    }

    instr(opcode, 0x8E, "adc a,(hl)")
    {
        uint8_t val = ReadMem(gb, gb->regs[REG_HL]);
        uint8_t newVal =
            Get8Reg(gb, REG_A) + val + (gb->regs[REG_AF] & 0x8 >> 3);

        SetFlag(gb, FLAG_Z, newVal == 0);
        SetFlag(gb, FLAG_C, newVal < Get8Reg(gb, REG_A));
        SetFlag(gb, FLAG_N, 0);
        SetFlag(gb, FLAG_H, (newVal & 0xF) < (Get8Reg(gb, REG_A) & 0xF));

        Set8Reg(gb, REG_A, newVal);
    }

    instr_right_r(opcode, 0x90, "sub a,r")
    {
        uint8_t reg = opcode & 0b111;
        uint8_t val = Get8Reg(gb, REG_A) - Get8Reg(gb, reg);

        SetFlag(gb, FLAG_Z, val == 0);
        SetFlag(gb, FLAG_C, val > Get8Reg(gb, REG_A));
        SetFlag(gb, FLAG_N, 1);
        SetFlag(gb, FLAG_H, (val & 0xF) > (Get8Reg(gb, REG_A) & 0xF));

        Set8Reg(gb, REG_A, val);
    }

    instr(opcode, 0xD6, "sub a,n")
    {
        uint8_t val    = FetchByte(gb);
        uint8_t newVal = Get8Reg(gb, REG_A) - val;

        SetFlag(gb, FLAG_Z, newVal == 0);
        SetFlag(gb, FLAG_C, newVal > Get8Reg(gb, REG_A));
        SetFlag(gb, FLAG_N, 1);
        SetFlag(gb, FLAG_H, (newVal & 0xF) > (Get8Reg(gb, REG_A) & 0xF));

        Set8Reg(gb, REG_A, newVal);
    }

    instr_right_r(opcode, 0x98, "sbc a,r")
    {
        uint8_t reg = opcode & 0b111;
        uint8_t val = Get8Reg(gb, REG_A) - Get8Reg(gb, reg) -
                      (gb->regs[REG_AF] & 0x8 >> 3);

        SetFlag(gb, FLAG_Z, val == 0);
        SetFlag(gb, FLAG_C, val > Get8Reg(gb, REG_A));
        SetFlag(gb, FLAG_N, 1);
        SetFlag(gb, FLAG_H, (val & 0xF) > (Get8Reg(gb, REG_A) & 0xF));

        Set8Reg(gb, REG_A, val);
    }

    instr(opcode, 0x9E, "sbc a,(hl)")
    {
        uint8_t reg = opcode & 0b111;
        uint8_t val = Get8Reg(gb, REG_A) - ReadMem(gb, gb->regs[REG_HL]) -
                      (gb->regs[REG_AF] & 0x8 >> 3);

        SetFlag(gb, FLAG_Z, val == 0);
        SetFlag(gb, FLAG_C, val > Get8Reg(gb, REG_A));
        SetFlag(gb, FLAG_N, 1);
        SetFlag(gb, FLAG_H, (val & 0xF) > (Get8Reg(gb, REG_A) & 0xF));

        Set8Reg(gb, REG_A, val);
    }

    instr_right_r(opcode, 0xA0, "and a,r")
    {
        uint8_t reg = opcode & 0b111;
        uint8_t val = Get8Reg(gb, REG_A) & Get8Reg(gb, reg);
        Set8Reg(gb, REG_A, val);

        SetFlag(gb, FLAG_Z, val == 0);
        SetFlag(gb, FLAG_C, 0);
        SetFlag(gb, FLAG_N, 0);
        SetFlag(gb, FLAG_H, 1);
    }

    instr(opcode, 0xE6, "and a,n")
    {
        uint8_t val    = FetchByte(gb);
        uint8_t newVal = Get8Reg(gb, REG_A) & val;
        Set8Reg(gb, REG_A, newVal);

        SetFlag(gb, FLAG_Z, newVal == 0);
        SetFlag(gb, FLAG_C, 0);
        SetFlag(gb, FLAG_N, 0);
        SetFlag(gb, FLAG_H, 1);
    }

    instr(opcode, 0xF6, "or a,n")
    {
        uint8_t val    = FetchByte(gb);
        uint8_t newVal = Get8Reg(gb, REG_A) | val;
        Set8Reg(gb, REG_A, newVal);

        SetFlag(gb, FLAG_Z, newVal == 0);
        SetFlag(gb, FLAG_C, 0);
        SetFlag(gb, FLAG_N, 0);
        SetFlag(gb, FLAG_H, 0);
    }

    instr(opcode, 0xB6, "or a,(hl)")
    {
        uint8_t newVal = Get8Reg(gb, REG_A) | ReadMem(gb, gb->regs[REG_HL]);
        Set8Reg(gb, REG_A, newVal);

        SetFlag(gb, FLAG_Z, newVal == 0);
        SetFlag(gb, FLAG_C, 0);
        SetFlag(gb, FLAG_N, 0);
        SetFlag(gb, FLAG_H, 0);
    }

    instr_right_r(opcode, 0xA8, "xor a,r")
    {
        uint8_t reg = opcode & 0b111;
        uint8_t val = Get8Reg(gb, REG_A) ^ Get8Reg(gb, reg);

        SetFlag(gb, FLAG_Z, val == 0);
        SetFlag(gb, FLAG_C, 0);
        SetFlag(gb, FLAG_N, 0);
        SetFlag(gb, FLAG_H, 0);
    }

    instr(opcode, 0xAE, "xor a,(hl)")
    {
        uint8_t val = Get8Reg(gb, REG_A) ^ ReadMem(gb, gb->regs[REG_HL]);

        SetFlag(gb, FLAG_Z, val == 0);
        SetFlag(gb, FLAG_C, 0);
        SetFlag(gb, FLAG_N, 0);
        SetFlag(gb, FLAG_H, 0);
    }

    instr_right_r(opcode, 0xB0, "or a,r")
    {
        uint8_t reg = opcode & 0b111;
        uint8_t val = Get8Reg(gb, REG_A) | Get8Reg(gb, reg);
        Set8Reg(gb, REG_A, val);

        SetFlag(gb, FLAG_Z, val == 0);
        SetFlag(gb, FLAG_C, 0);
        SetFlag(gb, FLAG_N, 0);
        SetFlag(gb, FLAG_H, 0);
    }

    instr_right_r(opcode, 0xB8, "cp a,r")
    {
        uint8_t reg = opcode & 0b111;
        uint8_t val = Get8Reg(gb, REG_A) - Get8Reg(gb, reg);

        SetFlag(gb, FLAG_Z, val == 0);
        SetFlag(gb, FLAG_C, val > Get8Reg(gb, REG_A));
        SetFlag(gb, FLAG_N, 1);
        SetFlag(gb, FLAG_H, (val & 0xF) > (Get8Reg(gb, REG_A) & 0xF));
    }

    instr(opcode, 0xFE, "cp a,n")
    {
        uint8_t val    = FetchByte(gb);
        uint8_t newVal = Get8Reg(gb, REG_A) - val;

        SetFlag(gb, FLAG_Z, newVal == 0);
        SetFlag(gb, FLAG_C, newVal > Get8Reg(gb, REG_A));
        SetFlag(gb, FLAG_N, 1);
        SetFlag(gb, FLAG_H, (newVal & 0xF) > (Get8Reg(gb, REG_A) & 0xF));
    }

    instr(opcode, 0xBE, "cp a,(hl)")
    {
        uint8_t val    = ReadMem(gb, gb->regs[REG_HL]);
        uint8_t newVal = Get8Reg(gb, REG_A) - val;

        SetFlag(gb, FLAG_Z, newVal == 0);
        SetFlag(gb, FLAG_C, newVal > Get8Reg(gb, REG_A));
        SetFlag(gb, FLAG_N, 1);
        SetFlag(gb, FLAG_H, (newVal & 0xF) > (Get8Reg(gb, REG_A) & 0xF));
    }

    instr_left_r(opcode, 0x4, 3, "inc r")
    {
        uint8_t reg = (opcode >> 3) & 0b111;
        uint8_t val = Get8Reg(gb, reg) + 1;

        SetFlag(gb, FLAG_Z, val == 0);
        SetFlag(gb, FLAG_N, 0);
        SetFlag(gb, FLAG_H, (val & 0xF) < (Get8Reg(gb, reg) & 0xF));

        Set8Reg(gb, reg, val);
    }

    instr(opcode, 0x34, "inc (hl)")
    {
        uint8_t data = ReadMem(gb, gb->regs[REG_HL]);
        uint8_t val  = data + 1;

        SetFlag(gb, FLAG_Z, val == 0);
        SetFlag(gb, FLAG_N, 0);
        SetFlag(gb, FLAG_H, (val & 0xF) < (data & 0xF));

        WriteMem(gb, gb->regs[REG_HL], val);
    }

    instr_left_r(opcode, 0x5, 3, "dec r")
    {
        uint8_t reg = (opcode >> 3) & 0b111;
        uint8_t val = Get8Reg(gb, reg) - 1;

        SetFlag(gb, FLAG_Z, val == 0);
        SetFlag(gb, FLAG_N, 1);
        SetFlag(gb, FLAG_H, (val & 0xF) > (Get8Reg(gb, reg) & 0xF));

        Set8Reg(gb, reg, val);
    }

    instr(opcode, 0x35, "dec (hl)")
    {
        uint8_t data = ReadMem(gb, gb->regs[REG_HL]);
        uint8_t val  = data - 1;

        SetFlag(gb, FLAG_Z, val == 0);
        SetFlag(gb, FLAG_N, 1);
        SetFlag(gb, FLAG_H, (val & 0xF) > (data & 0xF));

        WriteMem(gb, gb->regs[REG_HL], val);
    }

    instr(opcode, 0x27, "daa")
    {
        uint8_t val = Get8Reg(gb, REG_A);

        if ((val & 0xF) > 9 || (gb->regs[REG_AF] & 0x20) > 0)
        {
            if ((gb->regs[REG_AF] & 0x10) > 0)
            {
                val -= 6;
            }
            else
            {
                val += 6;
            }

            SetFlag(gb, FLAG_C, 0);
        }

        if ((val & 0xF0) > 9 || (gb->regs[REG_AF] & 0x20) > 0)
        {
            if ((gb->regs[REG_AF] & 0x4) > 0)
            {
                val -= 0x60;
            }
            else
            {
                val += 0x60;
            }

            SetFlag(gb, FLAG_C, 1);
        }

        SetFlag(gb, FLAG_Z, val == 0);
        SetFlag(gb, FLAG_H, 0);

        Set8Reg(gb, REG_A, val);
    }

    instr(opcode, 0x2F, "cpl")
    {
        uint8_t val = Get8Reg(gb, REG_A) ^ 0xFF;
        Set8Reg(gb, REG_A, val);

        SetFlag(gb, FLAG_N, 1);
        SetFlag(gb, FLAG_H, 1);
    }

    //-------------16 bit Load/Arithmetic/Logical Commands-------------
    instr_left_r(opcode, 0x01, 4, "ld rr, nn")
    {
        uint8_t  reg = (opcode & 0xF0) >> 4;
        uint16_t val = FetchWord(gb);

        gb->regs[reg] = val;
    }

    instr_left_r(opcode, 0xC5, 4, "push rr")
    {
        uint8_t reg = (opcode >> 4) & 0b11;
        Push16(gb, gb->regs[reg]);
    }

    instr_left_r(opcode, 0xC1, 4, "pop rr")
    {
        uint8_t reg   = (opcode >> 4) & 0b11;
        gb->regs[reg] = Pop16(gb);
    }

    instr_left_r(opcode, 0x09, 4, "add hl, rr")
    {
        uint8_t reg    = (opcode >> 4) & 0b11;
        uint8_t newVal = gb->regs[REG_HL] + gb->regs[reg];

        SetFlag(gb, FLAG_C, newVal < gb->regs[REG_HL]);
        SetFlag(gb, FLAG_N, 0);
        SetFlag(gb, FLAG_H, (newVal & 0xF) < (gb->regs[REG_HL] & 0xF));

        gb->regs[REG_HL] = newVal;
    }

    instr_left_r(opcode, 0x03, 4, "inc rr")
    {
        uint8_t reg = (opcode >> 4) & 0b11;

        gb->regs[reg] += 1;
    }

    instr_left_r(opcode, 0x0B, 4, "dec rr")
    {
        uint8_t reg = (opcode >> 4) & 0b11;

        gb->regs[reg] -= 1;
    }

    instr(opcode, 0xE8, "add sp, dd")
    {
        uint8_t val    = FetchByte(gb);
        uint8_t newVal = gb->regs[REG_SP];

        if ((val & 0x80) > 0)
        {
            val = ~val;
            val += 1;

            newVal -= val;
            SetFlag(gb, FLAG_H, (newVal & 0xF) > (gb->regs[REG_SP] & 0xF));
        }
        else
        {
            newVal += val;
            SetFlag(gb, FLAG_H, (newVal & 0xF) < (gb->regs[REG_SP] & 0xF));
        }

        SetFlag(gb, FLAG_Z, 0);
        SetFlag(gb, FLAG_C, ((newVal ^ gb->regs[REG_SP]) & 0x80) > 0);
        SetFlag(gb, FLAG_N, 0);

        gb->regs[REG_SP] = newVal;
    }

    //-------------Rotate/Shift Commands-------------
    instr(opcode, 0x07, "rlca")
    {
        uint8_t val = Get8Reg(gb, REG_A);

        uint8_t tmp = val >> 7;
        val <<= 1;
        val |= tmp;

        SetFlag(gb, FLAG_Z, 0);
        SetFlag(gb, FLAG_C, (Get8Reg(gb, REG_A) & 0x80) > 0);
        SetFlag(gb, FLAG_N, 0);
        SetFlag(gb, FLAG_H, 0);

        Set8Reg(gb, REG_A, val);
    }

    instr(opcode, 0x17, "rla")
    {
        uint8_t val = Get8Reg(gb, REG_A);

        val <<= 1;
        val |= gb->regs[REG_AF] & 0x8;

        SetFlag(gb, FLAG_Z, 0);
        SetFlag(gb, FLAG_C, Get8Reg(gb, REG_A) >> 7);
        SetFlag(gb, FLAG_N, 0);
        SetFlag(gb, FLAG_H, 0);

        Set8Reg(gb, REG_A, val);
    }

    instr(opcode, 0x1F, "rra")
    {
        uint8_t val = Get8Reg(gb, REG_A);

        val >>= 1;
        val |= (gb->regs[REG_AF] & 0x8) << 3;

        SetFlag(gb, FLAG_Z, 0);
        SetFlag(gb, FLAG_C, Get8Reg(gb, REG_A) & 1);
        SetFlag(gb, FLAG_N, 0);
        SetFlag(gb, FLAG_H, 0);

        Set8Reg(gb, REG_A, val);
    }

    //-------------CPU Control Commands-------------
    instr(opcode, 0x3F, "ccf")
    {
        SetFlag(gb, FLAG_C, 0);
    }

    instr(opcode, 0x37, "scf")
    {
        SetFlag(gb, FLAG_C, 1);
    }

    instr(opcode, 0x76, "halt")
    {
        gb->halted = true;
    }

    instr(opcode, 0x10, "stop")
    {
        // TODO: actualy stop instead of nop'ing
        FetchByte(gb);
    }

    instr(opcode, 0xF3, "di")
    {
        gb->IME = false;
    }

    instr(opcode, 0xFB, "ei")
    {
        gb->IME = true;
    }

    //-------------Jump Commands-------------
    instr(opcode, 0xC3, "jp nn")
    {
        uint16_t addr = FetchWord(gb);

        gb->regs[REG_PC] = addr + 1;
    }

    instr(opcode, 0xE9, "jp hl")
    {
        uint16_t addr = gb->regs[REG_HL];

        gb->regs[REG_PC] = addr;
    }

    instr_left_f(opcode, 0xC2, 3, "jp f,nn")
    {
        uint8_t addr = FetchWord(gb);
        uint8_t flag = (opcode >> 3) & 0b11;

        if (CheckFlag(gb, flag))
        {
            gb->regs[REG_PC] = addr;
        }
        else
        {
            // printf("didn't jump\n");
        }
    }

    instr_left_f(opcode, 0x20, 3, "jp f,dd")
    {
        uint8_t offset = FetchByte(gb);
        uint8_t flag   = (opcode >> 3) & 0b11;

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

    instr(opcode, 0x18, "jr dd")
    {
        uint8_t offset = FetchByte(gb);

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

    instr(opcode, 0xCD, "call nn")
    {
        uint16_t addr = FetchWord(gb);
        Push16(gb, gb->regs[REG_PC]);

        gb->regs[REG_PC] = addr;
    }

    instr_left_r(opcode, 0xC4, 3, "call f,nn")
    {
        uint8_t  flag = opcode >> 3 & 0b111;
        uint16_t addr = FetchWord(gb);

        if (CheckFlag(gb, flag))
        {
            gb->regs[REG_SP] -= 2;
            WriteMem(gb, gb->regs[REG_SP], gb->regs[REG_PC]);

            gb->regs[REG_PC] = addr;
        }
    }

    instr(opcode, 0xC9, "ret")
    {
        gb->regs[REG_PC] = Pop16(gb);
    }

    instr_left_r(opcode, 0xC0, 3, "ret f,nn")
    {
        uint8_t flag = opcode >> 3 & 0b111;

        if (CheckFlag(gb, flag))
        {
            gb->regs[REG_PC] = Pop16(gb);
        }
    }

    instr(opcode, 0xD9, "reti")
    {
        gb->regs[REG_PC] = Pop16(gb);
        gb->IME          = true;
    }

    instr_rst(opcode, "rst n")
    {
        uint8_t val = (FetchByte(gb) >> 3) & 0b111;
        Push16(gb, gb->regs[REG_PC]);

        gb->regs[REG_PC] = val;
    }

    instr_invalid()
    {
        DumpCPURegisters(gb);
        printf("PC: $%02X: Unknown instruction: 0x%01X\n", instrPC, opcode);
        return false;
    }
    return true;
}

void StartGB(GB *gb, const char *rom)
{
    if (gb == NULL)
    {
        return;
    }

    printf("GB Starting...\n");

    uint8_t bootstrapROMSize = 0;
    gb->bootRom              = LoadRom("DMG_ROM.bin", &gb->bootRomSize);
    if (gb->bootRom == NULL)
    {
        return;
    }

    gb->cart = LoadRom(rom, &gb->cartSize);
    if (gb->cart == NULL)
    {
        return;
    }

    DumpRomInfo(gb);

    // Info from
    // https://realboyemulator.files.wordpress.com/2013/01/gbcpuman.pdf
    // Power Up Sequence (Found on Page 18)

    gb->regs[REG_BC] = 0x0013;
    gb->regs[REG_DE] = 0x00D8;
    gb->regs[REG_HL] = 0x014D;
    gb->regs[REG_SP] = 0xFFFE;
    gb->regs[REG_AF] = 0x0000;

    gb->regs[REG_PC] = 0x0;
    gb->IME          = false;
    gb->halted       = false;

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

        if (!gb->halted && !DoInstruction(gb))
        {
            running = false;
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