#pragma once

#include <stdbool.h> // bool
#include <stddef.h> // size_t
#include <stdint.h> // [u]int(8|16|32|64)_t

#define CVT_HEX_MAX_DIGITS  16
/*  8 = roundUp(32bits/4) */
/* 16 = roundUp(64bits/4) */
#define CVT_UDEC_MAX_DIGITS 20
/* 10 = roundUp(32bits*3/10) */
/* 20 = roundUp(64bits*3/10) */
#define CVT_SDEC_MAX_DIGITS (1+CVT_UDEC_MAX_DIGITS)
#define CVT_BINARY_MAX_DIGITS 64

size_t cvt_ud2a(char* buffer, uint64_t number, uint8_t base, bool upperCase);
size_t cvt_hex (char* buffer, uint64_t number, bool upperCase);
size_t cvt_uDec(char* buffer, uint64_t number);
size_t cvt_sDec(char* buffer,  int64_t number);
size_t cvt_oct (char* buffer, uint64_t number);
size_t cvt_bin (char* buffer, uint64_t number);

// Warning: Don't forget to provide extra space for zero terminator.
// Note: Returned value does not include zero terminator.
size_t cvt_ud2a_z(char* buffer, uint64_t number, uint8_t base, bool upperCase);
size_t cvt_hex_z (char* buffer, uint64_t number, bool upperCase);
size_t cvt_uDec_z(char* buffer, uint64_t number);
size_t cvt_sDec_z(char* buffer,  int64_t number);
size_t cvt_oct_z (char* buffer, uint64_t number);
size_t cvt_bin_z (char* buffer, uint64_t number);

size_t cvt_u32tos(char* buffer, uint32_t number, uint8_t base, bool upperCase);
