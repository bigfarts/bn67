#include "runtime.h"

BN67_INCBIN(doublebeast_icon, "build/doublebeast-icon.bin");
BN67_INCBIN(doublebeast_image, "build/doublebeast-image.bin");
BN67_INCBIN(doublebeast_palette, "build/doublebeast-palette.bin");

/* The English chip record retained DoubleBeast's routine, but all three menu
 * art pointers were replaced by placeholders. Restore the complete Japanese
 * record, including its dedicated palette. */
BN67_PATCH_POINTER(0x0802533C, doublebeast_icon);
BN67_PATCH_POINTER(0x08025340, doublebeast_image);
BN67_PATCH_POINTER(0x08025344, doublebeast_palette);
