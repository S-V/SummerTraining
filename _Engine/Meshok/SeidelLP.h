#pragma once

/*
=======================================================================
	
		Linear programming.

=======================================================================
*/

// status from linprog
enum LPStatus
{
	LP_Minimum,		// minimum attained
	LP_Infeasible,	// no feasible solution
	LP_Unbounded,	// region is unbounded
	LP_Ambiguous	// region is bounded by plane orthogonal to minimization vector
};

struct LPInput
{
	const Float4* planes;	// constraints
	int num_planes;
};
struct LPOutput
{
	Float3 solution;
	LPStatus status;
};

ERet SolveLP( const LPInput& input, LPOutput &output );

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
