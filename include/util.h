#pragma once

// Are bits set?
#define iss(src, bits) ((src) & (bits))
// Set bits
#define set(dst, bits) (dst |= (bits))
// Set bits with mask
#define sem(dst, mask, bits) (dst = (dst & ~(mask)) | ((bits) & (mask)))
// Clear bits
#define clr(dst, bits) (dst &= ~(bits))
