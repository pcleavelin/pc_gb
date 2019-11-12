#include <stdio.h>
#include "GB.h"

int main(int argc, char **argv)
{
    printf("Hello, world!\n");

    GB *gb = CreateGB("");
    if (gb == NULL)
    {
        printf("Failed to create GameBoy\n");
        return 1;
    }

    StartGB(gb);

    DestroyGB(gb);
    gb = NULL;

    return 0;
}
