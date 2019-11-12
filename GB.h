// GB.h - Main structure for the GameBoy

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "GBOpcodes.h"

#define GB_VID_WIDTH 160
#define GB_VID_HEIGHT 144

#define RENDER_SCALE 4

#define REG_BC 0x00
#define REG_DE 0x01
#define REG_HL 0x02
#define REG_SP 0x03
#define REG_PC 0x04

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

    // BC[0], DE[1], HL[2], SP[3], PC[4]
    // Stored as an array for easier instruction handling
    uint16_t regs[5];
    uint8_t A, F;

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
    printf("\tA: 0x%01x\n", gb->A);
    printf("\tBC: 0x%02x\n", gb->regs[REG_BC]);
    printf("\tDE: 0x%02x\n", gb->regs[REG_DE]);
    printf("\tHL: 0x%02x\n", gb->regs[REG_HL]);
    printf("\tSP: 0x%02x\n", gb->regs[REG_SP]);
    printf("PC: 0x%02x\n", gb->regs[REG_PC]);
}

void SimpleRender(GB *gb, RenderContext *ctx)
{
    SDL_LockTexture(
        ctx->backbufferTexture,
        NULL,
        (void **)(&ctx->pixels),
        &ctx->pitch);

    // Render Each Tile
    uint16_t tileBase = 0x8000;
    uint16_t bgBase = 0x9800;
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
                                ctx->pixels[px + scalex + (py + scaley) * GB_VID_WIDTH * RENDER_SCALE] = 0xFF;
                            }
                            else if (color == 1)
                            {
                                ctx->pixels[px + scalex + (py + scaley) * GB_VID_WIDTH * RENDER_SCALE] = 0x3F3F3FFF;
                            }
                            else if (color == 2)
                            {
                                ctx->pixels[px + scalex + (py + scaley) * GB_VID_WIDTH * RENDER_SCALE] = 0x7E7E7EFF;
                            }
                            else if (color == 3)
                            {
                                ctx->pixels[px + scalex + (py + scaley) * GB_VID_WIDTH * RENDER_SCALE] = 0xFFFFFFFF;
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

        if (regDst == 7)
        {
            // If both src and dst are A do nothing
            if (regSrc != 7)
            {
                gb->A = gb->regs[regSrc];
            }
        }
        else
        {
            if (regSrc == 7)
            {
                gb->regs[regDst] = gb->A;
            }
            else
            {
                gb->regs[regDst] = gb->regs[regSrc];
            }
        }
    }
    break;

    // 16bit Load Commands
    case OP_LD_BC_NN:
    case OP_LD_DE_NN:
    case OP_LD_HL_NN:
    case OP_LD_SP_NN:
    {
        uint8_t reg = (opcode & 0xF0) >> 8;
        uint16_t val = FetchWord(gb);

        gb->regs[reg] = val;
    }
    break;

    // Jump Commands
    case OP_JP_NN:
    {
        gb->regs[REG_PC] = FetchWord(gb);
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

    gb->cart = LoadRom("tests/carts/cpu_instrs/individual/01-special.gb", &gb->cartSize);
    if (gb->cart == NULL)
    {
        return;
    }

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

    // TODO: Run boot up procedure instead of jumping to cartridge immediately
    gb->regs[REG_PC] = 0x0100;

    // memset(gb->mem, 0, sizeof(uint8_t) * 0x8000);

    // gb->mem[0x1800] = 0;

    // gb->mem[0] = 0x7C;
    // gb->mem[1] = 0x7C;
    // gb->mem[2] = 0x00;
    // gb->mem[3] = 0xC6;
    // gb->mem[4] = 0xC6;
    // gb->mem[5] = 0x00;
    // gb->mem[6] = 0x00;
    // gb->mem[7] = 0xFE;
    // gb->mem[8] = 0xC6;
    // gb->mem[9] = 0xC6;
    // gb->mem[0xa] = 0x00;
    // gb->mem[0xb] = 0xC6;
    // gb->mem[0xc] = 0xC6;
    // gb->mem[0xd] = 0x00;
    // gb->mem[0xe] = 0x00;
    // gb->mem[0xf] = 0x00;

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

        running = DoInstruction(gb);

        SimpleRender(gb, gb->ctx);
    }
}