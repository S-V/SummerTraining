// Cube-related constants.
#include "stdafx.h"
#pragma hdrstop
#include <Meshok/Cube.h>

// CUBE_EDGE[ cube_edge ] => { start_vertex, end_vertex }
const UINT8 CUBE_EDGE[ 12 ][2] =
{
	{ 0, 1 }, { 2, 3 }, { 6, 7 }, { 4, 5 },	// X-axis
	{ 0, 2 }, { 4, 6 }, { 5, 7 }, { 1, 3 },	// Y-axis
	{ 0, 4 }, { 1, 5 }, { 3, 7 }, { 2, 6 },	// Z-axis
};

// CUBE_VERTEX[ cube_face ] => { cube vertices [4] }
const UINT8 CUBE_VERTEX[6][4] =
{
	// cube vertices
    {  2,   3,   1,   0 }, // face 0
    {  0,   1,   5,   4 }, // face 1
    {  2,   0,   4,   6 }, // face 2
    {  1,   3,   7,   5 }, // face 3
    {  3,   2,   6,   7 }, // face 4
    {  4,   5,   7,   6 }, // face 5
};

// CUBE_FACE_EDGE[ face_edge ] => { cube edges [4] }
const UINT8 CUBE_FACE_EDGE[6][4] =
{
	//edge0, edge1, edge2, edge3
    {  1,   7,   0,   4 }, // face 0
    {  0,   9,   3,   8 }, // face 1
    {  4,   8,   5,  11 }, // face 2
    {  7,  10,   6,   9 }, // face 3
    {  1,  11,   2,  10 }, // face 4
    {  3,   6,   2,   5 }, // face 5
};

// Given a face and a face edge, this table returns the next face and face edge,
// which are actually the same edge in the combined cube.
// NEXT_POSITION[face][edge] gives the connected [face, edge]
const UINT8 NEXT_POSITION[6][4][2] =
{
	// edge0 | edge1 | edge2 | edge3
    {{4, 0}, {3, 0}, {1, 0}, {2, 0}}, // face 0
    {{0, 2}, {3, 3}, {5, 0}, {2, 1}}, // face 1
    {{0, 3}, {1, 3}, {5, 3}, {4, 1}}, // face 2
    {{0, 1}, {4, 3}, {5, 1}, {1, 1}}, // face 3
    {{0, 0}, {2, 3}, {5, 2}, {3, 1}}, // face 4
    {{1, 2}, {3, 2}, {4, 2}, {2, 2}}, // face 5
};

// http://en.wikipedia.org/wiki/Marching_squares

// MS_EDGES[ vertex_mask ] => intesecting_edges [0..2]
const int MS_EDGES[16][4] =
{
	// case | {active edges}
	{ -1, -1, -1, -1 }, //0x0 0000 Nothing intersects (fully outside)
	{  0,  3, -1, -1 }, //0x1 0001 0-3
	{  1,  0, -1, -1 }, //0x2 0010 1-0
	{  1,  3, -1, -1 }, //0x3 0011 1-3
	{  2,  1, -1, -1 }, //0x4 0100 2-1
#if 1
	{  0,  1,  2,  3 }, //0x5 0101 0-1, 2-3 (Ambiguous case)
#else
	{  0,  3,  2,  1 }, //0x5 0101 0-3, 2-1 (Ambiguous case)
#endif
	{  2,  0, -1, -1 }, //0x6 0110 2-0
	{  2,  3, -1, -1 }, //0x7 0111 2-3
	{  3,  2, -1, -1 }, //0x8 1000 3-2
	{  0,  2, -1, -1 }, //0x9 1001 0-2
#if 1
	{  1,  2,  3,  0 }, //0xA 1010 1-2, 3-0 (Ambiguous case)
#else
	{  1,  0,  3,  2 }, //0xA 1010 0-1, 2-3 (Ambiguous case)
#endif
	{  1,  2, -1, -1 }, //0xB 1011 1-2
	{  3,  1, -1, -1 }, //0xC 1100 3-1
	{  0,  1, -1, -1 }, //0xD 1101 0-1
	{  3,  0, -1, -1 }, //0xE 1110 3-0
	{ -1, -1, -1, -1 }, //0xF 1111 Nothing intersects (fully inside)
};
//const UINT8 MS_EDGE_MASK[16] =
//{
//	// case | edge_mask
//	0x0, //0 0000 Nothing intersects (fully outside)
//	0x9, //1 0001 0-3
//	0x3, //2 0010 1-0
//	0xA, //3 0011 1-3
//
//	0x6, //4 0100 2-1
//	0xF, //5 0101 0-1, 2-3
//	0x5, //6 0110 2-0
//	0xC, //7 0111 2-3
//
//	0xC, //8 1000 3-2
//	0x5, //9 1001 0-2
//	0xF, //A 1010 1-2, 3-0
//	0x6, //B 1011 1-2
//
//	0xA, //C 1100 3-1
//	0x3, //D 1101 0-1
//	0x9, //E 1110 3-0
//	0x0  //F 1111 Nothing intersects (fully inside)
//};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
