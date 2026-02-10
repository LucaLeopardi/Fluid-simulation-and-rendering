#pragma once

#include <vector>
#include <glad/glad.h>
#include <utils/Vertex.hpp>
#include <utils/Texture.hpp>

class Mesh
{
private:

	GLuint _vbo = 0;			
	GLuint _ebo = 0;			
	GLuint _ebo_length = 0;
	GLuint _vao = 0;

public:

	/// @brief !!! CAN BE NULL !!! Pointer to Texture used by Mesh, either from a Model or standalone. If null, Renderer uses default Texture.
	Texture* diffuse_tex = nullptr; 
	// ^ Currently only used in Renderer, which checks for null values. // TODO: Make shared pointer?
	// TODO: Material?

	Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);	// Default constructor

	// Copy constructor/assignment are forbidden: we don't want multiple instances pointing to the same OpenGL data, since it can be deleted by them
	Mesh(const Mesh& other) = delete;												// Copy constructor
	Mesh& operator=(const Mesh& other) = delete;									// Copy assignment

	Mesh(Mesh&& other) noexcept;													// Move constructor
	Mesh& operator=(Mesh&& other) noexcept;											// Move assignment

	~Mesh();

	void cleanup();

	GLuint get_VBO() const;
	GLuint get_EBO() const;
	GLuint get_EBO_length() const;
	GLuint get_VAO() const;
};