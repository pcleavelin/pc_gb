// GB.h - Main structure for the GameBoy

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <SDL2/SDL.h>

#define GB_VID_WIDTH 160
#define GB_VID_HEIGHT 144

// Info from https://realboyemulator.files.wordpress.com/2013/01/gbcpuman.pdf
// Power Up Sequence (Found on Page 18)
// BC = $0013
// DE = $00D8
// HL = $014D
// Stack Pointer = $FFFE
// CPU Opcode information (Found on Page 65)

// Memory Info (Found on Page 8)
// RAM: 8K = uint8_t[0x8000]

typedef struct RenderContextstruct
{
    SDL_Window *window;
    SDL_Renderer *renderer;

    SDL_Texture *backbufferTexture;

    uint32_t *pixels;
    int pitch;
} RenderContext;

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
        GB_VID_WIDTH,
        GB_VID_HEIGHT,
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
        GB_VID_WIDTH,
        GB_VID_HEIGHT);
    if (ctx->backbufferTexture == NULL)
    {
        DestroyGBRenderContext(ctx);
        return NULL;
    }

    ctx->pixels = malloc(sizeof(uint32_t) * GB_VID_WIDTH * GB_VID_HEIGHT);
    if (ctx->pixels == NULL)
    {
        DestroyGBRenderContext(ctx);
        return NULL;
    }
    ctx->pitch = sizeof(uint32_t) * GB_VID_WIDTH;

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

void StartGB(GB *gb)
{
    if (gb == NULL)
    {
        return;
    }

    printf("GB Starting...\n");

    SDL_LockTexture(
        gb->ctx->backbufferTexture,
        NULL,
        (void **)(&gb->ctx->pixels),
        &gb->ctx->pitch);

    for (int i = 0; i < GB_VID_WIDTH; ++i)
    {
        for (int j = 0; j < GB_VID_HEIGHT; ++j)
        {
            gb->ctx->pixels[i + j * GB_VID_WIDTH] = 0xFF00FFFF;
        }
    }

    SDL_UnlockTexture(gb->ctx->backbufferTexture);

    SDL_RenderCopy(gb->ctx->renderer, gb->ctx->backbufferTexture, NULL, NULL);
    SDL_RenderPresent(gb->ctx->renderer);

    SDL_Delay(2000);
}