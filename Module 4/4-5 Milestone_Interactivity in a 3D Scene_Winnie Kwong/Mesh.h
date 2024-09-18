///////////////////////////////////////////////////////////////////////////////
// mesh.h
// ========
// create meshes for various 3D primitives: plane, cylinder, torus, sphere
//
//  meshes.h code provided by: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 7th, 2022
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <GL/glew.h>

#include <glm/glm.hpp>

class Meshes
{

public:

	// Stores the GL data relative to a given mesh
	struct GLMesh
	{
		GLuint vao;         // Handle for the vertex array object
		GLuint vbos[2];     // Handles for the vertex buffer objects
		GLuint nVertices;	// Number of vertices for the mesh
		GLuint nIndices;    // Number of indices for the mesh
	};

	GLMesh gCylinderMesh;
	GLMesh gPlaneMesh;
	GLMesh gSphereMesh;
	GLMesh gTorusMesh;
	GLMesh gBoxMesh;

public:
	void CreateMeshes();
	void DestroyMeshes();

private:
	void UCreatePlaneMesh(GLMesh& mesh);
	void UCreateCylinderMesh(GLMesh& mesh);
	void UCreateTorusMesh(GLMesh& mesh);
	void UCreateSphereMesh(GLMesh& mesh);
	void UCreateBoxMesh(GLMesh& mesh);
	void UDestroyMesh(GLMesh& mesh);

};