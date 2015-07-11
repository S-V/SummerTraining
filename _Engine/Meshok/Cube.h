// Cube-related constants.
#pragma once

#include <Meshok/Meshok.h>

/*
	   Z
	   |  /Y
	   | /
	   |/_____X
	 (0,0,0)

Face orientation (normal) - apply right-hand rule to the edge loop.

Vertex enumeration:
        6___________7
       /|           /
      / |          /|
     /  |         / |
    4------------5  |
    |   2________|__3
    |   /        |  /
    |  /         | /
    | /          |/
    0/___________1

or using locational (Morton) codes (X - lowest bit, Y - middle, Z - highest):

       110__________111
       /|           /
      / |          /|
     /  |         / |
  100-----------101 |
    |  010_______|__011
    |   /        |  /
    |  /         | /
    | /          |/
  000/___________001

(see Gray code, Hamming distance, De Bruijn sequence)

NOTE: two vertices are connected by an edge, if their indices differ by one and only one bit.

Cube edge enumeration (edges are split into 3 groups by axes X,Y,Z, numbered using right-hand rule):
        ______2______
       /|           /|
      5 |11        6 |
     /  |         /  |10
    |------3-----|   |
    |   |_____1__|___|
    |   /        |   /
   8|  4        9|  7
    | /          | /
    |/___________|/
           0

The indexing of cube faces:

Faces are numbered in the order -Z, -Y, -X, +X, +Y, +Z.
Face index - face normal:

0 (Bottom) is -Z,
1 (Front)  is -Y,
2 (Left)   is -X,
3 (Right)  is +X,
4 (Back)   is +Y,
5 (Top)    is +Z.

Cubes are unrolled in the following order,
looking along +Y direction (labelled '1'):
        _____
        | 5 |
    -----------------
    | 2 | 1 | 3 | 4 |
    -----------------
        | 0 |
        -----

           +----------+
           |+Y       5|
           | ^  +Z    |
           | |        |
           | +---->+X |
+----------+----------+----------+----------+
|+Z       2|+Z       1|+Z       3|+Z       4|
| ^  -X    | ^  -Y    | ^  +X    | ^  +Y    |
| |        | |        | |        | |        |
| +---->-Y | +---->+X | +---->+Y | +---->-X |
+----------+----------+----------+----------+
           |-Y       0|
           | ^  -Z    |
           | |        |
           | +---->+X |
           +----------+

Face Edges are numbered as follows:

                    Z
        2           ^
    ---------       |
    |       |       |
  3 |       | 1     Y----> X
    |       |
    ---------
        0
*/

// CUBE_EDGE[ cube_edge ] => { start_vertex, end_vertex }
// where cube_edge is [0..11] and (start_vertex, end_vertex is [0..7])
extern const UINT8 CUBE_EDGE[ 12 ][2];

// CUBE_VERTEX[ cube_face ] => { cube vertices [4] }
extern const UINT8 CUBE_VERTEX[6][4];

// CUBE_FACE_EDGE[ face_edge ] => { cube edges [4] }
// where face_edge is [0..3] and cube_edge is [0..11]
extern const UINT8 CUBE_FACE_EDGE[6][4];

// NEXT_POSITION[face][edge] gives the connected [face, edge]
// where face is [0..5] and edge is [0..3] (edge relative to face)
extern const UINT8 NEXT_POSITION[6][4][2];

/*
Marching Squares:

Vertices are numbered as follows:

    3-------2
    |       |
    |       |
    |       |
    0-------1

Edges are numbered as follows:
        2
    ---------
    |       |
  3 |       | 1
    |       |
    ---------
        0
*/
extern const int MS_EDGES[16][4];

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
