/*
basics:
http://en.wikipedia.org/wiki/Z-order_curve
http://stackoverflow.com/questions/18529057/produce-interleaving-bit-patterns-morton-keys-for-32-bit-64-bit-and-128bit
http://stackoverflow.com/questions/1024754/how-to-compute-a-3d-morton-number-interleave-the-bits-of-3-ints
http://code.activestate.com/recipes/577558-interleave-bits-aka-morton-ize-aka-z-order-curve/
http://graphics.stanford.edu/~seander/bithacks.html#InterleaveBMN
http://dmytry.com/texts/collision_detection_using_z_order_curve_aka_Morton_order.html
http://www.forceflow.be/2013/10/07/morton-encodingdecoding-through-bit-interleaving-implementations/
https://www.fpcomplete.com/user/edwardk/revisiting-matrix-multiplication/part-1
https://www.fpcomplete.com/user/edwardk/revisiting-matrix-multiplication/part-2
Converting to and from Dilated Integers [2007]:
http://www.cs.indiana.edu/~dswise/Arcee/castingDilated-comb.pdf

a very good post:
http://asgerhoedt.dk/?p=276

It turns out we don't have to actually interleave the bits
if all we want is the ability to compare two keys as if they had been interleaved.
What we need to know is where the most significant difference between them occurs.
Given two halves of a key we can exclusive or them together to find the positions at which they differ.
*/

#pragma once

// from http://www.forceflow.be/2012/07/24/out-of-core-construction-of-sparse-voxel-octrees/
// method to separate bits from a given integer 3 positions apart
inline UINT64 splitBy3(unsigned int a)
{
    uint64_t x = a & 0x1fffff; // we only look at the first 21 bits
    x = (x | x << 32) & 0x1f00000000ffff;  // shift left 32 bits, OR with self, and 00011111000000000000000000000000000000001111111111111111
    x = (x | x << 16) & 0x1f0000ff0000ff;  // shift left 32 bits, OR with self, and 00011111000000000000000011111111000000000000000011111111
    x = (x | x << 8) & 0x100f00f00f00f00f; // shift left 32 bits, OR with self, and 0001000000001111000000001111000000001111000000001111000000000000
    x = (x | x << 4) & 0x10c30c30c30c30c3; // shift left 32 bits, OR with self, and 0001000011000011000011000011000011000011000011000011000100000000
    x = (x | x << 2) & 0x1249249249249249;
    return x;
}
 
inline UINT64 mortonEncode_magicbits(unsigned int x, unsigned int y, unsigned int z){
    UINT64 answer = 0;
    answer |= splitBy3(x) | splitBy3(y) << 1 | splitBy3(z) << 2;
    return answer;
}

// from https://fgiesen.wordpress.com/2009/12/13/decoding-morton-codes/

// "Insert" a 0 bit after each of the 16 low bits of x
inline UINT32 Part1By1(UINT32 x)
{
  x &= 0x0000ffff;                  // x = ---- ---- ---- ---- fedc ba98 7654 3210
  x = (x ^ (x <<  8)) & 0x00ff00ff; // x = ---- ---- fedc ba98 ---- ---- 7654 3210
  x = (x ^ (x <<  4)) & 0x0f0f0f0f; // x = ---- fedc ---- ba98 ---- 7654 ---- 3210
  x = (x ^ (x <<  2)) & 0x33333333; // x = --fe --dc --ba --98 --76 --54 --32 --10
  x = (x ^ (x <<  1)) & 0x55555555; // x = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
  return x;
}

// "Insert" two 0 bits after each of the 10 low bits of x
inline UINT32 Part1By2(UINT32 x)
{
  x &= 0x000003ff;                  // x = ---- ---- ---- ---- ---- --98 7654 3210
  x = (x ^ (x << 16)) & 0xff0000ff; // x = ---- --98 ---- ---- ---- ---- 7654 3210
  x = (x ^ (x <<  8)) & 0x0300f00f; // x = ---- --98 ---- ---- 7654 ---- ---- 3210
  x = (x ^ (x <<  4)) & 0x030c30c3; // x = ---- --98 ---- 76-- --54 ---- 32-- --10
  x = (x ^ (x <<  2)) & 0x09249249; // x = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
  return x;
}

inline UINT32 PartBy4(UINT32 x) {
    x &= 0x0000007f;                  // x = ---- ---- ---- ---- ---- ---- -654 3210
    x = (x ^ (x << 16)) & 0x0070000F; // x = ---- ---- -654 ---- ---- ---- ---- 3210
    x = (x ^ (x <<  8)) & 0x40300C03; // x = -6-- ---- --54 ---- ---- 32-- ---- --10
    x = (x ^ (x <<  4)) & 0x42108421; // x = -6-- --5- ---4 ---- 3--- -2-- --1- ---0
    return x;
}

// Inverse of Part1By1 - "delete" all odd-indexed bits
inline UINT32 Compact1By1(UINT32 x)
{
  x &= 0x55555555;                  // x = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
  x = (x ^ (x >>  1)) & 0x33333333; // x = --fe --dc --ba --98 --76 --54 --32 --10
  x = (x ^ (x >>  2)) & 0x0f0f0f0f; // x = ---- fedc ---- ba98 ---- 7654 ---- 3210
  x = (x ^ (x >>  4)) & 0x00ff00ff; // x = ---- ---- fedc ba98 ---- ---- 7654 3210
  x = (x ^ (x >>  8)) & 0x0000ffff; // x = ---- ---- ---- ---- fedc ba98 7654 3210
  return x;
}

// Inverse of Part1By2 - "delete" all bits not at positions divisible by 3
inline UINT32 Compact1By2(UINT32 x)
{
  x &= 0x09249249;                  // x = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
  x = (x ^ (x >>  2)) & 0x030c30c3; // x = ---- --98 ---- 76-- --54 ---- 32-- --10
  x = (x ^ (x >>  4)) & 0x0300f00f; // x = ---- --98 ---- ---- 7654 ---- ---- 3210
  x = (x ^ (x >>  8)) & 0xff0000ff; // x = ---- --98 ---- ---- ---- ---- 7654 3210
  x = (x ^ (x >> 16)) & 0x000003ff; // x = ---- ---- ---- ---- ---- --98 7654 3210
  return x;
}

inline UINT32 EncodeMorton2(UINT32 x, UINT32 y)
{
  return (Part1By1(y) << 1) + Part1By1(x);
}

inline UINT32 EncodeMorton3(UINT32 x, UINT32 y, UINT32 z)
{
  return (Part1By2(z) << 2) + (Part1By2(y) << 1) + Part1By2(x);
}

inline UINT32 DecodeMorton2X(UINT32 code)
{
  return Compact1By1(code >> 0);
}

inline UINT32 DecodeMorton2Y(UINT32 code)
{
  return Compact1By1(code >> 1);
}

inline UINT32 DecodeMorton3X(UINT32 code)
{
  return Compact1By2(code >> 0);
}

inline UINT32 DecodeMorton3Y(UINT32 code)
{
  return Compact1By2(code >> 1);
}

inline UINT32 DecodeMorton3Z(UINT32 code)
{
  return Compact1By2(code >> 2);
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
