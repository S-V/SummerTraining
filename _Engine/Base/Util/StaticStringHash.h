/*
 * This source file is part of the CrunchyBytes Game Development Kit.
 *
 * For the latest information, see http://n00body.squarespace.com/
 *
 * Copyright (c) 2011 CrunchyBytes
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#pragma once


/** Generates integer IDs at compile time by hashing strings.

    Based on:
    http://www.humus.name/index.php?page=News&ID=296

    @remarks This class exploits two language details to get optimized out of
            the compiled code. Firstly, the use of array references lets the 
            compiler know the string's existence and size. Secondly, the lack
            of the @c explicit keyword allows a string literal to be converted 
            directly to a StringId.
            
    @author Joshua Ols <crunchy.bytes.blog@gmail.com>
    @date 2011-05-28
*/

inline UINT32 GetDynamicStringHash( const char* str )
{
	UINT32 hash = *str++;
	while( *str ) {
		hash = (hash * 65599u) + *str++;
	}
	return hash;
}
inline UINT32 GetDynamicStringHash( const char* _text, UINT32 _length )
{
	UINT32 hash = *_text++;
	UINT32 i = 1;
	while( i < _length ) {
		hash = (hash * 65599u) + *_text++;
		i++;
	}
	return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[2])
{
    return str[0];
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[3])
{
    UINT32 hash = str[0];
    hash = (hash * 65599u) + str[1];
    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[4])
{
    UINT32 hash = str[0];
    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[5])
{
    UINT32 hash = str[0];
    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[6])
{
    UINT32 hash = str[0];
    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[7])
{
    UINT32 hash = str[0];
    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[8])
{
    UINT32 hash = str[0];
    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[9])
{
    UINT32 hash = str[0];
    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[10])
{
    UINT32 hash = str[0];
    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[11])
{
    UINT32 hash = str[0];
    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[12])
{
    UINT32 hash = str[0];
    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[13])
{
    UINT32 hash = str[0];
    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[14])
{
    UINT32 hash = str[0];
    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[15])
{
    UINT32 hash = str[0];
    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[16])
{
    UINT32 hash = str[0];
    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    hash = (hash * 65599u) + str[14];
    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[17])
{
    UINT32 hash = str[0];
    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    hash = (hash * 65599u) + str[14];
    hash = (hash * 65599u) + str[15];
    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[18])
{
    UINT32 hash = str[0];
    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    hash = (hash * 65599u) + str[14];
    hash = (hash * 65599u) + str[15];
    hash = (hash * 65599u) + str[16];
    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[19])
{
    UINT32 hash = str[0];
    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    hash = (hash * 65599u) + str[14];
    hash = (hash * 65599u) + str[15];
    hash = (hash * 65599u) + str[16];
    hash = (hash * 65599u) + str[17];
    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[20])
{
    UINT32 hash = str[0];
    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    hash = (hash * 65599u) + str[14];
    hash = (hash * 65599u) + str[15];
    hash = (hash * 65599u) + str[16];
    hash = (hash * 65599u) + str[17];
    hash = (hash * 65599u) + str[18];
    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[21])
{
    UINT32 hash = str[0];
    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    hash = (hash * 65599u) + str[14];
    hash = (hash * 65599u) + str[15];
    hash = (hash * 65599u) + str[16];
    hash = (hash * 65599u) + str[17];
    hash = (hash * 65599u) + str[18];
    hash = (hash * 65599u) + str[19];
    return hash;
}


//#define HELPER_MACRO_HASH_COMBINE_1

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[22])
{
    UINT32 hash = str[0];

    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    hash = (hash * 65599u) + str[14];
    hash = (hash * 65599u) + str[15];
    hash = (hash * 65599u) + str[16];
    hash = (hash * 65599u) + str[17];
    hash = (hash * 65599u) + str[18];
    hash = (hash * 65599u) + str[19];
	hash = (hash * 65599u) + str[20];

    return hash;
}


mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[23])
{
    UINT32 hash = str[0];

    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    hash = (hash * 65599u) + str[14];
    hash = (hash * 65599u) + str[15];
    hash = (hash * 65599u) + str[16];
    hash = (hash * 65599u) + str[17];
    hash = (hash * 65599u) + str[18];
    hash = (hash * 65599u) + str[19];
	hash = (hash * 65599u) + str[20];
	hash = (hash * 65599u) + str[21];

    return hash;
}



mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[24])
{
    UINT32 hash = str[0];

    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    hash = (hash * 65599u) + str[14];
    hash = (hash * 65599u) + str[15];
    hash = (hash * 65599u) + str[16];
    hash = (hash * 65599u) + str[17];
    hash = (hash * 65599u) + str[18];
    hash = (hash * 65599u) + str[19];
	hash = (hash * 65599u) + str[20];
	hash = (hash * 65599u) + str[21];
	hash = (hash * 65599u) + str[22];

    return hash;
}


mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[25])
{
    UINT32 hash = str[0];

    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    hash = (hash * 65599u) + str[14];
    hash = (hash * 65599u) + str[15];
    hash = (hash * 65599u) + str[16];
    hash = (hash * 65599u) + str[17];
    hash = (hash * 65599u) + str[18];
    hash = (hash * 65599u) + str[19];
	hash = (hash * 65599u) + str[20];
	hash = (hash * 65599u) + str[21];
	hash = (hash * 65599u) + str[22];
	hash = (hash * 65599u) + str[23];

    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[26])
{
    UINT32 hash = str[0];

    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    hash = (hash * 65599u) + str[14];
    hash = (hash * 65599u) + str[15];
    hash = (hash * 65599u) + str[16];
    hash = (hash * 65599u) + str[17];
    hash = (hash * 65599u) + str[18];
    hash = (hash * 65599u) + str[19];
	hash = (hash * 65599u) + str[20];
	hash = (hash * 65599u) + str[21];
	hash = (hash * 65599u) + str[22];
	hash = (hash * 65599u) + str[23];
	hash = (hash * 65599u) + str[24];

    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[27])
{
    UINT32 hash = str[0];

    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    hash = (hash * 65599u) + str[14];
    hash = (hash * 65599u) + str[15];
    hash = (hash * 65599u) + str[16];
    hash = (hash * 65599u) + str[17];
    hash = (hash * 65599u) + str[18];
    hash = (hash * 65599u) + str[19];
	hash = (hash * 65599u) + str[20];
	hash = (hash * 65599u) + str[21];
	hash = (hash * 65599u) + str[22];
	hash = (hash * 65599u) + str[23];
	hash = (hash * 65599u) + str[24];
	hash = (hash * 65599u) + str[25];

    return hash;
}


mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[28])
{
    UINT32 hash = str[0];

    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    hash = (hash * 65599u) + str[14];
    hash = (hash * 65599u) + str[15];
    hash = (hash * 65599u) + str[16];
    hash = (hash * 65599u) + str[17];
    hash = (hash * 65599u) + str[18];
    hash = (hash * 65599u) + str[19];
	hash = (hash * 65599u) + str[20];
	hash = (hash * 65599u) + str[21];
	hash = (hash * 65599u) + str[22];
	hash = (hash * 65599u) + str[23];
	hash = (hash * 65599u) + str[24];
	hash = (hash * 65599u) + str[25];
	hash = (hash * 65599u) + str[26];

    return hash;
}


mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[29])
{
    UINT32 hash = str[0];

    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    hash = (hash * 65599u) + str[14];
    hash = (hash * 65599u) + str[15];
    hash = (hash * 65599u) + str[16];
    hash = (hash * 65599u) + str[17];
    hash = (hash * 65599u) + str[18];
    hash = (hash * 65599u) + str[19];
	hash = (hash * 65599u) + str[20];
	hash = (hash * 65599u) + str[21];
	hash = (hash * 65599u) + str[22];
	hash = (hash * 65599u) + str[23];
	hash = (hash * 65599u) + str[24];
	hash = (hash * 65599u) + str[25];
	hash = (hash * 65599u) + str[26];
	hash = (hash * 65599u) + str[27];

    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[30])
{
    UINT32 hash = str[0];

    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    hash = (hash * 65599u) + str[14];
    hash = (hash * 65599u) + str[15];
    hash = (hash * 65599u) + str[16];
    hash = (hash * 65599u) + str[17];
    hash = (hash * 65599u) + str[18];
    hash = (hash * 65599u) + str[19];
	hash = (hash * 65599u) + str[20];
	hash = (hash * 65599u) + str[21];
	hash = (hash * 65599u) + str[22];
	hash = (hash * 65599u) + str[23];
	hash = (hash * 65599u) + str[24];
	hash = (hash * 65599u) + str[25];
	hash = (hash * 65599u) + str[26];
	hash = (hash * 65599u) + str[27];
	hash = (hash * 65599u) + str[28];

    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[31])
{
    UINT32 hash = str[0];

    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    hash = (hash * 65599u) + str[14];
    hash = (hash * 65599u) + str[15];
    hash = (hash * 65599u) + str[16];
    hash = (hash * 65599u) + str[17];
    hash = (hash * 65599u) + str[18];
    hash = (hash * 65599u) + str[19];
	hash = (hash * 65599u) + str[20];
	hash = (hash * 65599u) + str[21];
	hash = (hash * 65599u) + str[22];
	hash = (hash * 65599u) + str[23];
	hash = (hash * 65599u) + str[24];
	hash = (hash * 65599u) + str[25];
	hash = (hash * 65599u) + str[26];
	hash = (hash * 65599u) + str[27];
	hash = (hash * 65599u) + str[28];
	hash = (hash * 65599u) + str[29];

    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[32])
{
    UINT32 hash = str[0];

    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    hash = (hash * 65599u) + str[14];
    hash = (hash * 65599u) + str[15];
    hash = (hash * 65599u) + str[16];
    hash = (hash * 65599u) + str[17];
    hash = (hash * 65599u) + str[18];
    hash = (hash * 65599u) + str[19];
	hash = (hash * 65599u) + str[20];
	hash = (hash * 65599u) + str[21];
	hash = (hash * 65599u) + str[22];
	hash = (hash * 65599u) + str[23];
	hash = (hash * 65599u) + str[24];
	hash = (hash * 65599u) + str[25];
	hash = (hash * 65599u) + str[26];
	hash = (hash * 65599u) + str[27];
	hash = (hash * 65599u) + str[28];
	hash = (hash * 65599u) + str[29];
	hash = (hash * 65599u) + str[30];

    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[33])
{
    UINT32 hash = str[0];

    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    hash = (hash * 65599u) + str[14];
    hash = (hash * 65599u) + str[15];
    hash = (hash * 65599u) + str[16];
    hash = (hash * 65599u) + str[17];
    hash = (hash * 65599u) + str[18];
    hash = (hash * 65599u) + str[19];
	hash = (hash * 65599u) + str[20];
	hash = (hash * 65599u) + str[21];
	hash = (hash * 65599u) + str[22];
	hash = (hash * 65599u) + str[23];
	hash = (hash * 65599u) + str[24];
	hash = (hash * 65599u) + str[25];
	hash = (hash * 65599u) + str[26];
	hash = (hash * 65599u) + str[27];
	hash = (hash * 65599u) + str[28];
	hash = (hash * 65599u) + str[29];
	hash = (hash * 65599u) + str[30];
	hash = (hash * 65599u) + str[31];

    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[34])
{
    UINT32 hash = str[0];

    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    hash = (hash * 65599u) + str[14];
    hash = (hash * 65599u) + str[15];
    hash = (hash * 65599u) + str[16];
    hash = (hash * 65599u) + str[17];
    hash = (hash * 65599u) + str[18];
    hash = (hash * 65599u) + str[19];
	hash = (hash * 65599u) + str[20];
	hash = (hash * 65599u) + str[21];
	hash = (hash * 65599u) + str[22];
	hash = (hash * 65599u) + str[23];
	hash = (hash * 65599u) + str[24];
	hash = (hash * 65599u) + str[25];
	hash = (hash * 65599u) + str[26];
	hash = (hash * 65599u) + str[27];
	hash = (hash * 65599u) + str[28];
	hash = (hash * 65599u) + str[29];
	hash = (hash * 65599u) + str[30];
	hash = (hash * 65599u) + str[31];
	hash = (hash * 65599u) + str[32];

    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[35])
{
    UINT32 hash = str[0];

    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    hash = (hash * 65599u) + str[14];
    hash = (hash * 65599u) + str[15];
    hash = (hash * 65599u) + str[16];
    hash = (hash * 65599u) + str[17];
    hash = (hash * 65599u) + str[18];
    hash = (hash * 65599u) + str[19];
	hash = (hash * 65599u) + str[20];
	hash = (hash * 65599u) + str[21];
	hash = (hash * 65599u) + str[22];
	hash = (hash * 65599u) + str[23];
	hash = (hash * 65599u) + str[24];
	hash = (hash * 65599u) + str[25];
	hash = (hash * 65599u) + str[26];
	hash = (hash * 65599u) + str[27];
	hash = (hash * 65599u) + str[28];
	hash = (hash * 65599u) + str[29];
	hash = (hash * 65599u) + str[30];
	hash = (hash * 65599u) + str[31];
	hash = (hash * 65599u) + str[32];
	hash = (hash * 65599u) + str[33];

    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[36])
{
    UINT32 hash = str[0];

    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    hash = (hash * 65599u) + str[14];
    hash = (hash * 65599u) + str[15];
    hash = (hash * 65599u) + str[16];
    hash = (hash * 65599u) + str[17];
    hash = (hash * 65599u) + str[18];
    hash = (hash * 65599u) + str[19];
	hash = (hash * 65599u) + str[20];
	hash = (hash * 65599u) + str[21];
	hash = (hash * 65599u) + str[22];
	hash = (hash * 65599u) + str[23];
	hash = (hash * 65599u) + str[24];
	hash = (hash * 65599u) + str[25];
	hash = (hash * 65599u) + str[26];
	hash = (hash * 65599u) + str[27];
	hash = (hash * 65599u) + str[28];
	hash = (hash * 65599u) + str[29];
	hash = (hash * 65599u) + str[30];
	hash = (hash * 65599u) + str[31];
	hash = (hash * 65599u) + str[32];
	hash = (hash * 65599u) + str[33];
	hash = (hash * 65599u) + str[34];

    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[37])
{
    UINT32 hash = str[0];

    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    hash = (hash * 65599u) + str[14];
    hash = (hash * 65599u) + str[15];
    hash = (hash * 65599u) + str[16];
    hash = (hash * 65599u) + str[17];
    hash = (hash * 65599u) + str[18];
    hash = (hash * 65599u) + str[19];
	hash = (hash * 65599u) + str[20];
	hash = (hash * 65599u) + str[21];
	hash = (hash * 65599u) + str[22];
	hash = (hash * 65599u) + str[23];
	hash = (hash * 65599u) + str[24];
	hash = (hash * 65599u) + str[25];
	hash = (hash * 65599u) + str[26];
	hash = (hash * 65599u) + str[27];
	hash = (hash * 65599u) + str[28];
	hash = (hash * 65599u) + str[29];
	hash = (hash * 65599u) + str[30];
	hash = (hash * 65599u) + str[31];
	hash = (hash * 65599u) + str[32];
	hash = (hash * 65599u) + str[33];
	hash = (hash * 65599u) + str[34];
	hash = (hash * 65599u) + str[35];

    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[38])
{
    UINT32 hash = str[0];

    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    hash = (hash * 65599u) + str[14];
    hash = (hash * 65599u) + str[15];
    hash = (hash * 65599u) + str[16];
    hash = (hash * 65599u) + str[17];
    hash = (hash * 65599u) + str[18];
    hash = (hash * 65599u) + str[19];
	hash = (hash * 65599u) + str[20];
	hash = (hash * 65599u) + str[21];
	hash = (hash * 65599u) + str[22];
	hash = (hash * 65599u) + str[23];
	hash = (hash * 65599u) + str[24];
	hash = (hash * 65599u) + str[25];
	hash = (hash * 65599u) + str[26];
	hash = (hash * 65599u) + str[27];
	hash = (hash * 65599u) + str[28];
	hash = (hash * 65599u) + str[29];
	hash = (hash * 65599u) + str[30];
	hash = (hash * 65599u) + str[31];
	hash = (hash * 65599u) + str[32];
	hash = (hash * 65599u) + str[33];
	hash = (hash * 65599u) + str[34];
	hash = (hash * 65599u) + str[35];
	hash = (hash * 65599u) + str[36];

    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[39])
{
    UINT32 hash = str[0];

    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    hash = (hash * 65599u) + str[14];
    hash = (hash * 65599u) + str[15];
    hash = (hash * 65599u) + str[16];
    hash = (hash * 65599u) + str[17];
    hash = (hash * 65599u) + str[18];
    hash = (hash * 65599u) + str[19];
	hash = (hash * 65599u) + str[20];
	hash = (hash * 65599u) + str[21];
	hash = (hash * 65599u) + str[22];
	hash = (hash * 65599u) + str[23];
	hash = (hash * 65599u) + str[24];
	hash = (hash * 65599u) + str[25];
	hash = (hash * 65599u) + str[26];
	hash = (hash * 65599u) + str[27];
	hash = (hash * 65599u) + str[28];
	hash = (hash * 65599u) + str[29];
	hash = (hash * 65599u) + str[30];
	hash = (hash * 65599u) + str[31];
	hash = (hash * 65599u) + str[32];
	hash = (hash * 65599u) + str[33];
	hash = (hash * 65599u) + str[34];
	hash = (hash * 65599u) + str[35];
	hash = (hash * 65599u) + str[36];
	hash = (hash * 65599u) + str[37];

    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[40])
{
    UINT32 hash = str[0];

    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    hash = (hash * 65599u) + str[14];
    hash = (hash * 65599u) + str[15];
    hash = (hash * 65599u) + str[16];
    hash = (hash * 65599u) + str[17];
    hash = (hash * 65599u) + str[18];
    hash = (hash * 65599u) + str[19];
	hash = (hash * 65599u) + str[20];
	hash = (hash * 65599u) + str[21];
	hash = (hash * 65599u) + str[22];
	hash = (hash * 65599u) + str[23];
	hash = (hash * 65599u) + str[24];
	hash = (hash * 65599u) + str[25];
	hash = (hash * 65599u) + str[26];
	hash = (hash * 65599u) + str[27];
	hash = (hash * 65599u) + str[28];
	hash = (hash * 65599u) + str[29];
	hash = (hash * 65599u) + str[30];
	hash = (hash * 65599u) + str[31];
	hash = (hash * 65599u) + str[32];
	hash = (hash * 65599u) + str[33];
	hash = (hash * 65599u) + str[34];
	hash = (hash * 65599u) + str[35];
	hash = (hash * 65599u) + str[36];
	hash = (hash * 65599u) + str[37];
	hash = (hash * 65599u) + str[38];

    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[41])
{
    UINT32 hash = str[0];

    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    hash = (hash * 65599u) + str[14];
    hash = (hash * 65599u) + str[15];
    hash = (hash * 65599u) + str[16];
    hash = (hash * 65599u) + str[17];
    hash = (hash * 65599u) + str[18];
    hash = (hash * 65599u) + str[19];
	hash = (hash * 65599u) + str[20];
	hash = (hash * 65599u) + str[21];
	hash = (hash * 65599u) + str[22];
	hash = (hash * 65599u) + str[23];
	hash = (hash * 65599u) + str[24];
	hash = (hash * 65599u) + str[25];
	hash = (hash * 65599u) + str[26];
	hash = (hash * 65599u) + str[27];
	hash = (hash * 65599u) + str[28];
	hash = (hash * 65599u) + str[29];
	hash = (hash * 65599u) + str[30];
	hash = (hash * 65599u) + str[31];
	hash = (hash * 65599u) + str[32];
	hash = (hash * 65599u) + str[33];
	hash = (hash * 65599u) + str[34];
	hash = (hash * 65599u) + str[35];
	hash = (hash * 65599u) + str[36];
	hash = (hash * 65599u) + str[37];
	hash = (hash * 65599u) + str[38];
	hash = (hash * 65599u) + str[39];

    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[42])
{
    UINT32 hash = str[0];

    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    hash = (hash * 65599u) + str[14];
    hash = (hash * 65599u) + str[15];
    hash = (hash * 65599u) + str[16];
    hash = (hash * 65599u) + str[17];
    hash = (hash * 65599u) + str[18];
    hash = (hash * 65599u) + str[19];
	hash = (hash * 65599u) + str[20];
	hash = (hash * 65599u) + str[21];
	hash = (hash * 65599u) + str[22];
	hash = (hash * 65599u) + str[23];
	hash = (hash * 65599u) + str[24];
	hash = (hash * 65599u) + str[25];
	hash = (hash * 65599u) + str[26];
	hash = (hash * 65599u) + str[27];
	hash = (hash * 65599u) + str[28];
	hash = (hash * 65599u) + str[29];
	hash = (hash * 65599u) + str[30];
	hash = (hash * 65599u) + str[31];
	hash = (hash * 65599u) + str[32];
	hash = (hash * 65599u) + str[33];
	hash = (hash * 65599u) + str[34];
	hash = (hash * 65599u) + str[35];
	hash = (hash * 65599u) + str[36];
	hash = (hash * 65599u) + str[37];
	hash = (hash * 65599u) + str[38];
	hash = (hash * 65599u) + str[39];
	hash = (hash * 65599u) + str[40];

    return hash;
}

mxFORCEINLINE
UINT32 GetStaticStringHash(
    const char (&str)[43])
{
    UINT32 hash = str[0];

    hash = (hash * 65599u) + str[1];
    hash = (hash * 65599u) + str[2];
    hash = (hash * 65599u) + str[3];
    hash = (hash * 65599u) + str[4];
    hash = (hash * 65599u) + str[5];
    hash = (hash * 65599u) + str[6];
    hash = (hash * 65599u) + str[7];
    hash = (hash * 65599u) + str[8];
    hash = (hash * 65599u) + str[9];
    hash = (hash * 65599u) + str[10];
    hash = (hash * 65599u) + str[11];
    hash = (hash * 65599u) + str[12];
    hash = (hash * 65599u) + str[13];
    hash = (hash * 65599u) + str[14];
    hash = (hash * 65599u) + str[15];
    hash = (hash * 65599u) + str[16];
    hash = (hash * 65599u) + str[17];
    hash = (hash * 65599u) + str[18];
    hash = (hash * 65599u) + str[19];
	hash = (hash * 65599u) + str[20];
	hash = (hash * 65599u) + str[21];
	hash = (hash * 65599u) + str[22];
	hash = (hash * 65599u) + str[23];
	hash = (hash * 65599u) + str[24];
	hash = (hash * 65599u) + str[25];
	hash = (hash * 65599u) + str[26];
	hash = (hash * 65599u) + str[27];
	hash = (hash * 65599u) + str[28];
	hash = (hash * 65599u) + str[29];
	hash = (hash * 65599u) + str[30];
	hash = (hash * 65599u) + str[31];
	hash = (hash * 65599u) + str[32];
	hash = (hash * 65599u) + str[33];
	hash = (hash * 65599u) + str[34];
	hash = (hash * 65599u) + str[35];
	hash = (hash * 65599u) + str[36];
	hash = (hash * 65599u) + str[37];
	hash = (hash * 65599u) + str[38];
	hash = (hash * 65599u) + str[39];
	hash = (hash * 65599u) + str[40];
	hash = (hash * 65599u) + str[41];

    return hash;
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
