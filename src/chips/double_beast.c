#include "runtime.h"

BN67_INCBIN(double_beast_icon, "build/double_beast_icon.bin");
BN67_INCBIN(double_beast_image, "build/double_beast_image.bin");
BN67_INCBIN(double_beast_palette, "build/double_beast_palette.bin");

/* The English chip record retained DoubleBeast's routine, but all three menu
 * art pointers were replaced by placeholders. Restore the complete Japanese
 * record, including its dedicated palette. */
BN67_PATCH_POINTER(0x0802533C, double_beast_icon);
BN67_PATCH_POINTER(0x08025340, double_beast_image);
BN67_PATCH_POINTER(0x08025344, double_beast_palette);
