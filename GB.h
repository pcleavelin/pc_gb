// GB.h - Main structure for the GameBoy

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#define GB_VID_WIDTH 160
#define GB_VID_HEIGHT 144

#define RENDER_SCALE 4

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

// Info from https://realboyemulator.files.wordpress.com/2013/01/gbcpuman.pdf
// Power Up Sequence (Found on Page 18)
// BC = $0013
// DE = $00D8
// HL = $014D
// Stack Pointer = $FFFE
// CPU Opcode information (Found on Page 65)

// Memory Info (Found on Page 8)
// RAM: 8K = uint8_t[0x8000]

typedef struct GBstruct
{
    RenderContext *ctx;

    // Info From http://problemkaputt.de/pandocs.htm#cpuregistersandflags

    // Hi & Lo Registers respectively
    uint8_t A, B, D, H;
    uint8_t F, C, E, L;

    // Flags
    // Bit 7 - Zero
    // Bit 6 - Add/Sub-Flag (BCD)
    // Bit 5 - Half Carry Flag (BCD)
    // Bit 4 - Carry
    // Bit 3->0 - Unused (always zero)

    // Stack Pointer & Program Counter
    uint16_t SP;
    uint16_t PC;

    // Main Memory
    uint8_t mem[0x8000];
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

    return 0;
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

void StartGB(GB *gb)
{
    if (gb == NULL)
    {
        return;
    }

    printf("GB Starting...\n");

    // memset(gb->mem, 0, sizeof(uint8_t) * 0x8000);

    gb->mem[0x1800] = 0;
    gb->mem[0x1801] = 1;

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
    }
}