#include "MeshManager.h"

// std
#include <cassert>
#include <fstream>
#include <string>

VertexData::VertexData()
	: position(0, 0, 0)
	, normal(0, 0, 0)
	, uv(0, 0)
	, tangent(0, 0, 0)
{}

VertexData::VertexData(float px, float py, float pz,
					   float nx, float ny, float nz,
					   float u,  float v,
					   float tx, float ty, float tz)
	: position(px, py, pz)
	, normal(nx, ny, nz)
	, uv(u, v)
	, tangent(tx, ty, tz)
{}

MeshData MeshManager::CreateBox(const float width, const float height, const float depth)
{
	MeshData mesh;

	// half extents
	const float w = 0.5f * width;
	const float h = 0.5f * height;
	const float d = 0.5f * depth;

	std::vector<VertexData>& vertices = mesh.vertices;
	vertices.resize(24);
	
	// front
	vertices[ 0] = VertexData(-w, -h, -d,  0,  0, -1,  0, +1, +1,  0,  0);
	vertices[ 1] = VertexData(-w, +h, -d,  0,  0, -1,  0,  0, +1,  0,  0);
	vertices[ 2] = VertexData(+w, +h, -d,  0,  0, -1, +1,  0, +1,  0,  0);
	vertices[ 3] = VertexData(+w, -h, -d,  0,  0, -1, +1, +1, +1,  0,  0);

	// back
	vertices[ 4] = VertexData(-w, -h, +d,  0,  0, +1, +1, +1, -1,  0,  0);
	vertices[ 5] = VertexData(+w, -h, +d,  0,  0, +1,  0, +1, -1,  0,  0);
	vertices[ 6] = VertexData(+w, +h, +d,  0,  0, +1,  0,  0, -1,  0,  0);
	vertices[ 7] = VertexData(-w, +h, +d,  0,  0, +1, +1,  0, -1,  0,  0);

	// top
	vertices[ 8] = VertexData(-w, +h, -d,  0, +1,  0,  0, +1, +1,  0,  0);
	vertices[ 9] = VertexData(-w, +h, +d,  0, +1,  0,  0,  0, +1,  0,  0);
	vertices[10] = VertexData(+w, +h, +d,  0, +1,  0, +1,  0, +1,  0,  0);
	vertices[11] = VertexData(+w, +h, -d,  0, +1,  0, +1, +1, +1,  0,  0);

	// bottom
	vertices[12] = VertexData(-w, -h, -d,  0, -1,  0, +1, +1, -1,  0,  0);
	vertices[13] = VertexData(+w, -h, -d,  0, -1,  0,  0, +1, -1,  0,  0);
	vertices[14] = VertexData(+w, -h, +d,  0, -1,  0,  0,  0, -1,  0,  0);
	vertices[15] = VertexData(-w, -h, +d,  0, -1,  0, +1,  0, -1,  0,  0);

	// left
	vertices[16] = VertexData(-w, -h, +d, -1,  0,  0,  0, +1,  0,  0, -1);
	vertices[17] = VertexData(-w, +h, +d, -1,  0,  0,  0,  0,  0,  0, -1);
	vertices[18] = VertexData(-w, +h, -d, -1,  0,  0, +1,  0,  0,  0, -1);
	vertices[19] = VertexData(-w, -h, -d, -1,  0,  0, +1, +1,  0,  0, -1);

	// right
	vertices[20] = VertexData(+w, -h, -d, +1,  0,  0,  0, +1,  0,  0, +1);
	vertices[21] = VertexData(+w, +h, -d, +1,  0,  0,  0,  0,  0,  0, +1);
	vertices[22] = VertexData(+w, +h, +d, +1,  0,  0, +1,  0,  0,  0, +1);
	vertices[23] = VertexData(+w, -h, +d, +1,  0,  0, +1, +1,  0,  0, +1);

	std::vector<uint16_t>& indices = mesh.indices;
	indices.resize(36);

	// front
	indices[ 0] =  0; indices[ 1] =  1; indices[ 2] =  2;
	indices[ 3] =  0; indices[ 4] =  2; indices[ 5] =  3;

	// back
	indices[ 6] =  4; indices[ 7] =  5; indices[ 8] =  6;
	indices[ 9] =  4; indices[10] =  6; indices[11] =  7;

	// top
	indices[12] =  8; indices[13] =  9; indices[14] = 10;
	indices[15] =  8; indices[16] = 10; indices[17] = 11;

	// bottom
	indices[18] = 12; indices[19] = 13; indices[20] = 14;
	indices[21] = 12; indices[22] = 14; indices[23] = 15;

	// left
	indices[24] = 16; indices[25] = 17; indices[26] = 18;
	indices[27] = 16; indices[28] = 18; indices[29] = 19;

	// right
	indices[30] = 20; indices[31] = 21; indices[32] = 22;
	indices[33] = 20; indices[34] = 22; indices[35] = 23;

	return mesh;
}

MeshData MeshManager::CreateGrid(const float width, const float depth, const std::size_t m, const std::size_t n)
{
	assert(m > 1 && n > 1);

	MeshData mesh;

	const std::size_t vertexCount = m * n;
	const std::size_t triangleCount = (m - 1) * (n - 1) * 2;

	// half extents
	float w = 0.5f * width;
	float d = 0.5f * depth;

	float dx = width / (n - 1);
	float dz = depth / (m - 1);

	float du = 1.0f / (n - 1);
	float dv = 1.0f / (m - 1);

	mesh.vertices.resize(vertexCount);

	for (std::size_t i = 0; i < m; ++i)
	{
		float z = d - i * dz;

		for (std::size_t j = 0; j < n; ++j)
		{
			float x = j * dx - w;

			mesh.vertices[i * n + j].position = XMFLOAT3(x, 0.0f, z);
			mesh.vertices[i * n + j].uv = XMFLOAT2(j * du, i * dv);
			mesh.vertices[i * n + j].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
			mesh.vertices[i * n + j].tangent = XMFLOAT3(1.0f, 0.0f, 0.0f);
		}
	}

	mesh.indices.resize(triangleCount * 3);

	std::size_t k = 0;

	for (std::size_t i = 0; i < m - 1; ++i)
	{
		for (std::size_t j = 0; j < n - 1; ++j)
		{
			mesh.indices[k + 0] = MeshData::IndexType((i + 0) * n + (j + 0));
			mesh.indices[k + 1] = MeshData::IndexType((i + 0) * n + (j + 1));
			mesh.indices[k + 2] = MeshData::IndexType((i + 1) * n + (j + 0));

			mesh.indices[k + 3] = MeshData::IndexType((i + 1) * n + (j + 0));
			mesh.indices[k + 4] = MeshData::IndexType((i + 0) * n + (j + 1));
			mesh.indices[k + 5] = MeshData::IndexType((i + 1) * n + (j + 1));

			k += 6;
		}
	}

	return mesh;
}

MeshData MeshManager::LoadModel(const std::string& path)
{
	MeshData mesh;


	std::vector<VertexData>& vertices = mesh.vertices;

	std::ifstream stream(path);
	assert(stream);



	std::string line;


	char a;
	int i;

	// skip header
	std::getline(stream, line);

	while (std::getline(stream, line))
	{
		std::istringstream iss(line);

		
		VertexData vertex;
		
		// skip index
		iss >> i >> a >> i >> a >> i; // ib
		//iss >> i >> a; // vb

		// position
		iss >> a >> vertex.position.x >> a >> vertex.position.y >> a >> vertex.position.z;

		// skip blend index
		iss >> a >> i >> a >> i >> a >> i >> a >> i;
		
		// skip blend weight
		iss >> a >> i >> a >> i >> a >> i >> a >> i;

		// normal
		iss >> a >> vertex.normal.x >> a >> vertex.normal.y >> a >> vertex.normal.z >> a >> i;
		XMStoreFloat3(&vertex.normal, XMLoadFloat3(&vertex.normal) / 255.0f);
		
		// skip binormal
		iss >> a >> i >> a >> i >> a >> i >> a >> i;

		// tangent
		iss >> a >> vertex.tangent.x >> a >> vertex.tangent.y >> a >> vertex.tangent.z >> a >> i;
		XMStoreFloat3(&vertex.tangent, XMLoadFloat3(&vertex.tangent) / 255.0f);

		// texcoord0
		iss >> a >> vertex.uv.x >> a >> vertex.uv.y >> a >> i >> a >> i;
		XMStoreFloat2(&vertex.uv, XMLoadFloat2(&vertex.uv) / 255.0f);

		// texcoord1
		iss >> a >> i >> a >> i >> a >> i >> a >> i;

		vertices.push_back(vertex);
	}

	//std::vector<uint16_t>& indices = mesh.indices;
	//indices.resize(36);

	return mesh;
}