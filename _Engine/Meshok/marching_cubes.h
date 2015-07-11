#pragma once

#include <Meshok/Meshok.h>
#include <Meshok/SDF.h>

namespace MarchingCubes
{
	void Contour(const AVolume* surface,
		const Float3& _aabbMin, const Float3& _aabbMax,
		int gridSize[3], AMeshBuilder& _renderer);

	void Triangulate(
		const AVolume* surface,
		const Float3& _min, const Float3& _max,
		int gridSizeX, int gridSizeY, int gridSizeZ,
		AMeshBuilder& _mesh
	);

	class MC_Stats {
	public:
		UINT32		m_numPolygons;	// number of resulting polygons
	public:
		MC_Stats();
		void Print( UINT32 elapsedTimeMSec );
	};

	void GenerateEdgeTable( UINT16 edgeTable[256] );
	void GenerateTriangleTable( UINT16 triangleTable[256] );

}//namespace MarchingCubes
