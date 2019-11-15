#include "GB.h"

#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Please provide a rom\n");
        return 0;
    }

    printf("%s\n", argv[1]);

    GB *gb = CreateGB("");
    if (gb == NULL)
    {
        printf("Failed to create GameBoy\n");
        return 1;
    }

    StartGB(gb, argv[1]);

    DestroyGB(gb);
    gb = NULL;

    return 0;
}
