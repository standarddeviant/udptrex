
#ifndef U32_BIT_TRICKS_HEADER_GUARD
#define U32_BIT_TRICKS_HEADER_GUARD


#include <stdint.h>


/* power-of-two convenience functions */
uint32_t u32_set_bits(uint32_t i);
uint32_t u32_msb(uint32_t x);
uint32_t u32_is_pow2(uint32_t i);
uint32_t u32_force_pow2(uint32_t i, uint32_t up);
uint32_t u32_set_bits(uint32_t i) {
     // Java: use int, and use >>> instead of >>. Or use Integer.bitCount()
     // C or C++: use uint32_t
     i = i - ((i >> 1) & 0x55555555);        // add pairs of bits
     i = (i & 0x33333333) + ((i >> 2) & 0x33333333);  // quads
     i = (i + (i >> 4)) & 0x0F0F0F0F;        // groups of 8
     return (i * 0x01010101) >> 24;          // horizontal sum of bytes
}


uint32_t u32_msb(uint32_t x) {
    static const uint32_t bval[] =
    { 0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4 };

    unsigned int base = 0;
    if (x & 0xFFFF0000) { base += 32/2; x >>= 32/2; }
    if (x & 0x0000FF00) { base += 32/4; x >>= 32/4; }
    if (x & 0x000000F0) { base += 32/8; x >>= 32/8; }
    return base + bval[x];
}


uint32_t u32_is_pow2(uint32_t i) {
    return (1 == u32_set_bits(i));
}


uint32_t u32_force_pow2(uint32_t i, uint32_t up) {
    if(u32_is_pow2(i)) return i;
    return (uint32_t) up ?
        1 << (u32_msb(i) + 1) :
        1 << (u32_msb(i) + 0) ;
}

#endif
