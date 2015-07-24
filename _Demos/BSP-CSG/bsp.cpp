/*
Useful references:
BSP Tips
October 30, 2000
by Charles Bloom, cbloom@cbloom.com
http://www.cbloom.com/3d/techdocs/bsp_tips.txt

source code:
CSGTOOL is a library, Ruby Gem and command line tool for performing Constructive Solid Geometry operations on STL Files using 3D BSP Trees.
https://github.com/sshirokov/csgtool

Papers:
Efficient Boundary Extraction of BSP Solids Based on Clipping Operations

http://www.me.mtu.edu/~rmdsouza/CSG_BSP.html
*/
#include <Base/Base.h>
#include <Core/VectorMath.h>
#include "bsp.h"

namespace BSP
{

#define UnpackUV( UV )	UV
#define PackUV( UV )	UV

mxBEGIN_STRUCT(Vertex)
	mxMEMBER_FIELD( xyz ),
	mxMEMBER_FIELD( N ),
	mxMEMBER_FIELD( T ),
	mxMEMBER_FIELD( UV ),
	mxMEMBER_FIELD( c ),
mxEND_REFLECTION;

bgfx::VertexDecl Vertex::ms_decl;

#define THICK_PLANE_SIDE_EPSILON	(0.25f)
#define NORMAL_PLANE_SIDE_EPSILON	(0.1f)
#define PRECISE_PLANE_SIDE_EPSILON	(0.01f)

/*
-----------------------------------------------------------------------------
	SBspStats
-----------------------------------------------------------------------------
*/
BspStats::BspStats()
{
	this->Reset();
}
void BspStats::Reset()
{
	mxZERO_OUT( *this );
}
void BspStats::Print( UINT32 elapsedTimeMSec )
{
	DBGOUT( "=== BSP statistics ========"			);
	DBGOUT( "Num. Polys(Begin): %u", m_polysBefore	 );
	DBGOUT( "Num. Polys(End):   %u", m_polysAfter		 );
	DBGOUT( "Num. Splits:       %u", m_numSplits			);
	DBGOUT( "Num. Planes:       %u", m_numPlanes );
	DBGOUT( "Num. Inner Nodes:  %u", m_numInternalNodes );
	DBGOUT( "Num. Solid Leaves: %u", m_numSolidLeaves	 );
	DBGOUT( "Num. Empty Leaves: %u", m_numEmptyLeaves	 );
	//DBGOUT( "Tree Depth:        %u", depth			 );
	DBGOUT( "Memory used:		%u KiB", m_bytesAllocated / 1024 );
	DBGOUT( "Time elapsed:      %u msec", elapsedTimeMSec		);
	DBGOUT( "==== End ===================="			 );
}

mxDEFINE_CLASS(Node);
mxBEGIN_REFLECTION(Node)
	mxMEMBER_FIELD( plane ),
	mxMEMBER_FIELD( front ),
	mxMEMBER_FIELD( back ),
	mxMEMBER_FIELD( faces ),
mxEND_REFLECTION;

mxDEFINE_CLASS(Face);
mxBEGIN_REFLECTION(Face)
	mxMEMBER_FIELD( vertices ),
	mxMEMBER_FIELD( next ),
mxEND_REFLECTION;

/*
-----------------------------------------------------------------------------
	Tree
-----------------------------------------------------------------------------
*/
mxDEFINE_CLASS(Tree);
mxBEGIN_REFLECTION(Tree)
	mxMEMBER_FIELD( m_planes ),	
	mxMEMBER_FIELD( m_nodes ),
	mxMEMBER_FIELD( m_faces ),
mxEND_REFLECTION;
Tree::Tree()
{
}

enum {
	MAX_TRACE_PLANES = 32,
	MAX_VERTS_IN_POLY = 32,
};

typedef TStaticList<Vector4,MAX_TRACE_PLANES>	PlaneStack;

// keep 1/8 unit away to keep the position valid before network snapping
// and to avoid various numeric issues
//#define	SURFACE_CLIP_EPSILON	(0.125)
#define SURFACE_CLIP_EPSILON	(1/32.0f)

// relation of a polygon to some splitting plane
// used to classify polygons when building a BSP tree
//
enum EPolyStatus
{
	Poly_Front,	// The polygon is lying in front of the plane.
	Poly_Back,	// The polygon is lying in back of the plane.
	Poly_Split,	// The polygon intersects with the plane.
	Poly_Coplanar,	// The polygon is lying on the plane.
};

static inline
EPlaneSide CalculatePlaneSide(
							  const Vector4& plane,
							  const Float3& point,
							  const float epsilon
							  )
{
	const float distance = Plane_PointDistance( plane, point );
	if ( distance > epsilon ) {
		return PLANESIDE_FRONT;
	} else if ( distance < -epsilon ) {
		return PLANESIDE_BACK;
	} else {
		return PLANESIDE_ON;
	}
}

EPlaneType CalculatePlaneType( const Vector4& plane )
{
	const Float3& N = Plane_CalculateNormal(plane);

	if ( N[0] == 0.0f ) {
		if ( N[1] == 0.0f ) {
			return N[2] > 0.0f ? EPlaneType::PLANETYPE_Z : EPlaneType::PLANETYPE_NEGZ;
		}
		else if ( N[2] == 0.0f ) {
			return N[1] > 0.0f ? EPlaneType::PLANETYPE_Y : EPlaneType::PLANETYPE_NEGY;
		}
		else {
			return EPlaneType::PLANETYPE_ZEROX;
		}
	}
	else if ( N[1] == 0.0f ) {
		if ( N[2] == 0.0f ) {
			return N[0] > 0.0f ? EPlaneType::PLANETYPE_X : EPlaneType::PLANETYPE_NEGX;
		}
		else {
			return EPlaneType::PLANETYPE_ZEROY;
		}
	}
	else if ( N[2] == 0.0f ) {
		return EPlaneType::PLANETYPE_ZEROZ;
	}
	else {
		return EPlaneType::PLANETYPE_NONAXIAL;
	}
}

Float3 GetPolygonCenter( const Face& polygon )
{
	Float3 center = Float3_Zero();
	int numVertices = polygon.vertices.Num();
	for( int i = 0; i < numVertices; i++ ) {
		center += polygon.vertices[i].xyz;
	}
	center *= ( 1.0f / numVertices );
	return center;
}

Vector4 PlaneFromPoints( const Float3& a, const Float3& b, const Float3& c )
{
	// right-hand rule - counterclockwise == front-facing
	Float3 normal = Float3_Cross( b - a, c - a );
	normal = Float3_Normalized( normal );
	float distance = -Float3_Dot( normal, b );
	return Vector4_Set( normal, distance );
}

Vector4 PlaneFromPolygon( const Face& polygon )
{
	mxASSERT(polygon.vertices.Num() >= 3);
	const Vertex& a = polygon.vertices[0];
	const Vertex& b = polygon.vertices[1];
	const Vertex& c = polygon.vertices[2];
	return PlaneFromPoints( a.xyz, b.xyz, c.xyz );
}

inline bool Planes_Equal( const Vector4 &a, const Vector4 &b ) {
	return Float4_Equal( a, b );
}

inline bool Planes_Equal( const Vector4 &a, const Vector4 &b, const float epsilon ) {
	return Float4_Equal( a, b, epsilon );
}

inline bool Planes_Equal( const Vector4 &a, const Vector4 &b, const float normalEps, const float distEps )
{
	if( fabs( a.w - b.w ) > distEps ) {
		return false;
	}
	Float3 normalA = Plane_CalculateNormal(a);
	Float3 normalB = Plane_CalculateNormal(b);
	if( !Float3_Equal(normalA,normalB,normalEps) ) {
		return false;
	}
	return true;
}

static
EPolyStatus ClassifyPolygon(
							const Vector4& plane,
							const Vertex* verts, const UINT32 numVerts,
							const float epsilon = 0.013f )
{
	UINT32	numPointsInFront = 0;
	UINT32	numPointsInBack = 0;
	UINT32	numPointsOnPlane = 0;

	for( UINT32 iVertex = 0; iVertex < numVerts; iVertex++ )
	{
		const Float3& point = verts[ iVertex ].xyz;

		const EPlaneSide ePlaneSide = CalculatePlaneSide( plane, point, epsilon );

		if( ePlaneSide == EPlaneSide::PLANESIDE_FRONT )
		{
			numPointsInFront++;
		}
		if( ePlaneSide == EPlaneSide::PLANESIDE_BACK )
		{
			numPointsInBack++;
		}
		if( ePlaneSide == EPlaneSide::PLANESIDE_ON )
		{
			numPointsOnPlane++;
		}
	}

	if( numVerts == numPointsInFront ) {
		return Poly_Front;
	}
	if( numVerts == numPointsInBack ) {
		return Poly_Back;
	}
	if( numVerts == numPointsOnPlane ) {
		return Poly_Coplanar;
	}
	return Poly_Split;
}

#if 0
/*
Sutherland-Hodgman clipping algorithm:
- For each clip plane:
-  Traverse each line segment in polygon:
-  Test against the plane
- Add a vertex if line crosses the plane
See: Splitting a polygon with a plane
http://ezekiel.vancouver.wsu.edu/~cs442/archive/lectures/bsp-trees/poly-split.pdf
*/
EPlaneSide SplitConvexPolygonByPlane(
							   const Poly& polygon,
							   Poly &front,
							   Poly &back,
							   const Vector4 &plane,
							   const float epsilon
							   )
{
	const UINT32 numPoints = polygon.vertices.Num();
	const Float3* points = polygon.vertices.ToPtr();

	float	dists[MAX_VERTS_IN_POLY+4];
	UINT8	sides[MAX_VERTS_IN_POLY+4];

	UINT32		counts[3];
	counts[PLANESIDE_FRONT] = 0;
	counts[PLANESIDE_BACK] = 0;
	counts[PLANESIDE_ON] = 0;

	for( UINT32 i = 0; i < numPoints; i++ )
	{
		float dist = Plane_PointDistance( plane, points[i] );
		if ( dist > epsilon ) {
			sides[i] = PLANESIDE_FRONT;
		} else if ( dist < -epsilon ) {
			sides[i] = PLANESIDE_BACK;
		} else {
			sides[i] = PLANESIDE_ON;
		}
		counts[sides[i]]++;
	}

	if( !counts[PLANESIDE_FRONT] && !counts[PLANESIDE_BACK] ) {
		return PLANESIDE_ON;
	}
	if( !counts[PLANESIDE_FRONT] ) {
		back = polygon;
		return PLANESIDE_BACK;
	}
	if( !counts[PLANESIDE_BACK] ) {
		front = polygon;
		return PLANESIDE_FRONT;
	}

	for( UINT32 i = 0; i < numPoints; i++ )
	{
		// Consider the edge from this vertex to the next vertex in the list, which defines a segment
		UINT32 iPointA = i;
		UINT32 iPointB = (i + 1) % numPoints;
		Float3 pointA = points[iPointA];
		Float3 pointB = points[iPointB];
		UINT32 sideA = sides[iPointA];
		UINT32 sideB = sides[iPointB];
		float distA = dists[iPointA];
		float distB = dists[iPointB];

		// If the segment crosses the plane, make a new vertex where the segment intersects the plane.
		if( sideA == PLANESIDE_BACK && sideB == PLANESIDE_FRONT ) {
			// generate a split point
			float t = distB / (distB - distA);
			Float3 mid = pointA + (pointB - pointA) * t;
			front.vertices.Add( mid );
			back.vertices.Add( mid );
		}
		if( sideB == PLANESIDE_BACK && sideA == PLANESIDE_FRONT ) {
			float t = distA / (distA - distB);
			Float3 mid = pointB + (pointA - pointB) * t;
			front.vertices.Add( mid );
			back.vertices.Add( mid );
		}

		if( sideB == PLANESIDE_BACK ) {
			back.vertices.Add( pointB );
		} else if( sideB == PLANESIDE_FRONT ) {
			front.vertices.Add( pointB );
		} else {
			back.vertices.Add( pointB );
			front.vertices.Add( pointB );
		}
	}

	return PLANESIDE_CROSS;
}
#endif

mxSWIPED("Doom 3 BFG Edition GPL Source Code, idSoftware");
EPlaneSide SplitConvexPolygonByPlane(
									 const Face& polygon,
									 Face &front,	// valid only if the polygon was split
									 Face &back,		// valid only if the polygon was split
									 const Vector4& plane,
									 const float epsilon
							   )
{
	const int numPoints = polygon.vertices.Num();
	const Vertex* p = polygon.vertices.ToPtr();

	float *	dists = (float *) _alloca( (numPoints+4) * sizeof( float ) );
	BYTE *	sides = (BYTE *) _alloca( (numPoints+4) * sizeof( BYTE ) );

	int		counts[3]; // 'front', 'back' and 'on'
	counts[0] = counts[1] = counts[2] = 0;

	// determine sides for each point
	int k;
	for ( k = 0; k < numPoints; k++ ) {
		float dist = Plane_PointDistance( plane, p[k].xyz );
		dists[k] = dist;
		if ( dist > epsilon ) {
			sides[k] = PLANESIDE_FRONT;
		} else if ( dist < -epsilon ) {
			sides[k] = PLANESIDE_BACK;
		} else {
			sides[k] = PLANESIDE_ON;
		}
		counts[sides[k]]++;
	}
	sides[k] = sides[0];
	dists[k] = dists[0];

	const Float3& planeNormal = Vector4_As_Float3( plane );
	const float planeDistance = -plane.w;

	// if the polygon is aligned with the splitting plane
	if ( counts[PLANESIDE_ON] == numPoints ) {
		return PLANESIDE_ON;
	}
	/** /
	// if coplanar, put on the front side if the normals match
	if ( !counts[PLANESIDE_FRONT] && !counts[PLANESIDE_BACK] )
	{
		const Vector4 polygonPlane = PlaneFromPolygon( polygon );
		const Float3& polygonNormal = Vector4_As_Float3( polygonPlane );
		if ( polygonNormal * planeNormal > 0.0f ) {
			//front = polygon;
			return PLANESIDE_FRONT;
		} else {
			//back = polygon;
			return PLANESIDE_BACK;
		}
	}
	/**/
	// if nothing at the front of the clipping plane
	if ( !counts[PLANESIDE_FRONT] ) {
		//back = polygon;
		return PLANESIDE_BACK;	// the polygon is completely behind the splitting plane
	}
	// if nothing at the back of the clipping plane
	if ( !counts[PLANESIDE_BACK] ) {
		//front = polygon;
		return PLANESIDE_FRONT;	// the polygon is completely in front of the splitting plane
	}

	// Straddles the splitting plane - we must clip.

	// Estimate the maximum number of points.
	const int maxpts = numPoints+4;	// cant use counts[0]+2 because of fp grouping errors

	for ( int i = 0; i < numPoints; i++)
	{
		const Vertex* p1 = &p[i];

		if ( sides[i] == PLANESIDE_ON ) {
			front.vertices.Add( *p1 );
			back.vertices.Add( *p1 );
			continue;
		}

		if ( sides[i] == PLANESIDE_FRONT ) {
			front.vertices.Add( *p1 );
		}

		if ( sides[i] == PLANESIDE_BACK ) {
			back.vertices.Add( *p1 );
		}

		if ( sides[i+1] == PLANESIDE_ON || sides[i+1] == sides[i] ) {
			continue;
		}

		// generate a split point
		const Vertex* p2 = &p[(i+1)%numPoints];

		// always calculate the split going from the same side
		// or minor epsilon issues can happen
		Vertex mid;
		if ( sides[i] == PLANESIDE_FRONT ) {
			float dot = dists[i] / ( dists[i] - dists[i+1] );
			for ( int j = 0; j < 3; j++ ) {
				// avoid round off error when possible
				if ( planeNormal[j] == 1.0f ) {
					mid.xyz[j] = planeDistance;
				} else if ( planeNormal[j] == -1.0f ) {
					mid.xyz[j] = -planeDistance;
				} else {
					mid.xyz[j] = (*p1).xyz[j] + dot * ( (*p2).xyz[j] - (*p1).xyz[j] );
				}
			}
			const Float2 p1st = UnpackUV( p1->UV );
			const Float2 p2st = UnpackUV( p2->UV );
			mid.UV = PackUV( p1st + ( p2st - p1st ) * dot );
		} else {
			float dot = dists[i+1] / ( dists[i+1] - dists[i] );
			for ( int j = 0; j < 3; j++ ) {	
				// avoid round off error when possible
				if ( planeNormal[j] == 1.0f ) {
					mid.xyz[j] = planeDistance;
				} else if ( planeNormal[j] == -1.0f ) {
					mid.xyz[j] = -planeDistance;
				} else {
					mid.xyz[j] = (*p2).xyz[j] + dot * ( (*p1).xyz[j] - (*p2).xyz[j] );
				}
			}
			const Float2 p1st = UnpackUV( p1->UV );
			const Float2 p2st = UnpackUV( p2->UV );
			mid.UV = PackUV( p2st + ( p1st - p2st ) * dot );
		}

		front.vertices.Add( mid );
		back.vertices.Add( mid );
	}

	if ( front.vertices.Num() > maxpts || back.vertices.Num() > maxpts ) {
		ptERROR( "SplitConvexPolygonByPlane: points exceeded estimate." );
	}

	return PLANESIDE_CROSS;
}

/*
The selection of the base polygon and the partitioning plane
is the crucial part of the BSP-tree construction. Depend-
ing on criteria for the base polygon selection different BSP-
trees can be obtained. In our work we use two different ap-
proaches: "naive" selection, where the polygon is randomly
selected from the polygon list, and "optimized" selection.
The optimization means using selection criteria that allow
for obtaining a tree with the following properties:
- Minimization of polygon splitting operations to reduce
the total number of nodes and the number of operations
in the function evaluation
- Minimization of computational errors during the function
evaluation and BSP-tree construction;
- Balancing the BSP tree, i.e., minimization of difference
between positive and negative list for the minimization of
the depth of the tree.
*/
// function for picking an optimal partitioning plane.
// we have two conflicting goals:
// 1) keep the tree balanced
// 2) avoid splitting the polygons
// and avoid introducing new partitioning planes
//
static
FaceID FindBestSplitterIndex( const Tree& tree, const FaceID polygons,
							 const SplittingCriteria& options = SplittingCriteria() )
{
	mxASSERT(polygons != NIL_INDEX);

	INT		numFrontFaces = 0;
	INT		numBackFaces = 0;
	INT		numSplitFaces = 0;
	INT		numCoplanarFaces = 0;

	FaceID	bestSplitter = 0;
	float	bestScore = 1e6f;	// the less value the better

	FaceID iPolyA = polygons;

	while( iPolyA != NIL_INDEX )
	{
		// select potential splitter
		const Face& polygonA = tree.m_faces[ iPolyA ];

		// potential splitting plane
		const Vector4 planeA = PlaneFromPolygon( polygonA );

		// test other polygons against the potential splitter
		FaceID iPolyB = polygons;
		while( iPolyB != NIL_INDEX )
		{
			const Face& polygonB = tree.m_faces[ iPolyB ];
			if( iPolyA != iPolyB )
			{
				// evaluate heuristic cost and select the best candidate

				const EPolyStatus planeSide = ClassifyPolygon(
					planeA,
					polygonB.vertices.ToPtr(), polygonB.vertices.Num(),
					options.planeEpsilon
				);

				switch( planeSide )
				{
				case Poly_Front :		numFrontFaces++;	break;
				case Poly_Back :		numBackFaces++;		break;
				case Poly_Coplanar :	numCoplanarFaces++;	break;
				case Poly_Split :		numSplitFaces++;	break;
				default:				mxUNREACHABLE;
				}

				// diff == 0 => tree is perfectly balanced
				const UINT32 diff = Abs<INT>( numFrontFaces - numBackFaces );

				FLOAT32 score = (diff * options.balanceVsCuts)
					+ (numSplitFaces * options.splitCost) * (1.0f - options.balanceVsCuts)
					;

				if( CalculatePlaneType(planeA) < PLANETYPE_TRUEAXIAL )
				{
					score *= 0.8f;	// axial is better
				}

				// A smaller score will yield a better tree.
				if( score < bestScore )
				{
					bestScore = score;
					bestSplitter = iPolyA;
				}
			}
			iPolyB = polygonB.next;
		}//for all tested polygons
		iPolyA = polygonA.next;
	}//for all potential splitters

	mxASSERT(bestSplitter != NIL_INDEX);
	return bestSplitter;
}

#define	NORMAL_EPSILON		0.00001f
#define	DIST_EPSILON		0.01f

static UINT32 GetPlaneIndex(
				   Tree & tree, const Vector4& plane,
				   const float normalEps = NORMAL_EPSILON,
				   const float distEps = DIST_EPSILON
				   )
{
	Vector4	normalizedPlane = plane;
	//normalizedPlane.FixDegeneracies( distEps );

	mxASSERT( distEps <= 0.125f );

	const UINT32 numExistingPlanes = tree.m_planes.Num();

	for( UINT32 iPlane = 0; iPlane < numExistingPlanes; iPlane++ )
	{
		const Vector4 existingPlane = tree.m_planes[ iPlane ];

		if( Planes_Equal( existingPlane, normalizedPlane, normalEps, distEps ) )
		{
			return iPlane;
		}
	}

	const UINT32 newPlaneIndex = numExistingPlanes;
	mxASSERT( newPlaneIndex <= BSP_MAX_PLANES );

	tree.m_planes.Add( normalizedPlane );

	return newPlaneIndex;
}

// Creates a new internal node.
static inline NodeID NewNode( Tree & tree )
{
	const UINT32 newNodeIndex = tree.m_nodes.Num();
	mxASSERT( newNodeIndex <= BSP_MAX_NODES );
	Node & newNode = tree.m_nodes.Add();
#if MX_DEBUG
	memset(&newNode,-1,sizeof Node);
	//DBGOUT("! creating node %u",newNodeIndex);
#endif //MX_DEBUG
	return newNodeIndex;
}

static FaceID AddPolygon( const Face& poly, Tree & tree, FaceID * head )
{
	const UINT32 newPolyIndex = tree.m_faces.Num();
	mxASSERT( newPolyIndex <= BSP_MAX_POLYS );
	Face &newPoly = tree.m_faces.Add();
	MemZero(&newPoly,sizeof(Face));
	newPoly.vertices.SetExternalStorage( newPoly.buffer, mxCOUNT_OF(newPoly.buffer) );
	newPoly.vertices = poly.vertices;
	newPoly.next = *head;
	*head = newPolyIndex;
	return newPolyIndex;
}

static int CalculatePolygonCount( const Tree& tree, FaceID iPoly )
{
	int result = 0;
	while( iPoly != NIL_INDEX )
	{
		const Face&	polygon = tree.m_faces[ iPoly ];
		result++;
		iPoly = polygon.next;
	}
	return result;
}

int Tree::PartitionPolygons(
							const Vector4& partitioner,
							const FaceID polygons,
							FaceID *frontFaces,
							FaceID *backFaces,
							FaceID *coplanar,
							int faceCounts[4],	// EPlaneSide
							const float epsilon
							)
{
	int totalFaces = 0;	// the total number of all considered polygons

	FaceID iPoly = polygons;
	while( iPoly != NIL_INDEX )
	{
		Face &	polygon = m_faces[ iPoly ];
		const FaceID iNextPoly = polygon.next;

		Vertex	buffer1[64];
		Vertex	buffer2[64];

		Face	frontPoly;
		Face	backPoly;
		frontPoly.vertices.SetExternalStorage( buffer1, mxCOUNT_OF(buffer1) );
		backPoly.vertices.SetExternalStorage( buffer2, mxCOUNT_OF(buffer2) );

		const EPlaneSide side = SplitConvexPolygonByPlane( polygon, frontPoly, backPoly, partitioner, epsilon );

		faceCounts[side]++;

		if( side == PLANESIDE_CROSS )
		{
			mxASSERT( frontPoly.vertices.Num() && backPoly.vertices.Num() );
			AddPolygon( frontPoly, *this, frontFaces );
			AddPolygon( backPoly, *this, backFaces );
		}
		else if( side == PLANESIDE_FRONT )
		{
			polygon.next = *frontFaces;
			*frontFaces = iPoly;
		}
		else if( side == PLANESIDE_BACK )
		{
			polygon.next = *backFaces;
			*backFaces = iPoly;
		}
		else
		{
			mxASSERT( side == PLANESIDE_ON );
			polygon.next = *coplanar;
			*coplanar = iPoly;
		}

		// continue
		iPoly = iNextPoly;
		totalFaces++;
	}

	return totalFaces;
}

static EPlaneSide ClassifyFaces( int total, int sides[4] )
{
	if ( sides[PLANESIDE_ON] == total ) {
		return PLANESIDE_ON;
	}
	// if nothing at the front of the clipping plane
	if ( sides[PLANESIDE_FRONT] == 0 ) {
		return PLANESIDE_BACK;
	}
	// if nothing at the back of the clipping plane
	if ( sides[PLANESIDE_BACK] == 0 ) {
		return PLANESIDE_FRONT;
	}
	return PLANESIDE_CROSS;
}

// returns index of new node
//
static NodeID BuildTree_R( Tree & tree, const FaceID polygons, BspStats &stats )
{
	mxASSERT( polygons != NIL_INDEX );

	// allocate a new internal node
	const NodeID nodeIndex = NewNode( tree );

	// select the best partitioner
	SplittingCriteria	settings;
	// we don't need polygons for collision detection
	settings.splitCost = 0;
	settings.balanceVsCuts = 1;
	settings.planeEpsilon = 0.1f;

	const FaceID bestSplitter = FindBestSplitterIndex( tree, polygons, settings );
	const Vector4 splittingPlane = PlaneFromPolygon( tree.m_faces[ bestSplitter ] );

	// partition the list
	FaceID	frontFaces = NIL_INDEX;
	FaceID	backFaces = NIL_INDEX;
	FaceID	coplanar = NIL_INDEX;
	int		faceCounts[4] = {0};

	tree.PartitionPolygons(
		splittingPlane, polygons, &frontFaces, &backFaces, &coplanar, faceCounts
	);

	stats.m_numSplits += faceCounts[PLANESIDE_CROSS];

	tree.m_nodes[ nodeIndex ].plane = GetPlaneIndex( tree, splittingPlane );
	tree.m_nodes[ nodeIndex ].faces = coplanar;

	// recursively process children
	if( frontFaces != NIL_INDEX )
	{
		tree.m_nodes[ nodeIndex ].front = BuildTree_R( tree, frontFaces, stats );
	}
	else
	{
		tree.m_nodes[ nodeIndex ].front = MAKE_LEAF( EMPTY_LEAF );
		stats.m_numEmptyLeaves++;
	}

	if( backFaces != NIL_INDEX )
	{
		tree.m_nodes[ nodeIndex ].back = BuildTree_R( tree, backFaces, stats );
	}
	else
	{
		tree.m_nodes[ nodeIndex ].back = MAKE_LEAF( SOLID_LEAF );
		stats.m_numSolidLeaves++;
	}

	Node& fsfs = tree.m_nodes[ nodeIndex ];
	NODE_TYPE frontT = GET_TYPE(fsfs.front);
	NODE_TYPE backT = GET_TYPE(fsfs.back);

	return nodeIndex;
}

ERet Tree::Build( ATriangleMeshInterface* triangleMesh )
{
	const UINT32 startTimeMSec = mxGetTimeInMilliseconds();

	struct CollectTriangles : ATriangleIndexCallback
	{
		Tree &	m_tree;
		CollectTriangles( Tree & tree ) : m_tree( tree )
		{}
		virtual void ProcessTriangle( const Vertex& a, const Vertex& b, const Vertex& c ) override
		{
			Face & newPoly = m_tree.m_faces.Add();
			newPoly.vertices.SetNum(3);
			Vertex & v1 = newPoly.vertices[0];
			Vertex & v2 = newPoly.vertices[1];
			Vertex & v3 = newPoly.vertices[2];
			v1 = a;
			v2 = b;
			v3 = c;
			newPoly.next = NIL_INDEX;
		}
	};

	CollectTriangles	callback( *this );
	triangleMesh->ProcessAllTriangles( &callback );

	chkRET_X_IF_NOT(m_faces.Num() > 0, ERR_INVALID_PARAMETER);

	// setup a linked list of polygons
	for( int iPoly = 0; iPoly < m_faces.Num()-1; iPoly++ )
	{
		m_faces[iPoly].next = iPoly+1;
	}

	BspStats	stats;
	stats.m_polysBefore = m_faces.Num();

	BuildTree_R( *this, 0, stats );

	stats.m_polysAfter = m_faces.Num();

	m_nodes.Shrink();
	m_planes.Shrink();
	m_faces.Shrink();

	stats.m_numInternalNodes = m_nodes.Num();
	stats.m_numPlanes = m_planes.Num();
	stats.m_bytesAllocated = this->BytesAllocated();

	const UINT32 currentTimeMSec = mxGetTimeInMilliseconds();
	stats.Print(currentTimeMSec - startTimeMSec);

	return ALL_OK;
}

bool Tree::PointInSolid( const Float3& point, float epsilon ) const
{
	NodeID nodeIndex = 0;
	// If < 0, we are in a leaf node
	while( !IS_LEAF( nodeIndex ) )
	{
		// Find which side of the node we are on
		const Node& node = m_nodes[ nodeIndex ];
		const Vector4& plane = m_planes[ node.plane ];
		const EPlaneSide side = CalculatePlaneSide( plane, point, epsilon );
		// Go down the appropriate side
		nodeIndex = (side == PLANESIDE_FRONT) ? node.front : node.back;
	}
	return IS_SOLID_LEAF( nodeIndex );
}
#if 0
float Tree::DistanceToPoint( const Float3& point, float epsilon ) const
{
	int nodeIndex = 0;
	float distance = 0.f;
	// If < 0, we are in a leaf node
	while( nodeIndex >= 0 )
	{
		// Find which side of the node we are on
		const Node& node = m_nodes[ nodeIndex ];
		const Vector4& plane = m_planes[ node.plane ];
		distance = Plane_PointDistance( plane, point );
		const int nearIndex = (distance >= 0.f);	// child of Node for half-space containing the origin of Ray: 1 - front, 0 - back
		const int sides[2] = { node.back, node.front };
		// Go down the appropriate side
		if( distance > +epsilon ) {
			nodeIndex = node.front;
		} else if( distance < -epsilon ) {
			nodeIndex = node.back;
		} else {
			nodeIndex = sides[ nearIndex ];
		}
	}
	// return the minimum (closest) distance
	return distance;
}
#endif
bool PlaneLineIntersection( const Float4& plane, const Float3& start, const Float3& end, float *fraction )
{
	const float d1 = Plane_PointDistance( plane, start );
	const float d2 = Plane_PointDistance( plane, end );
	if ( d1 == d2 ) {
		return false;
	}
	if ( d1 > 0.0f && d2 > 0.0f ) {
		return false;
	}
	if ( d1 < 0.0f && d2 < 0.0f ) {
		return false;
	}
	*fraction = ( d1 / ( d1 - d2 ) );
	return ( *fraction >= 0.0f && *fraction <= 1.0f );
}

bool PlaneRayIntersection( const Float4& plane, const Float3& start, const Float3& dir, float *scale )
{
	const Float3& planeNormal = Plane_CalculateNormal( plane );
	const float D = plane.w;

	const float d2 = planeNormal * dir;
	if ( d2 == 0.0f ) {
		return false;
	}

	const float d1 = planeNormal * start + D;

	*scale = -( d1 / d2 );
	return true;
}

size_t Tree::BytesAllocated() const
{
	return m_nodes.GetAllocatedMemory()
		+ m_planes.GetAllocatedMemory()
		+ m_faces.GetAllocatedMemory()
		;
}

// @todo:
// weld coplanar polygon faces

//// fix degenerate normal and dist
//bool FixDegeneracies( Vector4 * plane float distEpsilon )
//{
//	bool fixedNormal = FixDegenerateNormal();
//	// only fix dist if the normal was degenerate
//	if ( fixedNormal ) {
//		if ( fabs( d - Math::Rint( d ) ) < distEpsilon ) {
//			d = Math::Rint( d );
//		}
//	}
//	return fixedNormal;
//}

int CastRay_R(
			 const Float3& start, const Float3& direction,
			 const Tree& tree, const NodeID nodeIndex,
			 float tmin, float tmax,
			 float *thit
			 )
{
	if( !IS_LEAF( nodeIndex ) )
	{
		const Node& node = tree.m_nodes[ nodeIndex ];
		const Vector4& plane = tree.m_planes[ node.plane ];
		const float distance = Plane_PointDistance( plane, start );
		const float denom = Float3_Dot( Plane_GetNormal(plane), direction );
		const int nearIndex = (distance >= 0.f);	// child of Node for half-space containing the origin of Ray: 1 - front, 0 - back
		const NodeID sides[2] = { node.back, node.front };
		int firstSide = nearIndex;
		// If denom is zero, ray runs parallel to plane. In this case,
		// just fall through to visit the near side (the one 'start' lies on)
		if( denom != 0.0f )
		{
			float t = -distance / denom;
			if( 0.0f <= t && t <= tmax )
			{
				if( t >= tmin ) {
					// visit near side
					if( CastRay_R( start, direction, tree, sides[ firstSide ], tmin, t, thit ) ) {
						return 1;
					}
					// visit far side
					return CastRay_R( start, direction, tree, sides[ firstSide^1 ], t, tmax, thit );
				} else {
					// 0 <= t < tmin, only the far side needs to be traversed
					firstSide = 1^firstSide;
				}
			}
		}
		return CastRay_R( start, direction, tree, sides[ firstSide ], tmin, tmax, thit );
	}
	else
	{
		// Now at a leaf. If it is solid, there's a hit at time tmin, so exit
		if( IS_SOLID_LEAF( nodeIndex ) ) {
			*thit = tmin;
			return 1;
		}
	}
	// No hit
	return 0;
}

void Tree::CastRay(
			 const Float3& start, const Float3& direction,
			 RayCastResult &result
			 ) const
{
	float thit;
	result.hitAnything = CastRay_R(start,direction,*this,0,0.0f,9999.0f, &thit);
	if(result.hitAnything){
		result.position = start + direction * thit;
	}
}

float Tree::CastRay( const Float3& start, const Float3& direction ) const
{
	int nodeIndex = 0;
	Float3 end = start + direction * 1e4f;

	float distance = 0.f;

	const float epsilon = 1e-3f;

	for(;;)
	{
		// If < 0, we are in a leaf node
		if( nodeIndex < 0 ){
			// return the minimum (closest) distance
			return distance;
		}
		const Node& node = m_nodes[ nodeIndex ];
		const Vector4& plane = m_planes[ node.plane ];
		// distance from plane for trace start and end
		const float d1 = Plane_PointDistance( plane, start );
		const float d2 = Plane_PointDistance( plane, end );
		// see which sides we need to consider
		if( d1 >= +epsilon && d2 >= +epsilon ) {
			nodeIndex = node.front;
		}
		else if( d1 < -epsilon && d2 < -epsilon ) {
			nodeIndex = node.back;
		}
		else {
			// Straddling
			float fraction;
			PlaneLineIntersection( plane, start, end, &fraction );
			const Float3 mid = start + (end - start) * fraction;
			UNDONE;
			CastRay( start, mid );
			CastRay( mid, end );


		}
	}
	mxUNREACHABLE;
	return distance;

#if 0
	TStaticList< Node* >	nodes;
	TStaticList< float >	dists;

	float	tmin = 0.0f;
	float	tmax = 1e5f;

	int nodeIndex = 0;
	float distance = 0.f;

	for(;;)
	{
		if( nodeIndex >= 0 )
		{
			const Node& node = m_nodes[ nodeIndex ];
			const Vector4& plane = m_planes[ node.plane ];
			const Float3& planeNormal = Vector4_As_Float3( plane );
			const float denom = Float3_Dot(direction, planeNormal);
			const float distance = Plane_PointDistance( plane, start );
			const int nearIndex = (distance > 0.f);
			// If denom is zero, ray runs parallel to the plane.
			// In this case, just fall through to visit the near side
			// (the one 'start' lies on).
			if( denom != 0.0f )
			{
				float t = distance / denom;
				if( 0.f <= t && t <= tmax )
				{
					if( t >= tmin )
					{
						// Straddling, push far side onto stack, then visit near side
						UNDONE;
					}
				}
			}
		}


		UNDONE;
	}
	mxUNREACHABLE;
	return distance;
#endif
}
#if 0

bool Intersect_R( const Tree& tree, int nodeID, const Float3& start, const Float3& end, float &t )
{
	// If < 0, we are in a leaf node
	if( nodeID < 0 ) {
		// return the minimum (closest) distance
		return nodeID == BSP_SOLID_LEAF;
	}

	const float epsilon = 1e-3f;

	const Node& node = tree.m_nodes[ nodeID ];
	const Vector4& plane = tree.m_planes[ node.plane ];
	// distance from plane for trace start and end
	const float d1 = Plane_PointDistance( plane, start );
	const float d2 = Plane_PointDistance( plane, end );
	// see which sides we need to consider
	if( d1 >= +epsilon && d2 >= +epsilon ) {
		nodeID = node.front;
	}
	else if( d1 < -epsilon && d2 < -epsilon ) {
		nodeID = node.back;
	}
	else {
		// Straddling

		if ( d1 < d2 ) {
			idist = 1.0f / (d1-d2);
			side = 1;
			frac2 = (d1 + offset) * idist;
			frac = (d1 - offset) * idist;
		} else if (d1 > d2) {
			idist = 1.0f / (d1-d2);
			side = 0;
			frac2 = (d1 - offset) * idist;
			frac = (d1 + offset) * idist;
		} else {
			side = 0;
			frac = 1.0f;
			frac2 = 0.0f;
		}

		// move up to the node
		if ( frac < 0.0f ) {
			frac = 0.0f;
		}
		else if ( frac > 1.0f ) {
			frac = 1.0f;
		}

		float fraction = ( d1 / ( d1 - d2 ) );
		mxASSERT( fraction >= 0.0f && fraction <= 1.0f );

		t = smallest( t, fraction );

		const Float3 mid = start + (end - start) * fraction;
		if(Intersect_R( tree, node.front, start, mid, t )) {
			return true;
		}
		if(Intersect_R( tree, node.back, mid, end, t )) {
			return true;
		}
		return false;
	}
	return false;
}
#endif

// This was copied straight from the book "Real-Time Collision Detection" by Christer Ericson (2005):
#if 0
// Intersect ray/segment R(t)=p+t*d, tmin <= t <= tmax, against bsp tree
// ’node’, returning time thit of first intersection with a solid leaf, if any
int RayIntersect(BSPNode *node, Point p, Vector d, float tmin, float tmax, float *thit)
{
	std::stack<BSPNode *> nodeStack;
	std::stack<float> timeStack;
	assert(node != NULL);
	while (1) {
		if (!node->IsLeaf()) {
			float denom = Dot(node- >plane.n, d);
			float dist = node->plane.d - Dot(node->plane.n, p);
			int nearIndex = dist > 0.0f;
			// If denom is zero, ray runs parallel to plane. In this case,
			// just fall through to visit the near side (the one p lies on)
			if (denom != 0.0f) {
				float t = dist / denom;
				if (0.0f <= t && t <= tmax) {
					if (t >= tmin) {
						// Straddling, push far side onto stack, then visit near side
						nodeStack.push(node->child[1^nearIndex]);
						timeStack.push(tmax);
						tmax = t;
					} else nearIndex = 1^nearIndex;// 0 <= t < tmin, visit far side
				}
			}
			node = node->child[nearIndex];
		} else {
			// Now at a leaf. If it is solid, there’s a hit at time tmin, so exit
			if (node->IsSolid()) {
				*thit = tmin;
				return 1;
			}
			// Exit if no more subtrees to visit, else pop off a node and continue
			if (nodeStack.empty()) break;
			tmin = tmax;
			node = nodeStack.top(); nodeStack.pop();
			tmax = timeStack.top(); timeStack.pop();
		}
	}
	// No hit
	return 0;
}
#endif

EPlaneSide Tree::PartitionNodeWithPlane(
	const Vector4& partitioner,
	NodeID nodeId,
	NodeID *front,
	NodeID *back
	)
{
	if( IS_LEAF( nodeId ) ) {
		*front = nodeId;
		*back = nodeId;
		return PLANESIDE_CROSS;
	}

	const Node& node = m_nodes[ nodeId ];
	const UINT16 iPlaneIndex = node.plane;
	const Vector4& nodePlane = m_planes[ iPlaneIndex ];

	// partition the operand
	FaceID	frontFaces = NIL_INDEX;
	FaceID	backFaces = NIL_INDEX;
	FaceID	coplanar = NIL_INDEX;
	int		faceCounts[4] = {0};

	const int totalCount = PartitionPolygons( partitioner, node.faces, &frontFaces, &backFaces, &coplanar, faceCounts );
	const EPlaneSide side = ClassifyFaces( totalCount, faceCounts );

	const float dot = Plane_GetNormal( partitioner ) * Plane_GetNormal( nodePlane );

	// Variable to hold if the normal vectors are facing the same direction.
	bool  colinearPlanes = ( dot > 0.0f );

	// If planes are coplanar.
	if ( side == PLANESIDE_ON )
	{
		// case "On"
		if ( colinearPlanes )
		{
			// "parallel-on"
			*front = node.front;
			*back = node.back;
		}
		else
		{
			// "antiparallel-on"
			*front = node.back;
			*back = node.front;
		}

	}//End of case if coplanar

#if 0
	else if ( side == PLANESIDE_FRONT )
	{
		mxASSERT2( frontFaces, "All the polygons of the node must be in front of the splitting splitPlane" );
		mxASSERT2( coplanar == NULL, "Coplanar polys are not supported!" );

		node->Faces = frontFaces;

		if ( colinearPlanes )
		{
			// Only the back child of the node needs to be partitioned.
			Node *  partitioned_back_F = NULL;
			Node *  partitioned_back_B = NULL;

			PartitionNodeWithPlane( partitioner, node.back, partitioned_back_F, partitioned_back_B );

			*front = node;
			*front->SetBack( partitioned_back_F );
			// node->frontChild remains intact...

			*back = partitioned_back_B;
		}
		else
		{
			// Only the front child of the node has to be partitioned.
			Node *  partitioned_front_F = NULL;
			Node *  partitioned_front_B = NULL;

			PartitionNodeWithPlane( partitioner, node.front, partitioned_front_F, partitioned_front_B );

			*front = node;
			*front->SetFront( partitioned_front_F );
			// node->backChild remains intact...

			*back = partitioned_front_B;
		}
		return;
	}

	else if ( side == PLANESIDE_BACK )
	{
		mxASSERT2( backFaces, "All polygons of the node must be behind the splitting splitPlane" );
		mxASSERT2( coplanar == NULL, "Coplanar polys are not supported!" );

		node->Faces = backFaces;

		if ( colinearPlanes )
		{
			// Only the front child of the node needs to be partitioned.
			Node *  partitioned_front_F = NULL;
			Node *  partitioned_front_B = NULL;

			PartitionNodeWithPlane( partitioner, node.front, partitioned_front_F, partitioned_front_B );

			*front = partitioned_front_F;

			*back = node;
			*back->SetFront( partitioned_front_B );
			// node->backChild remains intact...
		}
		else
		{
			// Only the back child of the node is partitioned.
			Node *  partitioned_back_F = NULL;
			Node *  partitioned_back_B = NULL;

			PartitionNodeWithPlane( partitioner, node.back, partitioned_back_F, partitioned_back_B );

			*front = partitioned_back_F;

			*back = node;
			*back->SetBack( partitioned_back_B );
			// node->frontChild remains intact...
		}
	}//End of case "No front polys"
#endif
	else
	{
		// Split both children of the node.

		// Create two new nodes resulting from the partitioning.
		*front = NewNode( *this );
		*back = NewNode( *this );

		m_nodes[ *front ].plane = iPlaneIndex;
		m_nodes[ *front ].faces = frontFaces;

		m_nodes[ *back ].plane = iPlaneIndex;
		m_nodes[ *back ].faces = backFaces;


		NodeID partitioned_front_F = NIL_INDEX;
		NodeID	partitioned_front_B = NIL_INDEX;

		NodeID	partitioned_back_F = NIL_INDEX;
		NodeID	partitioned_back_B = NIL_INDEX;

		PartitionNodeWithPlane( partitioner, m_nodes[ nodeId ].front, &partitioned_front_F, &partitioned_front_B );
		PartitionNodeWithPlane( partitioner, m_nodes[ nodeId ].back, &partitioned_back_F, &partitioned_back_B );

		m_nodes[ *front ].front = partitioned_front_F;
		m_nodes[ *front ].back = partitioned_back_F;

		m_nodes[ *back ].front = partitioned_front_B;
		m_nodes[ *back ].back = partitioned_back_B;
	}

	return side;
}

#if 0
static int AppendTree( Tree & treeA, const Tree& treeB )
{
	const int planeOffset = treeA.m_planes.Num();
	const int nodeOffset = treeA.m_nodes.Num();
	const int faceOffset = treeA.m_faces.Num();

	const int newPlanes = treeB.m_planes.Num();
	const int newNodes = treeB.m_nodes.Num();
	const int newFaces = treeB.m_faces.Num();

	treeA.m_planes.ReserveMore( newPlanes );
	treeA.m_nodes.ReserveMore( newNodes );
	treeA.m_faces.ReserveMore( newFaces );

	for( UINT32 iPlane = 0; iPlane < m_planes.Num(); iPlane++ )
	{
		m_planes[iPlane] = Plane_Translate( m_planes[iPlane], T );
	}
	for( UINT32 iPoly = 0; iPoly < m_faces.Num(); iPoly++ )
	{
		Poly & poly = m_faces[iPoly];
		for( UINT32 iVtx = 0; iVtx < poly.vertices.Num(); iVtx++ )
		{
			poly.vertices[iVtx].xyz += T;
		}
	}


	return firstNodeId;
}
#endif

// treeA <- (treeB, nodeB)
static NodeID CopySubTree( Tree & treeA, const Tree& treeB, NodeID iNodeB )
{
	DBGOUT("CopySubTree: %d",GET_PAYLOAD(iNodeB));
	NodeID subtreeId = 0;
	if( IS_INTERNAL( iNodeB ) )
	{
		const Node& rNodeB = treeB.m_nodes[ iNodeB ];
		const Vector4& rPlaneB = treeB.m_planes[ rNodeB.plane ];

		const int iNewPlaneA = treeA.m_planes.Num();
		treeA.m_planes.Add( rPlaneB );

		const NodeID iNewNodeA = NewNode( treeA );
		Node &rNodeA = treeA.m_nodes[ iNewNodeA ];

		rNodeA.plane = iNewPlaneA;

#if 0
		rNodeA.faces = NIL_INDEX;

		FaceID iFaceB = rNodeB.faces;
		while( iFaceB != NIL_INDEX )
		{
			const Face& rFaceB = treeB.m_faces[ iFaceB ];

			const FaceID iNewFaceA = treeA.m_faces.Num();
			treeA.m_faces.Add( rFaceB );

			Face &rFaceA = treeA.m_faces[ iNewFaceA ];
			rFaceA.next = rNodeA.faces;
			rNodeA.faces = iNewFaceA;

			iFaceB = rFaceB.next;
		}
#endif

		treeA.m_nodes[ iNewNodeA ].front = CopySubTree( treeA, treeB, rNodeB.front );
		treeA.m_nodes[ iNewNodeA ].back = CopySubTree( treeA, treeB, rNodeB.back );
	}
	else
	{
		subtreeId = iNodeB;
	}
	return subtreeId;
}

//static const AABB24 CalculateFaceBounds( const Tree& tree, const FaceID faces )
//{
//	AABB24 bounds;
//	AABB24_Clear( &bounds );
//
//	FaceID iFace = faces;
//	while( iFace != NIL_INDEX )
//	{
//		const Face& rFace = tree.m_faces[ iFace ];
//
//		for( UINT32 iVtx = 0; iVtx < rFace.vertices.Num(); iVtx++ )
//		{
//			AABB24_AddPoint( &bounds, rFace.vertices[iVtx].xyz );
//		}
//
//		iFace = rFace.next;
//	}
//
//	return bounds;
//}
static const AABB24 CalculateFaceBounds( const Face& rFace )
{
	AABB24 bounds;
	AABB24_Clear( &bounds );
	for( UINT32 iVtx = 0; iVtx < rFace.vertices.Num(); iVtx++ )
	{
		AABB24_AddPoint( &bounds, rFace.vertices[iVtx].xyz );
	}
	return bounds;
}
static void CalculateNodeBounds_R( const Tree& tree, const NodeID iNode, AABB24 * bounds )
{
	if( IS_INTERNAL( iNode ) )
	{
		const Node& rNode = tree.m_nodes[ iNode ];

		FaceID iFace = rNode.faces;
		while( iFace != NIL_INDEX )
		{
			const Face& rFace = tree.m_faces[ iFace ];

			const AABB24 faceBounds = CalculateFaceBounds( rFace );

			AABB24_AddAABB( bounds, faceBounds );

			iFace = rFace.next;
		}

		CalculateNodeBounds_R( tree, rNode.front, bounds );
		CalculateNodeBounds_R( tree, rNode.back, bounds );
	}
}
static const AABB24 CalculateNodeBounds( const Tree& tree, const NodeID iNode )
{
	AABB24	bounds;
	AABB24_Clear( &bounds );
	CalculateNodeBounds_R( tree, iNode, &bounds );
	return bounds;
}

#if 0
static void ClipFacesWithConvexBrush_R( Tree & treeA, const NodeID iNodeA, const Tree& treeB, const NodeID iNodeB, const AABB24& boundsB )
{
	mxASSERT(IS_INTERNAL(iNodeA));
	mxASSERT(IS_INTERNAL(iNodeB));
	if( IS_INTERNAL( iNodeA ) && IS_INTERNAL( iNodeB ) )
	{
		Node& rNodeA = treeA.m_nodes[ iNodeA ];

		FaceID iFaceA = rNodeA.faces;
		while( iFaceA != NIL_INDEX )
		{
			const Face& rFaceA = treeA.m_faces[ iFaceA ];

			const AABB24 faceBoundsA = CalculateFaceBounds( rFaceA );

			if( AABB_Intersect( faceBoundsA, boundsB ) )
			{
				Vertex	buffer1[64];
				Vertex	buffer2[64];

				Face	frontPoly;
				Face	backPoly;
				frontPoly.vertices.SetExternalStorage( buffer1, mxCOUNT_OF(buffer1) );
				backPoly.vertices.SetExternalStorage( buffer2, mxCOUNT_OF(buffer2) );

				const EPlaneSide side = SplitConvexPolygonByPlane( rFaceA, frontPoly, backPoly, partitioner, epsilon );
			}

			iFaceA = rFaceA.next;
		}
	}
}

static void ClipFacesWithConvexBrush( Tree & treeA, const NodeID iNodeA, const Tree& treeB, const NodeID iNodeB )
{
	mxASSERT(IS_INTERNAL(iNodeA));
	mxASSERT(IS_INTERNAL(iNodeB));
	const AABB24 boundsB = CalculateNodeBounds( treeB, iNodeB );
	ClipFacesWithConvexBrush_R( treeA, iNodeA, treeB, iNodeB, boundsB );
}
#endif

#if 0
static void ClipFacesWithConvexBrush_R(
									   Tree & treeA, const FaceID facesA, FaceID *newFacesA,
									   const Tree& treeB, const NodeID iNodeB, const AABB24& boundsB
									   )
{
	mxASSERT(IS_INTERNAL(iNodeB));
	if( IS_INTERNAL( iNodeB ) )
	{
		const Node& rNodeB = treeB.m_nodes[ iNodeB ];
		const Vector4& planeB = treeB.m_planes[ rNodeB.plane ];

		FaceID iFaceA = *facesA;
		while( iFaceA != NIL_INDEX )
		{
			const Face& rFaceA = treeA.m_faces[ iFaceA ];

			const AABB24 faceBoundsA = CalculateFaceBounds( rFaceA );

			if( AABB_Intersect( faceBoundsA, boundsB ) )
			{
				Vertex	buffer1[64];
				Vertex	buffer2[64];

				Face	frontPoly;
				Face	backPoly;
				frontPoly.vertices.SetExternalStorage( buffer1, mxCOUNT_OF(buffer1) );
				backPoly.vertices.SetExternalStorage( buffer2, mxCOUNT_OF(buffer2) );

				const EPlaneSide side = SplitConvexPolygonByPlane( rFaceA, frontPoly, backPoly, planeB );

				if( frontPoly.vertices.Num() ) {
					AddPolygon( frontPoly, treeA, facesA );
				}
				// discard the polygon behind the plane
			}

			iFaceA = rFaceA.next;
		}
	}
}

static void ClipFacesWithConvexBrush(
									 Tree & treeA, const FaceID facesA, FaceID *newFacesA,
									 const Tree& treeB, const NodeID iNodeB, const AABB24& boundsB
									 )
{
	mxASSERT(IS_INTERNAL(iNodeB));
	const AABB24 boundsB = CalculateNodeBounds( treeB, iNodeB );
	ClipFacesWithConvexBrush_R( treeA, facesA, newFacesA, treeB, iNodeB, boundsB );
}
#endif
// computes boolean A - B
static void MergeSubtract( Tree & treeA, NodeID * iNodeA, Tree & treeB, const NodeID iNodeB )
{
	if( IS_INTERNAL( *iNodeA ) )
	{
		// this is an internal node
		Node& nodeA = treeA.m_nodes[ *iNodeA ];
		const Vector4& plane = treeA.m_planes[ nodeA.plane ];

		// Clip this node's polygons with the other tree.
		const FaceID faceListA = nodeA.faces;
//		nodeA.faces = NIL_INDEX;
//		ClipFacesWithConvexBrush( treeA, faceListA, &nodeA.faces, treeB, iNodeB );

		// Partition the other tree and merge the first tree with the resulting pieces.
		NodeID nodeB_front = NIL_INDEX;
		NodeID nodeB_back = NIL_INDEX;
		treeB.PartitionNodeWithPlane( plane, iNodeB, &nodeB_front, &nodeB_back );

		MergeSubtract( treeA, &nodeA.front, treeB, nodeB_front );
		MergeSubtract( treeA, &nodeA.back, treeB, nodeB_back );
	}
	else
	{
		// this is a leaf node
		if( IS_SOLID_LEAF( *iNodeA ) )
		{
			//@todo: discard the old subtree
			//*iNodeA = CopySubTree( treeA, treeB, iNodeB );
		}
		// empty space - do nothing
	}
}

void Tree::Subtract( Tree& other )
{
	NodeID rootId = 0;
	MergeSubtract( *this, &rootId, other, 0 );
}

void Tree::CopyFrom( const Tree& other )
{
	m_planes = other.m_planes;
	m_nodes = other.m_nodes;

	//const int newPlanes = other.m_planes.Num();
	//const int newNodes = other.m_nodes.Num();
	const int newFaces = other.m_faces.Num();

	m_faces.SetNum( newFaces );

	for( UINT32 iFace = 0; iFace < other.m_faces.Num(); iFace++ )
	{
		const Face& source = other.m_faces[ iFace ];
		Face &dest = m_faces[iFace];
		dest.next = source.next;
		DBGOUT("next: %d", source.next);
		dest.vertices.SetNum( source.vertices.Num() );
		for( UINT32 iVtx = 0; iVtx < source.vertices.Num(); iVtx++ )
		{
			dest.vertices[iVtx] = source.vertices[iVtx];
		}
	}
}

void Tree::Negate()
{
UNDONE;
}
void Tree::Translate( const Float3& T )
{
	for( UINT32 iPlane = 0; iPlane < m_planes.Num(); iPlane++ )
	{
		m_planes[iPlane] = Plane_Translate( m_planes[iPlane], T );
	}
	for( UINT32 iPoly = 0; iPoly < m_faces.Num(); iPoly++ )
	{
		Face & poly = m_faces[iPoly];
		for( UINT32 iVtx = 0; iVtx < poly.vertices.Num(); iVtx++ )
		{
			poly.vertices[iVtx].xyz += T;
		}
	}
}

static void GenerateMesh_R(
						   const Tree& tree, const NodeID nodeId,
						   TArray< BSP::Vertex > &vertices, TArray< UINT16 > &indices
						   )
{
	if( IS_INTERNAL( nodeId ) )
	{
		const Node& node = tree.m_nodes[ nodeId ];
		const Vector4& plane = tree.m_planes[ node.plane ];

		// TODO: don't emit this node if the data in this node hasn't been changed.
		GenerateMesh_R( tree, node.front, vertices, indices );
		GenerateMesh_R( tree, node.back, vertices, indices );

		// Loop through all faces of this node.
		FaceID iFaceId = node.faces;
		while( iFaceId != NIL_INDEX )
		{
			const Face& face = tree.m_faces[ iFaceId ];
			mxASSERT( face.vertices.Num() >= 3 );

			const int numTriangles = face.vertices.Num() - 2;

			// Triangulate the current convex polygon...

			const Vertex& basePoint = face.vertices[ 0 ];

			const UINT16 iBasePoint = vertices.Num();
			vertices.Add( basePoint );

			for ( int i = 1; i < numTriangles + 1; i++ )
			{
				vertices.Add( face.vertices[ i ] );
				vertices.Add( face.vertices[ i+1 ] );

				indices.Add( iBasePoint );
				indices.Add( iBasePoint + 1 );
				indices.Add( iBasePoint + 2 );
			}

			iFaceId = face.next;
		}
	}
}

void Tree::GenerateMesh( TArray< BSP::Vertex > &vertices, TArray< UINT16 > &indices ) const
{
	vertices.Empty();
	indices.Empty();
	GenerateMesh_R( *this, 0, vertices, indices );
}

namespace Debug
{
	static int CalculateFaceCount( const Tree& tree, FaceID faces )
	{
		int result = 0;
		FaceID iFaceId = faces;
		while( iFaceId != NIL_INDEX )
		{
			const Face& face = tree.m_faces[ iFaceId ];
			result++;
			iFaceId = face.next;
		}
		return result;
	}
	static const String32 NodeID_To_String( NodeID nodeIndex )
	{
		String32 result;
		if( IS_LEAF(nodeIndex) ) {
			Str::CopyS( result, IS_SOLID_LEAF(nodeIndex) ? "Solid" : "Air" );
		} else {
			Str::SetInt( result, GET_PAYLOAD(nodeIndex) );
		}
		return result;
	}
	static void PrintTree_R( const Tree& tree, INT16 nodeIndex, int depth )
	{
		LogStream log(LL_Debug);
		log.Repeat('\t', depth);

		if( !IS_LEAF( nodeIndex ) )
		{
			const Node& node = tree.m_nodes[ nodeIndex ];
			const Vector4& plane = tree.m_planes[ node.plane ];

			log << "Node[" << nodeIndex << "]: neg=" << NodeID_To_String(node.back) << ", pos=" << NodeID_To_String(node.front) << ", faces: " << CalculateFaceCount(tree, node.faces);
			log.Flush();

			PrintTree_R( tree, node.back, depth+1 );
			PrintTree_R( tree, node.front, depth+1 );
		}
		else
		{
			log << "Leaf: " << NodeID_To_String(nodeIndex);
		}
	}
	void PrintTree( const Tree& tree )
	{
		PrintTree_R(tree, 0, 0);
	}
}//namespace Debug

}//namespace BSP
